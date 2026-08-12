
module sciter.om.value;

import std.string;
import std.utf : toUTF8, toUTF16;
import std.traits;
import std.stdio;
import std.typecons;
import std.range;
import std.algorithm;
import core.memory : GC;

private void checkRV(uint rv) {
  if(rv == VALUE_RESULT.BAD_PARAMETER)
    throw new Exception("Invalid parameter");
  else if(rv == VALUE_RESULT.INCOMPATIBLE_TYPE)
    throw new Exception("Incompatible type");
}

private struct Box(T)
{
    T value;
}

struct VALUE // a.k.a. discriminated union
{
  uint  t;
  uint  u;
  ulong d;

  VALUE_TYPE type() { return cast(VALUE_TYPE)t; }
  uint unit()       { return u; } // see enum VALUE_UNIT_TYPE_*** below
  void clear()      { ValueClear(&this); } 

  this(bool v) { checkRV(ValueIntDataSet(&this,cast(int)v,VALUE_TYPE.T_BOOL,0)); }
  this(int v) { checkRV(ValueIntDataSet(&this,v,VALUE_TYPE.T_INT,0)); }
  this(long v) { checkRV(ValueInt64DataSet(&this,v,VALUE_TYPE.T_BIG_INT,0));  }
  this(double v) { checkRV(ValueFloatDataSet(&this,v,VALUE_TYPE.T_FLOAT,0));  }
  this(wstring v) { checkRV(ValueStringDataSet(&this,v.ptr,cast(uint)v.length,0)); }
  this(string v) { this(toUTF16(v)); }
  this(VALUE v) { checkRV(ValueCopy(&this,&v)); }
  this(ref return scope const VALUE v) { checkRV(ValueCopy(&this,&v)); }

  this(Promise promise) { checkRV(ValueCopy(&this,&promise.thenable)); }
  
  this(T)(T rhs) if(isCallable!T) { this = from(rhs); }
  // does not work, why?
  // this(T)(T rhs) if(is(T == struct) && !is(T == VALUE)) { writeln("this!isStruct(v)"); this = fromStruct(rhs); }

  bool isUndefined() const { return t == VALUE_TYPE.T_UNDEFINED; }
  bool isNull() const { return t == VALUE_TYPE.T_NULL; }
  bool isBool() const { return t == VALUE_TYPE.T_BOOL; }
  bool isInt() const  { return t == VALUE_TYPE.T_INT; }
  bool isLong() const { return t == VALUE_TYPE.T_BIG_INT; }
  bool isFloat() const { return t == VALUE_TYPE.T_FLOAT; }
  bool isString() const { return t == VALUE_TYPE.T_STRING; }
  bool isError() const { return t == VALUE_TYPE.T_STRING && u == VALUE_UNIT_TYPE_STRING.UT_STRING_ERROR; }

  /// val.call(...) will work for it
  bool isJsFunction() const { return t == VALUE_TYPE.T_OBJECT && u == VALUE_UNIT_TYPE_OBJECT.UT_OBJECT_FUNCTION; }

  /// is enumerable by foreach (ref string k, ref VALUE v; valMap) {}
  bool isMapAlike() const { return t == VALUE_TYPE.T_MAP || // primitive map
                                   (t == VALUE_TYPE.T_OBJECT && u == VALUE_UNIT_TYPE_OBJECT.UT_OBJECT_OBJECT); } // JS object

  /// will val[0] work for it
  bool isArrayAlike() const { return t == VALUE_TYPE.T_ARRAY || // primitive array
                                    (t == VALUE_TYPE.T_OBJECT && u == VALUE_UNIT_TYPE_OBJECT.UT_OBJECT_ARRAY); } // JS array

  bool    opCast(T : bool)() const { int v; checkRV(ValueIntData(&this,&v)); return v != 0; }
  int     opCast(T : int)() const { int v; checkRV(ValueIntData(&this,&v)); return v; }
  long    opCast(T : long)() const { long v; checkRV(ValueInt64Data(&this,&v)); return v; }
  double  opCast(T : double)() const { double v; checkRV(ValueFloatData(&this,&v)); return v; }
  wchar[] opCast(T : wchar[])() const { wchar* chars = null; uint numChars = 0; checkRV(ValueStringData(&this,&chars,&numChars)); return chars[0 .. numChars]; }
  string  opCast(T : string)() const { return toUTF8(cast(wchar[])(this)); }
  VALUE   opCast(T : VALUE)() const { VALUE v; checkRV(ValueCopy(&v, &this)); return v; }

