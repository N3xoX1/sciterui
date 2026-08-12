module sciter.dom;

import std.string;
import std.typecons; // for Tuple
import std.utf;

public import sciter.dom.events;
public import sciter.dom.types;
public import sciter.dom.controller;
import sciter.types;
import sciter.api;
import sciter.utils.cvt;

struct Element {
  HELEMENT handle;

  this(HELEMENT h) { handle = h; if(handle) checkDOMop(Sciter_UseElement(handle)); }
  this(ref return scope const Element e) { this(e.handle); }
  ~this() { clear(); }
  void clear() { if(handle) { checkDOMop(Sciter_UnuseElement(handle)); handle = null; } }

  void opAssign(HELEMENT h) { clear(); handle = h; if(handle) checkDOMop(Sciter_UseElement(handle)); }
  void opAssign(Element el) { clear(); this = el.handle; }

  /// creates detached element with the tag, e.g. el = Element.create("div");
  static Element create(string tag) { HELEMENT he; SciterCreateElement(tag.toStringz,null,&he); return Element(he); }

  bool opCast(T : bool)() const { return handle != null; }
  HELEMENT opCast(T : HELEMENT)() const { return handle; }
  Node     opCast(T : Node)() const { HNODE hn; SciterNodeCastFromElement(cast(HELEMENT)handle,&hn); return Node(hn); }

  string  tag() const { StringBox pb; checkDOMop(SciterGetElementTypeCB(handle,&LPCSTR_receiver,cast(LPVOID)&pb)); return pb.value; }
  Element parent() const { HELEMENT hp; checkDOMop(SciterGetParentElement(handle,&hp)); return Element(hp); }
  
  bytes   html() const { BytesBox pb; checkDOMop(SciterGetElementHtmlCB(handle,0,&LPCBYTES_receiver,cast(LPVOID)&pb)); return pb.value; }
  bytes   outerHtml() const { BytesBox pb; checkDOMop(SciterGetElementHtmlCB(handle,1,&LPCBYTES_receiver,cast(LPVOID)&pb)); return pb.value; }
  wstring text() const { WStringBox pb; checkDOMop(SciterGetElementTextCB(handle,&LPCWSTR_receiver,cast(LPVOID)&pb)); return pb.value; }
  void    text(wstring s) { checkDOMop(SciterSetElementText(handle,cast(LPCWSTR)s.ptr,cast(UINT)s.length)); }

  /// ordered collection of child elements
  ElementChildren   children() const { return ElementChildren(this); }
  /// ordered collection of child nodes
  NodeNodes         nodes() const { return (cast(Node)this).children; }
  /// name/value collection of attributes
  ElementAttributes attributes() const { return ElementAttributes(this); }

  /// CSS selector
  /// check if this element matches the selector: element.match("div.fancy") 
  bool    match(string cssSelector) const { HELEMENT heFound; SciterSelectParent(handle, cssSelector.toStringz, 1, &heFound); return heFound != null; }
  /// find first matching child:
  ///   ch = element.selectFirstChild(".fancy") - any child element with "fancy" class
  ///   ch = element.selectFirstChild("> .fancy") - any immediate child with "fancy" class
  Element selectFirstChild(string cssSelector) const {
    ElementsBox box;
    SciterSelectElements( handle, cssSelector.toStringz, &ELEMENT_receiver, cast(LPVOID) &box);
    return box.list.length ? box.list[0] : Element(null);
  }
  /// find all matching children:
  Element[] selectAllChildren(string cssSelector) const {
    ElementsBox box;
    SciterSelectElements( handle, cssSelector.toStringz, &ELEMENTS_receiver, cast(LPVOID) &box);
    return box.list;
  }
  /// find nearest matching parent:
  ///   table = td.selectNearestParent("table");
  Element selectNearestParent(string cssSelector) const { HELEMENT heFound; SciterSelectParent(handle, cssSelector.toStringz, 0, &heFound); return Element(heFound); }  


  enum AREA : UINT
  {
    ROOT_RELATIVE = 0x01,       /// - or this flag if you want to get Sciter window relative coordinates,
                                ///   otherwise it will use nearest windowed container e.g. popup window.
    SELF_RELATIVE = 0x02,       /// - "or" this flag if you want to get coordinates relative to the origin
                                ///   of element iself.
    PARENT_RELATIVE = 0x03,     /// - position inside immediate container.
    VIEW_RELATIVE   = 0x04,     /// - position relative to Sciter window

