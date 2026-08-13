module sciter.dom.api;

import sciter.types;
import sciter.api;
import sciter.utils.cvt;
public import sciter.dom.types;
public import sciter.dom.events.types;

enum SCDOM_RESULT : UINT
{
  OK_NOT_HANDLED = cast(UINT)-1, // ok, but not handled
  OK = 0,
  INVALID_HWINDOW = 1, // invalid window handle
  INVALID_HANDLE = 2, // invalid DOM handle
  PASSIVE_HANDLE = 3, // not connected to the DOM
  INVALID_PARAMETER = 4, // invalid call parameter
  OPERATION_FAILED = 5,
}

void checkDOMop(SCDOM_RESULT rv) {
  switch( rv ) {
    case SCDOM_RESULT.OK: break;
    case SCDOM_RESULT.OK_NOT_HANDLED: break;
    case SCDOM_RESULT.INVALID_HWINDOW: new Exception("Invalid window handle"); break;
    case SCDOM_RESULT.PASSIVE_HANDLE: new Exception("Element out of DOM"); break;
    case SCDOM_RESULT.INVALID_PARAMETER: new Exception("Invalid parameter"); break;
    case SCDOM_RESULT.OPERATION_FAILED: new Exception("Operation failed"); break;
    default: assert(0);
  }
}


alias LPCSTR_RECEIVER = extern(System) void function(LPCSTR str, UINT str_length, LPVOID param);
alias LPCWSTR_RECEIVER = extern(System) void function(LPCWSTR str, UINT str_length, LPVOID param);
alias LPCBYTE_RECEIVER = extern(System) void function(LPCBYTE str, UINT num_bytes, LPVOID param);
alias ELEMENT_RECEIVER = extern(System) SBOOL function ( HELEMENT he, LPVOID param );