  void opAssign(bool v) { clear(); checkRV(ValueIntDataSet(&this,cast(int)v,VALUE_TYPE.T_BOOL,0)); }
  void opAssign(int v) { clear(); checkRV(ValueIntDataSet(&this,v,VALUE_TYPE.T_INT,0)); }
  void opAssign(long v) { clear(); checkRV(ValueInt64DataSet(&this,v,VALUE_TYPE.T_BIG_INT,0));  }
  void opAssign(double v) { clear(); checkRV(ValueFloatDataSet(&this,v,VALUE_TYPE.T_FLOAT,0));  }
  void opAssign(wstring v) { clear(); checkRV(ValueStringDataSet(&this,v.ptr,cast(uint)v.length,0)); }
  void opAssign(string v) { clear(); opAssign(toUTF16(v)); }
  //void opAssign(ref const(VALUE) v) { if(&this == &v) return; clear(); checkRV(ValueCopy(&this,&v)); }
  void opAssign(VALUE v) { clear(); checkRV(ValueCopy(&this,&v)); }
  void opAssign(Promise promise) { clear(); checkRV(ValueCopy(&this,&promise.thenable)); }

  /// declares the value as a map and assigns val to key
  void  opIndexAssign(VALUE val, string key ) { VALUE vkey = key; checkRV(ValueSetValueToKey(&this,&vkey, &val)); }
  /// gets property 'key' of map or object contained in the VALUE
  VALUE opIndex(string key ) const { VALUE vkey = key; VALUE val; checkRV(ValueGetValueOfKey(&this,&vkey, &val)); return val; }

  /// declares the value as an array and assigns val to index
  void opIndexAssign(VALUE val, int index ) { checkRV(ValueNthElementValueSet(&this,index, &val)); }
  /// gets value at index of array or JS Array/TypedArray instances
  VALUE opIndex(int index ) const { VALUE val; checkRV(ValueNthElementValue(&this,index, &val)); return val; }

  /// returns number of items if this value holds an array
  uint  itemsCount() const { uint rv; checkRV(ValueElementsCount(&this,&rv)); return rv; }

  /// foreach (ref string k, ref VALUE v; valMap) { ... }
  int opApply(int delegate(ref string, ref const VALUE) dg) {
        auto dgh = new DGHolder;
        dgh.dg = dg;
        return cast(int) ValueEnumElements(&this,&eachKV,cast(void*)dgh);
    }

  /// error value, it is a string but marked as UT_STRING_ERROR
  static VALUE makeError(string errMsg) {
    VALUE v = errMsg;
    v.u = VALUE_UNIT_TYPE_STRING.UT_STRING_ERROR; 
    return v;
  }

  wstring toWstring(VALUE_STRING_CVT_TYPE cvt = VALUE_STRING_CVT_TYPE.SIMPLE) const {
    VALUE v = this;
    ValueToString(&v,cvt);
    return (cast(wchar[])v).idup;
  }

  string toString(VALUE_STRING_CVT_TYPE cvt = VALUE_STRING_CVT_TYPE.SIMPLE) {
    auto ws = toWstring(cvt);
    return toUTF8(ws);
  }

  ~this() { clear(); }

  // --- Generic call method ---
  VALUE call(Args...)(Args args) const
  {
    // Convert arguments to VALUE[]
    VALUE[Args.length] argv;
    static foreach (i, A; Args)
    {
      argv[i] = args[i];
    }

    VALUE This; // undefined
    VALUE retval;

    // Invoke the function
    uint rv = ValueInvoke(&this,&This,cast(uint)Args.length,argv.ptr,&retval,null);
    
    checkRV(rv);
    return retval;
  }