    CONTENT_BOX = 0x00,   /// content (inner)  box
    PADDING_BOX = 0x10,   /// content + paddings
    BORDER_BOX  = 0x20,   /// content + paddings + border
    MARGIN_BOX  = 0x30,   /// content + paddings + border + margins

    BACK_IMAGE_AREA = 0x40, /// relative to content origin - location of background image (if it set no-repeat)
    FORE_IMAGE_AREA = 0x50, /// relative to content origin - location of foreground image (if it set no-repeat)

    SCROLLABLE_AREA = 0x60, /// scroll_area - scrollable area in content box

    AS_PPX = 0x8000,        /// as OS pixels, otherwise in logical px units 
  }

  RECT location(UINT area /*AREA combinations*/) { RECT rc; checkDOMop(SciterGetElementLocation(handle,&rc,area)); return rc;  }

  /// make the element visible inside its scrollable containers
  void scrollToView(bool smooth = false, bool toTopOfView = false ) {
    enum { SCROLL_TO_TOP = 0x01, SCROLL_SMOOTH = 0x10 }
    UINT flags = 0;
    if(toTopOfView)  flags |= SCROLL_TO_TOP;
    if(smooth) flags |= SCROLL_SMOOTH;
    checkDOMop(SciterScrollToView(handle,flags));
  }

  /// request element to be repainted
  void requestPaint() const { checkDOMop(SciterRequestPaint(handle)); }

  enum // ELEMENT_STATE_BITS
  {
     STATE_LINK             = 0x00000001,  // :link pseudo-class in CSS
     STATE_HOVER            = 0x00000002,  // :hover pseudo-class in CSS
     STATE_ACTIVE           = 0x00000004,  // :active pseudo-class in CSS
     STATE_FOCUS            = 0x00000008,  // :focus pseudo-class in CSS
     STATE_VISITED          = 0x00000010,  // :visited pseudo-class in CSS
     STATE_CURRENT          = 0x00000020,  // :current, current (hot) item
     STATE_CHECKED          = 0x00000040,  // :checked, element is checked (or selected)
     STATE_DISABLED         = 0x00000080,  // :disabled, element is disabled
     STATE_READONLY         = 0x00000100,  // :read-only, readonly input element
     STATE_EXPANDED         = 0x00000200,  // :expanded, expanded state - nodes in tree view
     STATE_COLLAPSED        = 0x00000400,  // :collapsed, collapsed state - nodes in tree view - mutually exclusive with
     STATE_INCOMPLETE       = 0x00000800,  // :incomplete, fore (content) image was requested but not delivered
     STATE_ANIMATING        = 0x00001000,  // :animating, is animating currently
     STATE_FOCUSABLE        = 0x00002000,  // :focusable, will accept focus
     STATE_ANCHOR           = 0x00004000,  // :anchor, anchor in selection (used with current in selects)
     STATE_SYNTHETIC        = 0x00008000,  // :synthetic, this is a synthetic element - don't emit it's head/tail
     STATE_OWNS_POPUP       = 0x00010000,  // :owns-popup, has visible popup element (tooltip, menu, etc.) 
     STATE_TABFOCUS         = 0x00020000,  // :tab-focus, focus gained by tab traversal
     STATE_EMPTY            = 0x00040000,  // :empty, empty - element is empty (text.size() == 0 && subs.size() == 0)
                                           //  if element has behavior attached then the behavior is responsible for the value of this flag.
     STATE_BUSY             = 0x00080000,  // :busy, busy doing something (e.g. loading data)

     STATE_DRAG_OVER        = 0x00100000,  // drag over the block that can accept it (so is current drop target). Flag is set for the drop target block
     STATE_DROP_TARGET      = 0x00200000,  // active drop target.
     STATE_MOVING           = 0x00400000,  // dragging/moving - the flag is set for the moving block.
     STATE_COPYING          = 0x00800000,  // dragging/copying - the flag is set for the copying block.
     STATE_DRAG_SOURCE      = 0x01000000,  // element that is a drag source.
     STATE_DROP_MARKER      = 0x02000000,  // element is drop marker

     STATE_PRESSED          = 0x04000000,  // pressed - close to active but has wider life span - e.g. in MOUSE_UP it
                                           //   is still on; so behavior can check it in MOUSE_UP to discover MOUSE_CLICK condition.
     STATE_POPUP            = 0x08000000,  // :popup, this element is out of flow - shown as popup