alias Sciter_UseElementF = extern(System) SCDOM_RESULT function (HELEMENT he);
alias Sciter_UnuseElementF = extern(System) SCDOM_RESULT function (HELEMENT he);
alias SciterGetRootElementF = extern(System) SCDOM_RESULT function (HWINDOW hwnd, HELEMENT *phe);
alias SciterGetFocusElementF = extern(System) SCDOM_RESULT function (HWINDOW hwnd, HELEMENT *phe);
alias SciterFindElementF = extern(System) SCDOM_RESULT function (HWINDOW hwnd, POINT pt, HELEMENT *phe);
alias SciterGetChildrenCountF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT *count);
alias SciterGetNthChildF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT n, HELEMENT *phe);
alias SciterGetParentElementF = extern(System) SCDOM_RESULT function (HELEMENT he, HELEMENT *p_parent_he);
alias SciterGetElementHtmlCBF = extern(System) SCDOM_RESULT function (HELEMENT he, SBOOL outer, LPCBYTE_RECEIVER rcv, LPVOID rcv_param);
alias SciterGetElementTextCBF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCWSTR_RECEIVER rcv, LPVOID rcv_param);
alias SciterSetElementTextF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCWSTR utf16, UINT length);
alias SciterGetAttributeCountF = extern(System) SCDOM_RESULT function (HELEMENT he, LPUINT p_count);
alias SciterGetNthAttributeNameCBF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT n, LPCSTR_RECEIVER rcv, LPVOID rcv_param);
alias SciterGetNthAttributeValueCBF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT n, LPCWSTR_RECEIVER rcv, LPVOID rcv_param);
alias SciterGetAttributeByNameCBF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCSTR name, LPCWSTR_RECEIVER rcv, LPVOID rcv_param);
alias SciterSetAttributeByNameF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCSTR name, LPCWSTR value);
alias SciterClearAttributesF = extern(System) SCDOM_RESULT function (HELEMENT he);
alias SciterGetElementIndexF = extern(System) SCDOM_RESULT function (HELEMENT he, LPUINT p_index);
alias SciterGetElementTypeF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCSTR *p_type);
alias SciterGetElementTypeCBF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCSTR_RECEIVER rcv, LPVOID rcv_param);
alias SciterGetStyleAttributeCBF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCSTR name, LPCWSTR_RECEIVER rcv, LPVOID rcv_param);
alias SciterSetStyleAttributeF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCSTR name, LPCWSTR value);
alias SciterGetElementLocationF = extern(System) SCDOM_RESULT function (HELEMENT he, LPRECT p_location, UINT areas /*ELEMENT_AREAS*/);
alias SciterScrollToViewF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT SciterScrollFlags);
alias SciterUpdateElementF = extern(System) SCDOM_RESULT function (HELEMENT he, SBOOL andForceRender);
alias SciterRefreshElementAreaF = extern(System) SCDOM_RESULT function (HELEMENT he, RECT rc);
alias SciterSetCaptureF = extern(System) SCDOM_RESULT function (HELEMENT he);
alias SciterReleaseCaptureF = extern(System) SCDOM_RESULT function (HELEMENT he);
alias SciterGetElementHwndF = extern(System) SCDOM_RESULT function (HELEMENT he, HWINDOW *p_hwnd, SBOOL rootWindow);
alias SciterCombineURLF = extern(System) SCDOM_RESULT function (HELEMENT he, LPWSTR szUrlBuffer, UINT UrlBufferSize);
alias SciterSelectElementsF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCSTR CSS_selectors, ELEMENT_RECEIVER receiver, LPVOID param);
alias SciterSelectParentF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCSTR selector, UINT depth, HELEMENT *heFound);
alias SciterSetElementHtmlF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCBYTE html, UINT htmlLength, UINT where);
alias SciterGetElementUIDF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT *puid);
alias SciterGetElementByUIDF = extern(System) SCDOM_RESULT function (HWINDOW hwnd, UINT uid, HELEMENT *phe);
alias SciterShowPopupF = extern(System) SCDOM_RESULT function (HELEMENT hePopup, HELEMENT heAnchor, UINT placement);
alias SciterShowPopupAtF = extern(System) SCDOM_RESULT function (HELEMENT hePopup, POINT pos, UINT placement);
alias SciterHidePopupF = extern(System) SCDOM_RESULT function (HELEMENT he);
alias SciterGetElementStateF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT *pstateBits);
alias SciterSetElementStateF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT stateBitsToSet, UINT stateBitsToClear, SBOOL updateView);
alias SciterCreateElementF = extern(System) SCDOM_RESULT function (LPCSTR tagname, LPCWSTR textOrNull, /*out*/ HELEMENT *phe);
alias SciterCloneElementF = extern(System) SCDOM_RESULT function (HELEMENT he, /*out*/ HELEMENT *phe);
alias SciterInsertElementF = extern(System) SCDOM_RESULT function (HELEMENT he, HELEMENT hparent, UINT index);
alias SciterDetachElementF = extern(System) SCDOM_RESULT function (HELEMENT he);
alias SciterDeleteElementF = extern(System) SCDOM_RESULT function (HELEMENT he);
alias SciterSetTimerF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT milliseconds, UINT_PTR timer_id);
alias SciterSendEventF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT appEventCode, HELEMENT heSource, UINT_PTR reason,/*out*/ SBOOL *handled);
alias SciterPostEventF = extern(System) SCDOM_RESULT function (HELEMENT he, UINT appEventCode, HELEMENT heSource, UINT_PTR reason);
alias SciterFireEventF = extern(System) SCDOM_RESULT function (const SEMANTIC_EVENT *pevt, SBOOL post, SBOOL *handled);
alias SciterGetScrollInfoF = extern(System) SCDOM_RESULT function (HELEMENT he, LPPOINT scrollPos, LPRECT viewRect, LPSIZE contentSize);
alias SciterSetScrollPosF = extern(System) SCDOM_RESULT function (HELEMENT he, POINT scrollPos, SBOOL smooth);
alias SciterGetElementIntrinsicWidthsF = extern(System) SCDOM_RESULT function (HELEMENT he, INT *pMinWidth, INT *pMaxWidth);
alias SciterGetElementIntrinsicHeightF = extern(System) SCDOM_RESULT function (HELEMENT he, INT forWidth, INT *pHeight);
alias SciterIsElementVisibleF = extern(System) SCDOM_RESULT function (HELEMENT he, SBOOL *pVisible);
alias SciterIsElementEnabledF = extern(System) SCDOM_RESULT function (HELEMENT he, SBOOL *pEnabled);
alias SciterTraverseUIEventF = extern(System) SCDOM_RESULT function (UINT evt, LPVOID eventCtlStruct, SBOOL *bOutProcessed);
alias SciterCallScriptingMethodF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCSTR name, const VALUE *argv, UINT argc, VALUE *retval);
alias SciterCallScriptingFunctionF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCSTR name, const VALUE *argv, UINT argc, VALUE *retval);
alias SciterEvalElementScriptF = extern(System) SCDOM_RESULT function (HELEMENT he, LPCWSTR script, UINT scriptLength, VALUE *retval);
alias SciterAttachHwndToElementF = extern(System) SCDOM_RESULT function (HELEMENT he, HWINDOW hwnd);
alias SciterControlGetTypeF = extern(System) SCDOM_RESULT function (HELEMENT he, /*CTL_TYPE*/ UINT *pType);
alias SciterGetValueF = extern(System) SCDOM_RESULT function (HELEMENT he, VALUE *pval);
alias SciterSetValueF = extern(System) SCDOM_RESULT function (HELEMENT he, const VALUE *pval);
alias SciterGetExpandoF = extern(System) SCDOM_RESULT function (HELEMENT he, VALUE *pval, SBOOL forceCreation);
alias SciterGetObjectF = extern(System) SCDOM_RESULT function (HELEMENT he, void *pval, SBOOL forceCreation);
alias SciterGetElementNamespaceF = extern(System) SCDOM_RESULT function (HELEMENT he, void* pval);
alias SciterGetHighlightedElementF = extern(System) SCDOM_RESULT function (HWINDOW hwnd, HELEMENT *phe);
alias SciterSetHighlightedElementF = extern(System) SCDOM_RESULT function (HWINDOW hwnd, HELEMENT he);
alias SciterRequestPaintF = extern(System) SCDOM_RESULT function (HELEMENT he);

