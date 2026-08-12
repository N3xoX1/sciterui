
module sciter.api;

import core.runtime;
import std.stdio;
import std.string;
import std.traits;
import std.exception;
import std.typecons;
import std.file : thisExePath;
import std.path : dirName;

public import sciter.om.value;
public import sciter.types;
public import sciter.dom.types;
public import sciter.dom.api;
public import sciter.graphics.types;
public import sciter.graphics.api;

public enum SCITER_APP_CMD {
  STOP = 0,     /// reuest to quit message pump loop
  LOOP = 1,     /// run message pump loop until SCITER_APP_STOP or main window closure
  INIT = 2,     /// pass argc/argv to application: p1 - argc, p2 - CHAR** argv 
  SHUTDOWN = 3, /// free resources of the application 
  RUN  = 4,     /// scapp mode: load JS and run message pump loop until SCITER_APP_STOP or main window closure, p1 - JS url, p2 - 0 or SciterPrimordialLoader; 
  LOOP_ITERATION = 6, /// does single message pump loop iteration, SCITER_APP_LOOP is essentially this:
                                 /// while( SciterExec(SCITER_APP_LOOP_ITERATION,0,0) );
  LOOP_HEARTBIT = 7,  /// checks outstanding tasks and timers,
}

struct SCITER_CALLBACK_NOTIFICATION /// base struct for all notifications
{
  UINT code;    /// [in] one of the codes above.
  HWINDOW hwnd; /// [in] HWINDOW of the window this callback was attached to.
}

version (Windows) {
    import core.sys.windows.windows;
} else version (Posix) {
    import core.sys.posix.dlfcn;
}

alias SciterHostCallbackF = extern(System) UINT function(SCITER_CALLBACK_NOTIFICATION* pns, LPVOID param); 
alias ElementEventProcF = extern(System) SBOOL function(LPVOID tag, HELEMENT he, UINT evtg, LPVOID param);

alias SciterExecF = extern(System) INT_PTR function(UINT appCmd, UINT_PTR p1, UINT_PTR p2);

alias SciterCreateWindowF = extern(System) HWINDOW function( UINT creationFlags, sciter.types.LPRECT frame, LPVOID, LPVOID, HWINDOW parent);
alias SciterLoadFileF = extern(System) SBOOL function(HWINDOW hWndSciter, LPCWSTR filename);
alias SciterLoadHtmlF = extern(System) SBOOL function(HWINDOW hWndSciter, LPCBYTE html, UINT htmlSize, LPCWSTR baseUrl);

alias SciterSetCallbackF = extern(System) VOID function(HWINDOW hWndSciter, SciterHostCallbackF cb, LPVOID cbParam);
alias SciterWindowExecF = extern(System) INT_PTR function(HWINDOW hwnd, UINT wndCmd, UINT_PTR p1, UINT_PTR p2);

alias SciterDetachEventHandlerF = extern(System) SCDOM_RESULT function(HELEMENT he, ElementEventProcF pep, LPVOID tag);
alias SciterAttachEventHandlerF = extern(System) SCDOM_RESULT function(HELEMENT he, ElementEventProcF pep, LPVOID tag);  

alias SciterWindowDetachEventHandlerF = extern(System) SCDOM_RESULT function(HWINDOW hWndSciter, ElementEventProcF pep, LPVOID tag, UINT subscription);
alias SciterWindowAttachEventHandlerF = extern(System) SCDOM_RESULT function(HWINDOW hWndSciter, ElementEventProcF pep, LPVOID tag, UINT subscription);

alias SciterAtomValueF = extern(System) sciter.types.ATOM function(const char* name);

alias SciterOpenArchiveF = extern(System) HARCHIVE function(LPCBYTE archiveData, UINT archiveDataLength);
alias SciterGetArchiveItemF = extern(System) SBOOL function(HARCHIVE harc, LPCWSTR path, LPCBYTE* pdata, UINT* pdataLength);
alias SciterCloseArchiveF = extern(System) SBOOL function(HARCHIVE harc);