     STATE_IS_LTR           = 0x10000000,  // the element or one of its containers has dir=ltr declared
     STATE_IS_RTL           = 0x20000000,  // the element or one of its containers has dir=rtl declared
  }

  /// runtime ELEMENT_STATE_BITS
  UINT state() const { UINT s; checkDOMop(SciterGetElementState(handle,&s)); return s; }
  void state(UINT stateBitsToSet, UINT stateBitsToClear = 0) const { UINT s; checkDOMop(SciterSetElementState(handle,stateBitsToSet, stateBitsToClear,0)); }

  enum CONTROL_TYPE: UINT {
    NO      = 0, ///< This dom element has no behavior at all.
    UNKNOWN = 1, ///< This dom element has behavior but its type is unknown.

    EDIT            = 2,  ///< Single line edit box.
    NUMERIC         = 3,  ///< Numeric input with optional spin buttons.
    CLICKABLE       = 4,  ///< toolbar button, behavior:clickable.
    BUTTON          = 5,  ///< Command button.
    CHECKBOX        = 6,  ///< CheckBox (button).
    RADIO           = 7,  ///< OptionBox (button).
    SELECT_SINGLE   = 8,  ///< Single select, ListBox or TreeView.
    SELECT_MULTIPLE = 9,  ///< Multiselectable select, ListBox or TreeView.
    DD_SELECT       = 10, ///< Dropdown single select.
    TEXTAREA        = 11, ///< Multiline TextBox.
    HTML_SELECTION  = 12, ///< HTML selection behavior.
    PASSWORD        = 13, ///< Password input element.
    PROGRESS        = 14, ///< Progress element.
    SLIDER          = 15, ///< Slider input element.
    DECIMAL         = 16, ///< Decimal number input element.
    CURRENCY        = 17, ///< Currency input element.
    SCROLLBAR       = 18,
    LIST            = 19,
    HTMLAREA        = 20, ///< WYSIWYG HTML editor
    CALENDAR        = 21,
    DATE            = 22,
    TIME            = 23,
    FILE            = 24, ///< file input element.
    PATH            = 25, ///< path input element.

    LAST_INPUT = 26,

    HYPERLINK = LAST_INPUT,
    FORM       = 27,

    MENUBAR    = 28,
    MENU       = 29,
    MENUBUTTON = 30,

    FRAME    = 31,
    FRAMESET = 32,

    TOOLTIP = 33,

    HIDDEN  = 34,
    URL     = 35, ///< URL input element.
    TOOLBAR = 36,

    WINDOW = 37, ///< has HWND attached to it

    LABEL     = 38,
    IMAGE     = 39, ///< image/video object.
    PLAINTEXT = 40, ///< Multiline TextBox + colorizer.

    SELECT_TREE = 41, ///< TreeView.
  };  

  CONTROL_TYPE controlType() const { UINT ct; checkDOMop(SciterControlGetType(handle,&ct)); return cast(CONTROL_TYPE)ct; }

  /// runtime value (for input elements) : editBox.value = VALUE("new text");
  VALUE value() const { VALUE v; checkDOMop(SciterGetValue( handle, &v)); return v; }
  void  value(ref const VALUE v) const { checkDOMop(SciterSetValue( handle, &v)); }

  /// gets 'expando' of the element. 'expando' is a scripting object (of class Element)
  /// Example: int scriptingProp = cast(int) element.expando["someProp"]; 
  ///          gets value of someProp if it was set in JS as el.someProp = 42;
  VALUE expando() const { VALUE v; checkDOMop(SciterGetExpando( handle, &v, 1 )); return v; }

  /// call JS method with 'this' == this element
  VALUE callThis(string methodName, Args...)(Args args)
  {
    // Convert arguments to VALUE[]
    VALUE[Args.length] argv;
    static foreach (i, A; Args) { argv[i] = args[i]; }

    VALUE retval;
    // Invoke the method
    checkDOMop(SciterCallScriptingMethod( handle, methodName.toStringz, cast(uint)Args.length,argv.ptr,&retval));
    return retval;
  }

  /// call free standing JS function defined in element's document namespace
  /// Example, in D: window.root.call("doAction","greet") 
  ///          will call JS: function doAction(action) {}
  VALUE call(string functionName, Args...)(Args args)
  {
    // Convert arguments to VALUE[]
    VALUE[Args.length] argv;
    static foreach (i, A; Args) { argv[i] = args[i]; }

    VALUE retval;
    // Invoke the method
    checkDOMop(SciterCallScriptingFunction( handle, functionName.toStringz, cast(uint)Args.length,argv.ptr,&retval));
    return retval;
  }