//|
//| DOM Node API
//|
alias  SciterNodeAddRefF = extern(System) SCDOM_RESULT function (HNODE hn);
alias  SciterNodeReleaseF = extern(System) SCDOM_RESULT function (HNODE hn);
alias  SciterNodeCastFromElementF = extern(System) SCDOM_RESULT function (HELEMENT he, HNODE* phn);
alias  SciterNodeCastToElementF = extern(System) SCDOM_RESULT function (HNODE hn, HELEMENT* he);
alias  SciterNodeFirstChildF = extern(System) SCDOM_RESULT function (HNODE hn, HNODE* phn);
alias  SciterNodeLastChildF = extern(System) SCDOM_RESULT function (HNODE hn, HNODE* phn);
alias  SciterNodeNextSiblingF = extern(System) SCDOM_RESULT function (HNODE hn, HNODE* phn);
alias  SciterNodePrevSiblingF = extern(System) SCDOM_RESULT function (HNODE hn, HNODE* phn);
alias  SciterNodeParentF = extern(System) SCDOM_RESULT function (HNODE hnode, HELEMENT* pheParent);
alias  SciterNodeNthChildF = extern(System) SCDOM_RESULT function (HNODE hnode, UINT n, HNODE* phn);
alias  SciterNodeChildrenCountF = extern(System) SCDOM_RESULT function (HNODE hnode, UINT* pn);
alias  SciterNodeTypeF = extern(System) SCDOM_RESULT function (HNODE hnode, UINT* pNodeType /*NODE_TYPE*/);
alias  SciterNodeGetTextF = extern(System) SCDOM_RESULT function (HNODE hnode, LPCWSTR_RECEIVER rcv, LPVOID rcv_param);
alias  SciterNodeSetTextF = extern(System) SCDOM_RESULT function (HNODE hnode, LPCWSTR text, UINT textLength);
alias  SciterNodeInsertF = extern(System) SCDOM_RESULT function (HNODE hnode, UINT where /*NODE_INS_TARGET*/, HNODE what);
alias  SciterNodeRemoveF = extern(System) SCDOM_RESULT function (HNODE hnode, SBOOL finalize);
alias  SciterCreateTextNodeF = extern(System) SCDOM_RESULT function (LPCWSTR text, UINT textLength, HNODE* phnode);
alias  SciterCreateCommentNodeF = extern(System) SCDOM_RESULT function (LPCWSTR text, UINT textLength, HNODE* phnode);

//|
//| DOM Element API
//|