alias SciterDataReadyF = extern(System) SBOOL function(HWINDOW hwnd,LPCWSTR uri,LPCBYTE data, UINT dataLength);

alias SciterSetVariableF = extern(System) SBOOL function(HWINDOW hwndOrNull, LPCSTR name, const VALUE* pvalToSet);
alias SciterGetVariableF = extern(System) SBOOL function(HWINDOW hwndOrNull, LPCSTR name, VALUE* pvalToGet);

enum SCRIPT_RUNTIME_FEATURE
{
  ALLOW_FILE_IO = 0x00000001,
  ALLOW_SOCKET_IO = 0x00000002,
  ALLOW_EVAL = 0x00000004,
  ALLOW_SYSINFO = 0x00000008,
  ALLOW_CMODULES = 0x00000010
}

enum SCITER_RT_OPTION
{
   SMOOTH_SCROLL = 1,      // value:TRUE - enable, value:FALSE - disable, enabled by default
   CONNECTION_TIMEOUT = 2, // value: milliseconds, connection timeout of http client
   HTTPS_ERROR = 3,        // value: 0 - drop connection, 1 - use builtin dialog, 2 - accept connection silently

   SET_SCRIPT_RUNTIME_FEATURES = 8, // value - combination of SCRIPT_RUNTIME_FEATURES flags.
   SET_GFX_LAYER = 9,      // hWnd = NULL, value - GFX_LAYER
   SET_DEBUG_MODE = 10,    // hWnd, value - TRUE/FALSE

   SET_INIT_SCRIPT = 13,   // hWnd - N/A , value LPCSTR - UTF-8 encoded script source to be loaded into each view before any other script execution.
                           //                             The engine copies this string inside the call.

   SET_MAX_HTTP_DATA_LENGTH = 15, // hWnd - N/A , value - max request length in megabytes (1024*1024 bytes)

   SET_PX_AS_DIP = 16, // value 1 - 1px in CSS is treated as 1dip, value 0 - default behavior - 1px is a physical pixel 

   ENABLE_UIAUTOMATION = 17,  // hWnd - N/A , TRUE/FALSE, enables UIAutomation support. 

   USE_INTERNAL_HTTP_CLIENT = 18,  // hWnd - N/A , TRUE - use internal HTTP client
                                   //              FALSE - use system HTTP client (on platforms that has it: Win,Mac,Lin)
   EXTENDED_TOUCHPAD_SUPPORT = 19, // hWnd - N/A , TRUE/FALSE, enables/disables extended touchpad support, TRUE by default   

   SET_ROOT_CA = 21,               // hWnd - N/A , const char* certificates, in format acceptable by mbedtls_x509_crt_parse()
}

alias SciterSetOptionF = extern(System) SBOOL function(HWINDOW hWndOrNull, UINT option, UINT_PTR value );


SciterExecF         SciterExec;
SciterCreateWindowF SciterCreateWindow;
SciterLoadFileF     SciterLoadFile;
SciterLoadHtmlF     SciterLoadHtml;
SciterSetCallbackF  SciterSetCallback;
SciterWindowExecF   SciterWindowExec;

SciterDetachEventHandlerF SciterDetachEventHandler;
SciterAttachEventHandlerF SciterAttachEventHandler;
SciterWindowDetachEventHandlerF SciterWindowDetachEventHandler;
SciterWindowAttachEventHandlerF SciterWindowAttachEventHandler;

SciterOpenArchiveF SciterOpenArchive;
SciterGetArchiveItemF SciterGetArchiveItem;
SciterCloseArchiveF SciterCloseArchive;

SciterAtomValueF SciterAtomValue;

SciterDataReadyF SciterDataReady;

SciterSetVariableF SciterSetVariable;
SciterGetVariableF SciterGetVariable;
SciterSetOptionF   SciterSetOption;

protected void* libHandle = null;

protected void getProc(F)(ref F f, string name) {

  version (Windows) {
    auto addr = GetProcAddress(cast(HMODULE) libHandle, name.toStringz());
  }
  else version (OSX) {
    auto addr = dlsym(libHandle, name.toStringz);
  }
  else version (Posix) {
    //string _name = "_" ~ name;
    auto addr = dlsym(libHandle, name.toStringz);
  }
  enforce(addr !is null, "Failed to load symbol:" ~ name);    
  f = cast(F) addr;
}