  static VALUE from(T)(T rhs) {
      static if (isCallable!T)
      {
          VALUE v;
          // Allocate box to store the callable
          auto box = new Box!T;
          box.value = rhs;
          // Generate callable thunk based on TYPE T
          auto fp = &makeThunk!T.thunk;
          auto fpr = &makeThunk!T.thunkReleaser;
          // As this wraps D callable that can be passed to JS runtime we need to prevent it from collection
          GC.addRoot(box);
          GC.setAttr(box, GC.BlkAttr.NO_MOVE);
          // Register
          auto status = ValueNativeFunctorSet(&v, fp, fpr, cast(void*) box);
          checkRV(status);
          return v;
      }  else if (is(T == struct)) {
          VALUE v;
          foreach (name; FieldNameTuple!T)
          {
              // Get field value by name
              auto value = __traits(getMember, rhs, name);
              v[name] = cast(VALUE)value;
          }
          return v;
      } 
      else {
        assert(0, "Cannot coerse " ~ fullyQualifiedName!T ~ " to the VALUE");
      }
  }

  static VALUE fromStruct(T)(T rhs) {
      static if (is(T == struct))
      {
          VALUE v;
          foreach (name; FieldNameTuple!T)
          {
              // Get field value by name
              auto value = __traits(getMember, rhs, name);
              v[name] = cast(VALUE)value;
          }
          return v;
      } 
      else {
        assert(0, "Cannot coerse " ~ fullyQualifiedName!T ~ " to the VALUE");
      }
  }



}

///
/// The Promise/A+ thing, see https://promisesaplus.com/
///
/// To make native async function simply return a Promise from it.
///
class Promise {

  protected VALUE resolver;
  protected VALUE rejector;
  protected VALUE thenable;

  this() 
  { 
    // make a "thenable" object, it contains single then(resolverJSfunc,rejectorJSfunc)
    thenable["then"] = VALUE(&this.then);
  }

  void clear() {
    resolver.clear();
    rejector.clear();
    thenable.clear();
  }

  protected VALUE then(ref VALUE resolverCb,ref VALUE rejectorCb) {
    resolver = resolverCb;
    rejector = rejectorCb;
    return thenable;
  }

  /// use this method in D to mark "task is done and this is the result"
  void resolve(ref VALUE data) {
    if (!resolver.isJsFunction) return; 
    resolver.call(data);
    clear();
  }

  /// use this method in D to mark "task failed and this is the error"
  void reject(string errMsg) { 
    if (!rejector.isJsFunction) return; 
    rejector.call(VALUE.makeError(errMsg));
    clear();
  }  
}


template makeThunk(T)
{
    alias Params = Parameters!T;
    alias Ret    = ReturnType!T;

    extern(C) void thunk(void* tag, uint argc, const VALUE* argv, VALUE* retval)
    {
        auto box = cast(Box!T*) tag;   // retrieve callable
        auto callable = box.value;

        if (argc != Params.length)
            assert(0);

        alias ArgsTuple = Tuple!(Params);
        ArgsTuple args;

        static foreach (i, P; Params)
            args[i] = cast(P) argv[i];

        static if (is(Ret == void))
        {
            callable(args.expand);
        }
        else
        {
            auto r = callable(args.expand);
            *retval = cast(VALUE)r;
        }
    }

    extern(C) void thunkReleaser(void* tag) {
      auto box = cast(Box!T*) tag; // retrieve callable
      GC.removeRoot(box);
      GC.clrAttr(box, GC.BlkAttr.NO_MOVE); 
    }
}

private struct DGHolder
{
  int delegate(ref string, ref const VALUE) dg;
}

extern(C) int eachKV(void* param, const VALUE* pkey, const VALUE* pval ) {
  auto dgh = cast(DGHolder*)param;
  string key = cast(string) *pkey;
  return dgh.dg(key,*pval);
}

enum VALUE_RESULT
{
  OK_TRUE = -1,
  OK = 0,
  BAD_PARAMETER = 1,
  INCOMPATIBLE_TYPE = 2
}