  /// evaluates the script in context of the element - `this` is set to the Element
  VALUE eval(wstring script) {
    VALUE retval;
    checkDOMop(SciterEvalElementScript( handle, cast(LPCWSTR)script.ptr, cast(UINT) script.length, &retval));
    return retval;
  }

  VALUE eval(string script) { return eval(toUTF16(script)); }

  /// send/post named event, the event will bubble from this element up to the window
  bool fireEvent(wstring name, VALUE payload = VALUE(), bool post = true) const
  {
   SEMANTIC_EVENT params;
   params.type = EVENT.CUSTOM;
   params.target = handle;
   params.related = handle;
   params.name = name.ptr; // assumes that this is static string literal
   params.data = payload;
   SBOOL handled = false;
   checkDOMop(SciterFireEvent(&params, post, &handled));
   return handled != 0;
  }

}

struct ElementChildren {
  protected Element self;
  protected this(Element e) {self = e; }
 
  bool opCast(T : bool)() const { return self.handle != null && length() > 0; }

  Element opIndex(int index ) const { HELEMENT el; checkDOMop(SciterGetNthChild(self.handle,index, &el)); return Element(el); }

  int opApply(int delegate(ref uint, ref Element) dg) {
    uint count = length();
    for (uint n = 0; n < count; ++n) {
      Element child = this[n];
      auto result = dg(n, child);
      if (result) // non-zero means break
        return result;
    }
    return 0;
  }

  uint length() const { uint count; checkDOMop(SciterGetChildrenCount(self.handle,&count)); return count; }
}

struct ElementAttributes {
  protected Element self;
  protected this(Element e) {self = e; }
 
  bool opCast(T : bool)() const { return self.handle != null && length() > 0; }

  /// Getter: auto (name, value) = element.attributes[0];
  Tuple!(string, wstring) opIndex(uint index ) const { 
    StringBox sb; checkDOMop(SciterGetNthAttributeNameCB(self.handle,index,&LPCSTR_receiver,cast(LPVOID)&sb));
    WStringBox wsb; checkDOMop(SciterGetNthAttributeValueCB(self.handle,index,&LPCWSTR_receiver,cast(LPVOID)&wsb));
    return tuple(sb.value,wsb.value);
  }

  /// Getter: auto value = element.attributes["id"];
  wstring opIndex(string name ) const { 
    WStringBox wsb; 
    checkDOMop(SciterGetAttributeByNameCB(self.handle, name.toStringz, &LPCWSTR_receiver,cast(LPVOID)&wsb)); 
    return wsb.value; }

  /// Setter: element.attributes["role"] = "window-close"w; changes or creates attribute with the name
  void opIndexAssign(wstring val, string name ) { 
    checkDOMop(SciterSetAttributeByName(self.handle, name.toStringz, val.toWstringz)); 
  }
  /// foreach (name, value; element.attributes) { writeln("attribute ",name,": ",value); }
  int opApply(int delegate(ref string, ref wstring) dg) {
    uint count = length();
    for (uint n = 0; n < count; ++n) {
      Tuple!(string, wstring) nv = this[n];
      auto result = dg(nv[0], nv[1]);
      if (result) // non-zero means break
        return result;
    }
    return 0;
  }
  /// clear all attributes 
  void clear() {
    checkDOMop(SciterClearAttributes(self.handle)); 
  }

  uint length() const { 
    uint count; 
    checkDOMop(SciterGetAttributeCount(self.handle,&count)); 
    return count; 
  }
}

struct Node {
  HNODE handle;

  this(HNODE h) { handle = h; if(handle) checkDOMop(SciterNodeAddRef(handle)); }
  this(ref return scope const Node e) { this(e.handle); }
  ~this() { if(handle) checkDOMop(SciterNodeRelease(handle)); }

  bool     opCast(T : bool)() const { return handle != null; }
  HNODE    opCast(T : HNODE)() const { return handle; }
  HELEMENT opCast(T : HELEMENT)() const { HELEMENT he; checkDOMop(SciterNodeCastToElement(handle,&he)); return he; }
  Element  opCast(T : Element)() const { HELEMENT he; checkDOMop(SciterNodeCastToElement(handle,&he)); return Element(he); }

  enum NODE_TYPE : UINT {
      UNKNOWN = 0,
      ELEMENT =  1,
      TEXT_NODE = 3, 
      COMMENT = 8, 
      DOCUMENT = 9, 
      DOCUMENT_FRAGMENT = 11,
  }

