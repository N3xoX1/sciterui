module sciter.om.types;

import sciter.api;

struct AssetT {
  AssetClassT* isa;
}

alias AssetAddRefF =  int function(AssetT* thing);
alias AssetReleaseF =  int function(AssetT* thing);
alias AssetGetInterfaceF = int function(AssetT* thing, const char* name, void** ppif);
alias AssetGetPassportF = AssetPassportT* function(AssetT* thing);

struct AssetClassT {
  AssetAddRefF asset_add_ref; 
  AssetReleaseF asset_release;
  AssetGetInterfaceF asset_get_interface;
  AssetGetPassportF asset_get_passport;
}

alias AssetPropGetterF = SBOOL function(AssetT* thing, const VALUE* p_value);
alias AssetPropSetterF = SBOOL function(AssetT* thing, VALUE* p_value);
alias AssetItemGetterF = SBOOL function(AssetT* thing, const VALUE* p_key, const VALUE* p_value);
alias AssetItemSetterF = SBOOL function(AssetT* thing, const VALUE* p_key, VALUE* p_value);
alias AssetAnyPropGetterF = SBOOL function(AssetT* thing, ATOM prop, const VALUE* p_value);
alias AssetAnyPropSetterF = SBOOL function(AssetT* thing, ATOM prop, VALUE* p_value);
alias AssetItemNextF = SBOOL function(AssetT* thing, VALUE* p_idx /*in/out*/, VALUE* p_value);

alias AssetMethodF = SBOOL function(AssetT* thing, UINT argc, const VALUE* argv, VALUE* p_result);

alias AssetNameResolverF = SBOOL function(AssetT* thing, ATOM prop, UINT* pIndex, SBOOL *pIsMethod);

enum {
  PROP_TYPE_ACCESSOR = 0,
  PROP_TYPE_I32 = 1,
  PROP_TYPE_I64 = 2,
  PROP_TYPE_F64 = 3,
  PROP_TYPE_STRING = 4,
}

struct AssetPropAccessorsT {
  AssetPropGetterF getter;
  AssetPropSetterF setter;
}


struct AssetPropertyDefT
{
  INT_PTR   type; // PROP_TYPE
  ATOM      name;
  union {
    struct {
      AssetPropGetterF getter;
      AssetPropSetterF setter;
    }
    int i32;
    long i64;
    double f64;
    const char* str;
  }

  this(const char* n, AssetPropGetterF pg, AssetPropSetterF ps = null)
  {
    type = PROP_TYPE_ACCESSOR;
    name = SciterAtomValue(n);
    getter = pg;
    setter = ps;
  }
  this(const char* n, int c)
  {
    type = PROP_TYPE_I32;
    name = SciterAtomValue(n);
    i32 = c;
  }
  this(const char* n, long c) 
  {
    type = PROP_TYPE_I64;
    name = SciterAtomValue(n);
    i64 = c;
  }
  this(const char* n, double c) 
  {
    type = PROP_TYPE_F64;
    name = SciterAtomValue(n);
    f64 = c;
  }
  this(const char* n, const char* c) 
  {
    type = PROP_TYPE_STRING;
    name = SciterAtomValue(n);
    str = c;
  }
}

struct AssetMethodDefT {
  void*        reserved;
  ATOM         name;
  size_t       params;
  AssetMethodF func;

  this(const char* n, size_t np, AssetMethodF f) {
    name = SciterAtomValue(n);
    params = np;
    func = f; 
    reserved = null;
  }
}

enum PassportFlag {
  SEALED_OBJECT = 0x00,     // not extendable
  EXTENDABLE_OBJECT = 0x01, // extendable, asset may have new properties added
  HAS_NAME_RESOLVER = 0x02  // if name_resolver is valid 
}

// definiton of object's (the thing) access interface
// this structure should be statically allocated - at least to survive last instance of the engine
struct AssetPassportT {
  ulong          flags;
  ATOM           name;         // class name
  const AssetPropertyDefT* properties; size_t n_properties; // virtual property thunks
  const AssetMethodDefT*   methods; size_t n_methods;       // method thunks
  AssetItemGetterF         item_getter;  // var item_val = thing[k];
  AssetItemSetterF         item_setter;  // thing[k] = item_val;
  AssetItemNextF           item_next;    // for(var item of thisThing)
  // any property "inteceptors"
  AssetAnyPropGetterF     prop_getter;  // var prop_val = thing.k;
  AssetAnyPropSetterF     prop_setter;  // thing.k = prop_val;
  AssetNameResolverF      name_resolver;
  void* reserved;
}