enum VALUE_TYPE
{
    T_UNDEFINED = 0,
    T_NULL = 1,
    T_BOOL = 2,
    T_INT  = 3,
    T_FLOAT = 4,
    T_STRING = 5,
    T_DATE = 6,        // INT64 - contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC), a.k.a. FILETIME on Windows
    T_BIG_INT = 7,     // INT64
    T_LENGTH = 8,      // length units, value is int or float, units are VALUE_UNIT_TYPE
    T_ARRAY = 9,
    T_MAP = 10,
    T_FUNCTION = 11,   // named tuple , like array but with name tag
    T_BYTES = 12,      // sequence of bytes - e.g. image data
    T_OBJECT = 13,     // scripting object proxy (JS/SCITER)
    T_RESOURCE = 15,   // other thing derived from tool::resource
    T_DURATION = 17,   // double, seconds
    T_ANGLE = 18,      // double, radians
    T_COLOR = 19,      // [unsigned] INT, ABGR
    T_ASSET = 21,      // sciter::om::iasset* add_ref'ed pointer
}

enum VALUE_UNIT_TYPE_LENGTH
{
    UT_EM = 1, //height of the element's font. 
    UT_EX = 2, //height of letter 'x' 
    UT_PR = 3, //%
    UT_SP = 4, //%% "springs", a.k.a. flex units
    reserved1 = 5, 
    reserved2 = 6, 
    UT_PX = 7, //pixels
    UT_IN = 8, //inches (1 inch = 2.54 centimeters). 
    UT_CM = 9, //centimeters. 
    UT_MM = 10, //millimeters. 
    UT_PT = 11, //points (1 point = 1/72 inches). 
    UT_PC = 12, //picas (1 pica = 12 points). 
    UT_DIP = 13,
    reserved3 = 14, 
    reserved4 = 15, 

    UT_PR_WIDTH = 16, // width(n%)
    UT_PR_HEIGHT = 17, // height(n%)
    UT_PR_VIEW_WIDTH = 18, // vw
    UT_PR_VIEW_HEIGHT = 19, // vh
    UT_PR_VIEW_MIN = 20, // vmin
    UT_PR_VIEW_MAX = 21, // vmax

    UT_REM = 22, // root em
    UT_PPX = 23, // physical px
    UT_CH = 24   // width of '0'

}

enum VALUE_UNIT_TYPE_DATE
{
    DT_HAS_DATE         = 0x01, // date contains date portion
    DT_HAS_TIME         = 0x02, // date contains time portion HH:MM
    DT_HAS_SECONDS      = 0x04, // date contains time and seconds HH:MM:SS
    DT_UTC              = 0x10, // T_DATE is known to be UTC. Otherwise it is local date/time
}

enum VALUE_UNIT_TYPE_OBJECT
{
    UT_OBJECT_ARRAY  = 0,   // type T_OBJECT of type Array
    UT_OBJECT_OBJECT = 1,   // type T_OBJECT of type Object
    UT_OBJECT_CLASS  = 2,   // type T_OBJECT of type Class (class or namespace)
    UT_OBJECT_NATIVE = 3,   // type T_OBJECT of native Type with data slot (LPVOID)
    UT_OBJECT_FUNCTION = 4, // type T_OBJECT of type Function
    UT_OBJECT_ERROR = 5,    // type T_OBJECT of type Error
    UT_OBJECT_BUFFER = 6,   // type T_OBJECT of type ArrayBuffer or TypedArray
}

enum VALUE_UNIT_TYPE_STRING
{
    UT_STRING_STRING = 0,        // string
    UT_STRING_ERROR  = 1,        // is an error string
    UT_STRING_SECURE = 2,        // secure string ("wiped" on destroy)
    UT_STRING_SYMBOL = 0xffff,   // symbol/name[token]
}

enum VALUE_STRING_CVT_TYPE
{
  SIMPLE,        ///< simple conversion of terminal values 
  JSON_LITERAL,  ///< json literal parsing/emission 
  JSON_MAP,      ///< json parsing/emission, it parses as if token '{' already recognized 
  XJSON_LITERAL, ///< x-json parsing/emission, date is emitted as ISO8601 date literal, currency is emitted in the form DDDD$CCC
}


alias ValueInitF = extern(System) uint function(VALUE*);
alias ValueClearF = extern(System) uint function(VALUE*);
alias ValueCompareF = extern(System) uint function(const VALUE*,const VALUE*);
alias ValueCopyF = extern(System) uint function(VALUE*,const VALUE*);
alias ValueIsolateF = extern(System) uint function(VALUE*);
alias ValueTypeF = extern(System) uint function(const VALUE* pval, uint* pType, uint* pUnits);
alias ValueStringDataF = extern(System) uint function(const VALUE* pval, wchar** pChars, uint* pNumChars);
alias ValueStringDataSetF = extern(System) uint function(VALUE* pval, const(wchar)* chars, uint numChars, uint units);