  NODE_TYPE type() const { UINT t; checkDOMop(SciterNodeType(handle,&t)); return cast(NODE_TYPE)t; }
  Element   parent() const { HELEMENT hp; checkDOMop(SciterNodeParent(handle,&hp)); return Element(hp); }
  
  wstring text() const { WStringBox pb; checkDOMop(SciterNodeGetText(handle,&LPCWSTR_receiver,cast(LPVOID)&pb)); return pb.value; }
  void    text(wstring s) { checkDOMop(SciterNodeSetText(handle,cast(LPCWSTR)s.ptr,cast(UINT)s.length)); }

  Node firstChild() const { HNODE hn; checkDOMop(SciterNodeFirstChild(handle,&hn)); return Node(hn); }
  Node lastChild() const { HNODE hn; checkDOMop(SciterNodeLastChild(handle,&hn)); return Node(hn); }
  Node nextSibling() const { HNODE hn; checkDOMop(SciterNodeNextSibling(handle,&hn)); return Node(hn); }
  Node prevSibling() const { HNODE hn; checkDOMop(SciterNodePrevSibling(handle,&hn)); return Node(hn); }

  NodeNodes children() { return NodeNodes(handle); }

  enum NODE_INSERT
  {
    BEFORE,  /// before this one
    AFTER,   /// after this one
    APPEND,  /// as a last child of this one
    PREPEND, /// as a first child of this one
  };

  /// insert the node in the DOM tree relative to this one
  void insert(Node node, NODE_INSERT where ) { checkDOMop(SciterNodeInsert(handle, where, node.handle)); }

  /// remove the node from the DOM completely
  void remove() { checkDOMop(SciterNodeRemove(handle, 1)); }
  /// detach the node from the DOM but keep internal states, the node is supposed to be insert'ed later in other DOM place
  void detach() { checkDOMop(SciterNodeRemove(handle, 0)); }
  

  /// create text node
  static Node createText(wstring text) { HNODE hn; SciterCreateTextNode(cast(LPCWSTR)text.ptr,cast(UINT)text.length,&hn); return Node(hn); }
  /// create comment node
  static Node createComment(wstring text) { HNODE hn; SciterCreateCommentNode(cast(LPCWSTR)text.ptr,cast(UINT)text.length,&hn); return Node(hn); }

}

struct NodeNodes {
  protected HNODE handle;
  protected this(HNODE h) { handle = h; if(handle) checkDOMop(SciterNodeAddRef(handle)); }
  ~this() { if(handle) checkDOMop(SciterNodeRelease(handle)); }
 
  bool opCast(T : bool)() const { return handle != null && length() > 0; }

  Node opIndex(int index ) const { HNODE hn; checkDOMop(SciterNodeNthChild(handle,index, &hn)); return Node(hn); }

  int opApply(int delegate(ref uint, ref Node) dg) {
    uint count = length();
    for (uint n = 0; n < count; ++n) {
      Node child = this[n];
      auto result = dg(n, child);
      if (result) // non-zero means break
        return result;
    }
    return 0;
  }

  uint length() const { uint count; checkDOMop(SciterNodeChildrenCount(handle,&count)); return count; }
}


struct StringBox { string value; }

extern(System) void LPCSTR_receiver(LPCSTR pc, UINT nc, LPVOID param) {
  auto pb = cast(StringBox*)param;
  pb.value = pc[0 .. nc].idup;
}

struct WStringBox { wstring value; }

extern(System) void LPCWSTR_receiver(LPCWSTR pc, UINT nc, LPVOID param) {
  auto pb = cast(WStringBox*)param;
  pb.value = pc[0 .. nc].idup;
}


struct BytesBox { ubyte[] value; }

extern(System) void LPCBYTES_receiver(LPCBYTE pb, UINT nb, LPVOID param) {
  auto pbox = cast(BytesBox*)param;
  pbox.value = pb[0 .. nb].dup;
}

struct ElementsBox { Element[] list; }

extern(System) SBOOL ELEMENTS_receiver(HELEMENT he, LPVOID param) {
  auto pbox = cast(ElementsBox*)param;
  pbox.list ~= Element(he);
  return 1; // non-zero means continue 
}

extern(System) SBOOL ELEMENT_receiver(HELEMENT he, LPVOID param) {
  auto pbox = cast(ElementsBox*)param;
  pbox.list ~= Element(he);
  return 0; // non-zero means continue
}