bool loadApi() {

  version(Windows) { string libName = "sciter.dll"; }
  else version (OSX) { string libName = "libsciter.dylib"; }
  else version(Posix) { string libName = "libsciter.so"; }

  try
  {
    // Load the library
    version(Posix) {
      string exePath = thisExePath();
      string fullPath = dirName(exePath) ~ "/" ~ libName;
      libHandle = dlopen(fullPath.toStringz,RTLD_LOCAL | RTLD_LAZY);
    } else version(Windows) { 
      libHandle = Runtime.loadLibrary(libName);
    }
    enforce(libHandle !is null, "Failed to load library: " ~ libName);

    getProc(SciterExec,"SciterExecImp");
    getProc(SciterLoadFile,"SciterLoadFileImp");
    getProc(SciterLoadHtml,"SciterLoadHtmlImp");
    getProc(SciterCreateWindow, "SciterCreateWindowImp");
    getProc(SciterSetCallback, "SciterSetCallbackImp" );
    getProc(SciterWindowExec, "SciterWindowExecImp" );

    getProc(SciterDetachEventHandler, "SciterDetachEventHandlerImp" );
    getProc(SciterAttachEventHandler, "SciterAttachEventHandlerImp" );
    getProc(SciterWindowDetachEventHandler, "SciterWindowDetachEventHandlerImp" );
    getProc(SciterWindowAttachEventHandler, "SciterWindowAttachEventHandlerImp" );

    getProc(SciterAtomValue, "SciterAtomValueImp" );

    getProc(SciterOpenArchive,"SciterOpenArchiveImp");
    getProc(SciterGetArchiveItem,"SciterGetArchiveItemImp");
    getProc(SciterCloseArchive,"SciterCloseArchiveImp");

    getProc(SciterDataReady,"SciterDataReadyImp");

    getProc(SciterSetVariable,"SciterSetVariableImp");
    getProc(SciterGetVariable,"SciterGetVariableImp");

    getProc(SciterSetOption,"SciterSetOptionImp");

    getProc(sciter.om.value.ValueInit,"ValueInitImp"); 
    getProc(sciter.om.value.ValueClear,"ValueClearImp");
    getProc(sciter.om.value.ValueCompare,"ValueCompareImp");
    getProc(sciter.om.value.ValueCopy,"ValueCopyImp");
    getProc(sciter.om.value.ValueIsolate,"ValueIsolateImp");
    getProc(sciter.om.value.ValueType,"ValueTypeImp");
    getProc(sciter.om.value.ValueStringData,"ValueStringDataImp");
    getProc(sciter.om.value.ValueStringDataSet,"ValueStringDataSetImp");
    getProc(sciter.om.value.ValueIntData,"ValueIntDataImp");
    getProc(sciter.om.value.ValueIntDataSet,"ValueIntDataSetImp");
    getProc(sciter.om.value.ValueInt64Data,"ValueInt64DataImp");
    getProc(sciter.om.value.ValueInt64DataSet,"ValueInt64DataSetImp");
    getProc(sciter.om.value.ValueFloatData,"ValueFloatDataImp");
    getProc(sciter.om.value.ValueFloatDataSet,"ValueFloatDataSetImp");
    getProc(sciter.om.value.ValueBinaryData,"ValueBinaryDataImp");
    getProc(sciter.om.value.ValueBinaryDataSet,"ValueBinaryDataSetImp");
    getProc(sciter.om.value.ValueElementsCount,"ValueElementsCountImp");
    getProc(sciter.om.value.ValueNthElementValue,"ValueNthElementValueImp");
    getProc(sciter.om.value.ValueNthElementValueSet,"ValueNthElementValueSetImp");
    getProc(sciter.om.value.ValueEnumElements,"ValueEnumElementsImp");
    getProc(sciter.om.value.ValueSetValueToKey,"ValueSetValueToKeyImp");
    getProc(sciter.om.value.ValueGetValueOfKey,"ValueGetValueOfKeyImp");
    getProc(sciter.om.value.ValueToString,"ValueToStringImp");
    getProc(sciter.om.value.ValueFromString,"ValueFromStringImp");
    getProc(sciter.om.value.ValueInvoke,"ValueInvokeImp");
    getProc(sciter.om.value.ValueNativeFunctorSet,"ValueNativeFunctorSetImp");
    getProc(sciter.om.value.ValueIsNativeFunctor,"ValueIsNativeFunctorImp");

    getProc(sciter.dom.Sciter_UseElement,"Sciter_UseElementImp");
    getProc(sciter.dom.Sciter_UnuseElement,"Sciter_UnuseElementImp");
    getProc(sciter.dom.SciterGetRootElement,"SciterGetRootElementImp");
    getProc(sciter.dom.SciterGetFocusElement,"SciterGetFocusElementImp");
    getProc(sciter.dom.SciterFindElement,"SciterFindElementImp");
    getProc(sciter.dom.SciterGetChildrenCount,"SciterGetChildrenCountImp");
    getProc(sciter.dom.SciterGetNthChild,"SciterGetNthChildImp");
    getProc(sciter.dom.SciterGetParentElement,"SciterGetParentElementImp");
    getProc(sciter.dom.SciterGetElementHtmlCB,"SciterGetElementHtmlCBImp");
    getProc(sciter.dom.SciterGetElementTextCB,"SciterGetElementTextCBImp");
    getProc(sciter.dom.SciterSetElementText,"SciterSetElementTextImp");
    getProc(sciter.dom.SciterGetAttributeCount,"SciterGetAttributeCountImp");
    getProc(sciter.dom.SciterGetNthAttributeNameCB,"SciterGetNthAttributeNameCBImp");
    getProc(sciter.dom.SciterGetNthAttributeValueCB,"SciterGetNthAttributeValueCBImp");
    getProc(sciter.dom.SciterGetAttributeByNameCB,"SciterGetAttributeByNameCBImp");
    getProc(sciter.dom.SciterSetAttributeByName,"SciterSetAttributeByNameImp");
    getProc(sciter.dom.SciterClearAttributes,"SciterClearAttributesImp");
    getProc(sciter.dom.SciterGetElementIndex,"SciterGetElementIndexImp");
    getProc(sciter.dom.SciterGetElementType,"SciterGetElementTypeImp");
    getProc(sciter.dom.SciterGetElementTypeCB,"SciterGetElementTypeCBImp");
    getProc(sciter.dom.SciterGetStyleAttributeCB,"SciterGetStyleAttributeCBImp");
    getProc(sciter.dom.SciterSetStyleAttribute,"SciterSetStyleAttributeImp");
    getProc(sciter.dom.SciterGetElementLocation,"SciterGetElementLocationImp");
    getProc(sciter.dom.SciterScrollToView,"SciterScrollToViewImp");
    getProc(sciter.dom.SciterUpdateElement,"SciterUpdateElementImp");
    getProc(sciter.dom.SciterRefreshElementArea,"SciterRefreshElementAreaImp");
    getProc(sciter.dom.SciterSetCapture,"SciterSetCaptureImp");
    getProc(sciter.dom.SciterReleaseCapture,"SciterReleaseCaptureImp");
    getProc(sciter.dom.SciterGetElementHwnd,"SciterGetElementHwndImp");
    getProc(sciter.dom.SciterCombineURL,"SciterCombineURLImp");
    getProc(sciter.dom.SciterSelectElements,"SciterSelectElementsImp");
    getProc(sciter.dom.SciterSelectParent,"SciterSelectParentImp");
    getProc(sciter.dom.SciterSetElementHtml,"SciterSetElementHtmlImp");
    getProc(sciter.dom.SciterGetElementUID,"SciterGetElementUIDImp");
    getProc(sciter.dom.SciterGetElementByUID,"SciterGetElementByUIDImp");
    getProc(sciter.dom.SciterShowPopup,"SciterShowPopupImp");
    getProc(sciter.dom.SciterShowPopupAt,"SciterShowPopupAtImp");
    getProc(sciter.dom.SciterHidePopup,"SciterHidePopupImp");
    getProc(sciter.dom.SciterGetElementState,"SciterGetElementStateImp");
    getProc(sciter.dom.SciterSetElementState,"SciterSetElementStateImp");
    getProc(sciter.dom.SciterCreateElement,"SciterCreateElementImp");
    getProc(sciter.dom.SciterCloneElement,"SciterCloneElementImp");
    getProc(sciter.dom.SciterInsertElement,"SciterInsertElementImp");
    getProc(sciter.dom.SciterDetachElement,"SciterDetachElementImp");
    getProc(sciter.dom.SciterDeleteElement,"SciterDeleteElementImp");
    getProc(sciter.dom.SciterSetTimer,"SciterSetTimerImp");
    getProc(sciter.dom.SciterDetachEventHandler,"SciterDetachEventHandlerImp");
    getProc(sciter.dom.SciterAttachEventHandler,"SciterAttachEventHandlerImp");
    getProc(sciter.dom.SciterSendEvent,"SciterSendEventImp");
    getProc(sciter.dom.SciterPostEvent,"SciterPostEventImp");
    getProc(sciter.dom.SciterFireEvent,"SciterFireEventImp");
    getProc(sciter.dom.SciterGetScrollInfo,"SciterGetScrollInfoImp");
    getProc(sciter.dom.SciterSetScrollPos,"SciterSetScrollPosImp");
    getProc(sciter.dom.SciterGetElementIntrinsicWidths,"SciterGetElementIntrinsicWidthsImp");
    getProc(sciter.dom.SciterGetElementIntrinsicHeight,"SciterGetElementIntrinsicHeightImp");
    getProc(sciter.dom.SciterIsElementVisible,"SciterIsElementVisibleImp");
    getProc(sciter.dom.SciterIsElementEnabled,"SciterIsElementEnabledImp");
    getProc(sciter.dom.SciterTraverseUIEvent,"SciterTraverseUIEventImp");
    getProc(sciter.dom.SciterCallScriptingMethod,"SciterCallScriptingMethodImp");
    getProc(sciter.dom.SciterCallScriptingFunction,"SciterCallScriptingFunctionImp");
    getProc(sciter.dom.SciterEvalElementScript,"SciterEvalElementScriptImp");
    getProc(sciter.dom.SciterAttachHwndToElement,"SciterAttachHwndToElementImp");
    getProc(sciter.dom.SciterControlGetType,"SciterControlGetTypeImp");
    getProc(sciter.dom.SciterGetValue,"SciterGetValueImp");
    getProc(sciter.dom.SciterSetValue,"SciterSetValueImp");
    getProc(sciter.dom.SciterGetExpando,"SciterGetExpandoImp");
    getProc(sciter.dom.SciterGetObject,"SciterGetObjectImp");
    getProc(sciter.dom.SciterGetElementNamespace,"SciterGetElementNamespaceImp");
    getProc(sciter.dom.SciterGetHighlightedElement,"SciterGetHighlightedElementImp");
    getProc(sciter.dom.SciterSetHighlightedElement,"SciterSetHighlightedElementImp");
    getProc(sciter.dom.SciterRequestPaint,"SciterRequestPaintImp");
  

    //|
    //| DOM Node API
    //|
    getProc(sciter.dom.SciterNodeAddRef,"SciterNodeAddRefImp");
    getProc(sciter.dom.SciterNodeRelease,"SciterNodeReleaseImp");
    getProc(sciter.dom.SciterNodeCastFromElement,"SciterNodeCastFromElementImp");
    getProc(sciter.dom.SciterNodeCastToElement,"SciterNodeCastToElementImp");
    getProc(sciter.dom.SciterNodeFirstChild,"SciterNodeFirstChildImp");
    getProc(sciter.dom.SciterNodeLastChild,"SciterNodeLastChildImp");
    getProc(sciter.dom.SciterNodeNextSibling,"SciterNodeNextSiblingImp");
    getProc(sciter.dom.SciterNodePrevSibling,"SciterNodePrevSiblingImp");
    getProc(sciter.dom.SciterNodeParent,"SciterNodeParentImp");
    getProc(sciter.dom.SciterNodeNthChild,"SciterNodeNthChildImp");
    getProc(sciter.dom.SciterNodeChildrenCount,"SciterNodeChildrenCountImp");
    getProc(sciter.dom.SciterNodeType,"SciterNodeTypeImp");
    getProc(sciter.dom.SciterNodeGetText,"SciterNodeGetTextImp");
    getProc(sciter.dom.SciterNodeSetText,"SciterNodeSetTextImp");
    getProc(sciter.dom.SciterNodeInsert,"SciterNodeInsertImp");
    getProc(sciter.dom.SciterNodeRemove,"SciterNodeRemoveImp");
    getProc(sciter.dom.SciterCreateTextNode,"SciterCreateTextNodeImp");
    getProc(sciter.dom.SciterCreateCommentNode,"SciterCreateCommentNodeImp");

    //|
    //| Graphics API
    //|
    getProc(sciter.graphics.api.RGBA, "RGBAImp");
    getProc(sciter.graphics.api.GraphicsCreate, "GraphicsCreateImp");
    getProc(sciter.graphics.api.GraphicsAddRef, "GraphicsAddRefImp");
    getProc(sciter.graphics.api.GraphicsRelease, "GraphicsReleaseImp");
    getProc(sciter.graphics.api.GraphicsLine, "GraphicsLineImp");
    getProc(sciter.graphics.api.GraphicsRectangle, "GraphicsRectangleImp");
    getProc(sciter.graphics.api.GraphicsRoundedRectangle, "GraphicsRoundedRectangleImp");
    getProc(sciter.graphics.api.GraphicsEllipse, "GraphicsEllipseImp");
    getProc(sciter.graphics.api.GraphicsArc, "GraphicsArcImp");
    getProc(sciter.graphics.api.GraphicsStar, "GraphicsStarImp");
    getProc(sciter.graphics.api.GraphicsPolygon, "GraphicsPolygonImp");
    getProc(sciter.graphics.api.GraphicsPolyline, "GraphicsPolylineImp");
    getProc(sciter.graphics.api.PathCreate, "PathCreateImp");
    getProc(sciter.graphics.api.PathAddRef, "PathAddRefImp");
    getProc(sciter.graphics.api.PathRelease, "PathReleaseImp");
    getProc(sciter.graphics.api.PathMoveTo, "PathMoveToImp");
    getProc(sciter.graphics.api.PathLineTo, "PathLineToImp");
    getProc(sciter.graphics.api.PathArcTo, "PathArcToImp");
    getProc(sciter.graphics.api.PathQuadraticCurveTo, "PathQuadraticCurveToImp");
    getProc(sciter.graphics.api.PathBezierCurveTo, "PathBezierCurveToImp");
    getProc(sciter.graphics.api.PathClosePath, "PathClosePathImp");
    getProc(sciter.graphics.api.GraphicsDrawPath, "GraphicsDrawPathImp");
    getProc(sciter.graphics.api.GraphicsRotate, "GraphicsRotateImp");
    getProc(sciter.graphics.api.GraphicsTranslate, "GraphicsTranslateImp");
    getProc(sciter.graphics.api.GraphicsScale, "GraphicsScaleImp");
    getProc(sciter.graphics.api.GraphicsSkew, "GraphicsSkewImp");
    getProc(sciter.graphics.api.GraphicsTransform, "GraphicsTransformImp");
    getProc(sciter.graphics.api.GraphicsStateSave, "GraphicsStateSaveImp");
    getProc(sciter.graphics.api.GraphicsStateRestore, "GraphicsStateRestoreImp");
    getProc(sciter.graphics.api.GraphicsLineWidth, "GraphicsLineWidthImp");
    getProc(sciter.graphics.api.GraphicsLineJoin, "GraphicsLineJoinImp");
    getProc(sciter.graphics.api.GraphicsLineCap, "GraphicsLineCapImp");
    getProc(sciter.graphics.api.GraphicsLineColor, "GraphicsLineColorImp");
    getProc(sciter.graphics.api.GraphicsFillColor, "GraphicsFillColorImp");
    getProc(sciter.graphics.api.GraphicsLineGradientLinear, "GraphicsLineGradientLinearImp");
    getProc(sciter.graphics.api.GraphicsFillGradientLinear, "GraphicsFillGradientLinearImp");
    getProc(sciter.graphics.api.GraphicsLineGradientRadial, "GraphicsLineGradientRadialImp");
    getProc(sciter.graphics.api.GraphicsFillGradientRadial, "GraphicsFillGradientRadialImp");
    getProc(sciter.graphics.api.GraphicsFillMode, "GraphicsFillModeImp");
    getProc(sciter.graphics.api.TextCreateForElement, "TextCreateForElementImp");
    getProc(sciter.graphics.api.TextCreateForElementAndStyle, "TextCreateForElementAndStyleImp"); 
    getProc(sciter.graphics.api.TextAddRef, "TextAddRefImp");
    getProc(sciter.graphics.api.TextRelease, "TextReleaseImp");
    getProc(sciter.graphics.api.TextGetMetrics, "TextGetMetricsImp");
    getProc(sciter.graphics.api.TextSetBox, "TextSetBoxImp");
    getProc(sciter.graphics.api.GraphicsDrawText, "GraphicsDrawTextImp");
    getProc(sciter.graphics.api.GraphicsDrawImage, "GraphicsDrawImageImp");
    getProc(sciter.graphics.api.GraphicsWorldToScreen, "GraphicsWorldToScreenImp");
    getProc(sciter.graphics.api.GraphicsScreenToWorld, "GraphicsScreenToWorldImp");
    getProc(sciter.graphics.api.GraphicsPushClipBox, "GraphicsPushClipBoxImp");
    getProc(sciter.graphics.api.GraphicsPushClipPath, "GraphicsPushClipPathImp");
    getProc(sciter.graphics.api.GraphicsPopClip, "GraphicsPopClipImp");
    getProc(sciter.graphics.api.ImagePaint, "ImagePaintImp"); // paint on image using graphics
    getProc(sciter.graphics.api.vWrapGfx, "vWrapGfxImp");
    getProc(sciter.graphics.api.vWrapImage, "vWrapImageImp");
    getProc(sciter.graphics.api.vWrapPath, "vWrapPathImp");
    getProc(sciter.graphics.api.vWrapText, "vWrapTextImp");
    getProc(sciter.graphics.api.vUnWrapGfx, "vUnWrapGfxImp");
    getProc(sciter.graphics.api.vUnWrapImage, "vUnWrapImageImp");
    getProc(sciter.graphics.api.vUnWrapPath, "vUnWrapPathImp");
    getProc(sciter.graphics.api.vUnWrapText, "vUnWrapTextImp");
    getProc(sciter.graphics.api.GraphicsFlush, "GraphicsFlushImp");
    getProc(sciter.graphics.api.ImageCreateFromElement, "ImageCreateFromElementImp");
    //getProc(sciter.graphics.api.GraphicsGetNativeDC, "GraphicsGetNativeDCImp"); // 
    getProc(sciter.graphics.api.ImageCreate, "ImageCreateImp");
    getProc(sciter.graphics.api.ImageCreateFromPixmap, "ImageCreateFromPixmapImp");
    getProc(sciter.graphics.api.ImageAddRef, "ImageAddRefImp");
    getProc(sciter.graphics.api.ImageRelease, "ImageReleaseImp");
    getProc(sciter.graphics.api.ImageGetInfo, "ImageGetInfoImp");
    getProc(sciter.graphics.api.ImageClear, "ImageClearImp");
    getProc(sciter.graphics.api.ImageLoad, "ImageLoadImp");
    getProc(sciter.graphics.api.ImageSave, "ImageSaveImp");


    
  }
  catch (Exception e)
  {
    writeln("Error: ", e.msg);
    return false;
  }
  return true;
}

void unloadApi() {
  // Unload the library
  Runtime.unloadLibrary(libHandle);
}