alias ValueIntDataF = extern(System) uint function(const VALUE* pval, int* pData);
alias ValueIntDataSetF = extern(System) uint function(VALUE* pval, int data, uint type, uint units);

alias ValueInt64DataF = extern(System) uint function(const VALUE* pval, long* pData);
alias ValueInt64DataSetF = extern(System) uint function(VALUE* pval, long data, uint type, uint units);

alias ValueFloatDataF = extern(System) uint function(const VALUE* pval, double* pData);
alias ValueFloatDataSetF = extern(System) uint function(VALUE* pval, double data, uint type, uint units);

alias ValueBinaryDataF = extern(System) uint function(const VALUE* pval, const(ubyte)** ppBytes, uint* pnBytes);
alias ValueBinaryDataSetF = extern(System) uint function(VALUE* pval, const(ubyte)* pBytes, uint nBytes, uint type, uint units);

alias ValueElementsCountF = extern(System) uint function(const VALUE* pval, uint* pn);

alias ValueNthElementValueF = extern(System) uint function(const VALUE* pval, int n, VALUE* pretval);
alias ValueNthElementValueSetF = extern(System) uint function(VALUE* pval, int n, const VALUE* pval_to_set);

alias KEY_VALUE_CALLBACK = extern(C) int function(void* param, const VALUE* pkey, const VALUE* pval );
alias ValueEnumElementsF = extern(System) uint function(const VALUE* pval, KEY_VALUE_CALLBACK penum, void* param);

alias ValueSetValueToKeyF = extern(System) uint function(VALUE* pval, const VALUE* pkey, const VALUE* pval_to_set);
alias ValueGetValueOfKeyF = extern(System) uint function(const VALUE* pval, const VALUE* pkey, VALUE* pretval);

alias ValueToStringF = extern(System) uint function( VALUE* pval, uint how );
alias ValueFromStringF = extern(System) uint function( VALUE* pval, const(wchar)* str, uint strLength, uint how );

alias ValueInvokeF = extern(System) uint function( const VALUE* pval, VALUE* pthis, uint argc, const VALUE* argv, VALUE* pretval, const(wchar)* url );

// Native functor
alias NATIVE_FUNCTOR_INVOKE = extern(C) void function( void* tag, uint argc, const(VALUE)* argv, VALUE* retval); // retval may contain error definition
alias NATIVE_FUNCTOR_RELEASE = extern(C) void function( void* tag );

alias ValueNativeFunctorSetF = extern(System) uint function( VALUE* pval, NATIVE_FUNCTOR_INVOKE  pinvoke, NATIVE_FUNCTOR_RELEASE prelease, void* tag );
alias ValueIsNativeFunctorF = extern(System) int function(const VALUE* pval);

ValueInitF ValueInit; 
ValueClearF ValueClear;
ValueCompareF ValueCompare;
ValueCopyF ValueCopy;
ValueIsolateF ValueIsolate;
ValueTypeF ValueType;
ValueStringDataF ValueStringData;
ValueStringDataSetF ValueStringDataSet;

ValueIntDataF ValueIntData;
ValueIntDataSetF ValueIntDataSet;

ValueInt64DataF ValueInt64Data;
ValueInt64DataSetF ValueInt64DataSet;

ValueFloatDataF ValueFloatData;
ValueFloatDataSetF ValueFloatDataSet;

ValueBinaryDataF ValueBinaryData;
ValueBinaryDataSetF ValueBinaryDataSet;

ValueElementsCountF ValueElementsCount;

ValueNthElementValueF ValueNthElementValue;
ValueNthElementValueSetF ValueNthElementValueSet;

ValueEnumElementsF ValueEnumElements;

ValueSetValueToKeyF ValueSetValueToKey;
ValueGetValueOfKeyF ValueGetValueOfKey;

ValueToStringF ValueToString;
ValueFromStringF ValueFromString;

ValueInvokeF ValueInvoke;

ValueNativeFunctorSetF ValueNativeFunctorSet;
ValueIsNativeFunctorF ValueIsNativeFunctor;