Sciter_UseElementF Sciter_UseElement;
Sciter_UnuseElementF Sciter_UnuseElement;
SciterGetRootElementF SciterGetRootElement;
SciterGetFocusElementF SciterGetFocusElement;
SciterFindElementF SciterFindElement;
SciterGetChildrenCountF SciterGetChildrenCount;
SciterGetNthChildF SciterGetNthChild;
SciterGetParentElementF SciterGetParentElement;
SciterGetElementHtmlCBF SciterGetElementHtmlCB;
SciterGetElementTextCBF SciterGetElementTextCB;
SciterSetElementTextF SciterSetElementText;
SciterGetAttributeCountF SciterGetAttributeCount;
SciterGetNthAttributeNameCBF SciterGetNthAttributeNameCB;
SciterGetNthAttributeValueCBF SciterGetNthAttributeValueCB;
SciterGetAttributeByNameCBF SciterGetAttributeByNameCB;
SciterSetAttributeByNameF SciterSetAttributeByName;
SciterClearAttributesF SciterClearAttributes;
SciterGetElementIndexF SciterGetElementIndex;
SciterGetElementTypeF SciterGetElementType;
SciterGetElementTypeCBF SciterGetElementTypeCB;
SciterGetStyleAttributeCBF SciterGetStyleAttributeCB;
SciterSetStyleAttributeF SciterSetStyleAttribute;
SciterGetElementLocationF SciterGetElementLocation;
SciterScrollToViewF SciterScrollToView;
SciterUpdateElementF SciterUpdateElement;
SciterRefreshElementAreaF SciterRefreshElementArea;
SciterSetCaptureF SciterSetCapture;
SciterReleaseCaptureF SciterReleaseCapture;
SciterGetElementHwndF SciterGetElementHwnd;
SciterCombineURLF SciterCombineURL;
SciterSelectElementsF SciterSelectElements;
SciterSelectParentF SciterSelectParent;
SciterSetElementHtmlF SciterSetElementHtml;
SciterGetElementUIDF SciterGetElementUID;
SciterGetElementByUIDF SciterGetElementByUID;
SciterShowPopupF SciterShowPopup;
SciterShowPopupAtF SciterShowPopupAt;
SciterHidePopupF SciterHidePopup;
SciterGetElementStateF SciterGetElementState;
SciterSetElementStateF SciterSetElementState;
SciterCreateElementF SciterCreateElement;
SciterCloneElementF SciterCloneElement;
SciterInsertElementF SciterInsertElement;
SciterDetachElementF SciterDetachElement;
SciterDeleteElementF SciterDeleteElement;
SciterSetTimerF SciterSetTimer;
SciterDetachEventHandlerF SciterDetachEventHandler;
SciterAttachEventHandlerF SciterAttachEventHandler;
SciterSendEventF SciterSendEvent;
SciterPostEventF SciterPostEvent;
SciterFireEventF SciterFireEvent;
//SciterCallBehaviorMethodF SciterCallBehaviorMethod;
//SciterRequestElementDataF SciterRequestElementData;
//SciterHttpRequestF SciterHttpRequest;
SciterGetScrollInfoF SciterGetScrollInfo;
SciterSetScrollPosF SciterSetScrollPos;
SciterGetElementIntrinsicWidthsF SciterGetElementIntrinsicWidths;
SciterGetElementIntrinsicHeightF SciterGetElementIntrinsicHeight;
SciterIsElementVisibleF SciterIsElementVisible;
SciterIsElementEnabledF SciterIsElementEnabled;
//SciterSortElementsF SciterSortElements;
//SciterSwapElementsF SciterSwapElements;
SciterTraverseUIEventF SciterTraverseUIEvent;
SciterCallScriptingMethodF SciterCallScriptingMethod;
SciterCallScriptingFunctionF SciterCallScriptingFunction;
SciterEvalElementScriptF SciterEvalElementScript;
SciterAttachHwndToElementF SciterAttachHwndToElement;
SciterControlGetTypeF SciterControlGetType;
SciterGetValueF SciterGetValue;
SciterSetValueF SciterSetValue;
SciterGetExpandoF SciterGetExpando;
SciterGetObjectF SciterGetObject;
SciterGetElementNamespaceF SciterGetElementNamespace;
SciterGetHighlightedElementF SciterGetHighlightedElement;
SciterSetHighlightedElementF SciterSetHighlightedElement;
SciterRequestPaintF SciterRequestPaint;


//|
//| DOM Node API
//|
SciterNodeAddRefF  SciterNodeAddRef;
SciterNodeReleaseF  SciterNodeRelease;
SciterNodeCastFromElementF  SciterNodeCastFromElement;
SciterNodeCastToElementF  SciterNodeCastToElement;
SciterNodeFirstChildF  SciterNodeFirstChild;
SciterNodeLastChildF  SciterNodeLastChild;
SciterNodeNextSiblingF  SciterNodeNextSibling;
SciterNodePrevSiblingF  SciterNodePrevSibling;
SciterNodeParentF  SciterNodeParent;
SciterNodeNthChildF  SciterNodeNthChild;
SciterNodeChildrenCountF  SciterNodeChildrenCount;
SciterNodeTypeF  SciterNodeType;
SciterNodeGetTextF  SciterNodeGetText;
SciterNodeSetTextF  SciterNodeSetText;
SciterNodeInsertF  SciterNodeInsert;
SciterNodeRemoveF  SciterNodeRemove;
SciterCreateTextNodeF  SciterCreateTextNode;
SciterCreateCommentNodeF  SciterCreateCommentNode;

