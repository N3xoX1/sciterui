
// uncomment one of these:

//  D11 texture target
//#define SCITER_LITE_D11

//  D9 texture target
//#define SCITER_LITE_D9 -- NOTE: DX9 backend is not available at the moment

//  bitmap target (slow rasterization by CPU)
//#define SCITER_LITE_BITMAP

//  OpenGL current context target
#define SCITER_LITE_OPENGL

// SCITER {
#include "sciter-x.h"
#include "sciter-x-behavior.h"
#include "sciter-x-dom.hpp"
// } SCITER

#include "GLFW/glfw3.h"

#include "glfw-bitmap.h"
#include "glfw-dx11.h"
#include "glfw-dx9.h"
#include "glfw-opengl.h"

#include "aux-fs.h"

#include "facade-resources.cpp"

#if defined(SCITER_LITE_OPENGL)
  static glFunctionPointer GLGetProcAddress(const char* procName) {
    return glfwGetProcAddress(procName);
  }
#endif


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void char_callback(GLFWwindow* window, unsigned int codepoint);
static void focus_callback(GLFWwindow* window, int got_focus);
static void mouse_button_callback(GLFWwindow * window, int button, int acction, int modifiers);
static void mouse_move_callback(GLFWwindow * window, double x, double y);
static void mouse_enter_leave_callback(GLFWwindow *, int enter);
static void mouse_wheel_callback(GLFWwindow * window, double dx, double dy);

static UINT SC_CALLBACK handle_notification(LPSCITER_CALLBACK_NOTIFICATION pnm, LPVOID callbackParam);

static bool sciter_needs_drawing = true;
static RECT to_paint = { 0,0,0,0 };

SBOOL SC_CALLBACK DomEventProc(LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms) {
  if (evtg != HANDLE_SCRIPTING_METHOD_CALL)
    return FALSE;

  SCRIPTING_METHOD_PARAMS* p = (SCRIPTING_METHOD_PARAMS*)prms;

  // handle: r = Window.this.xcall("add",1,2);

  if (strcmp(p->name, "add") != 0) return FALSE;
  if (p->argc != 2) return FALSE;

  int a, b;
  // fetch args
  ValueIntData(&p->argv[0], &a);
  ValueIntData(&p->argv[1], &b);
  // set result
  ValueIntDataSet(&p->result, a + b, T_INT, 0);

  return TRUE;
}

int main(int argc, char *argv[])
{

    // these two calls are optional, used here to enable communication with inspector.exe (CTRL+SHIFT+I with running inspector)
    SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES,
      ALLOW_FILE_IO |
      ALLOW_SOCKET_IO |
      ALLOW_EVAL |
      ALLOW_SYSINFO);

    if (!glfwInit())
        exit(EXIT_FAILURE);

#if defined(SCITER_LITE_OPENGL)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
#else 
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

    //sciter::debug_output_console yes;

    SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);

    glfwSetErrorCallback(error_callback);

    GLFWwindow* window = glfwCreateWindow(700, 900, "Sciter.Lite example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

#if defined(SCITER_LITE_OPENGL)
    glfwMakeContextCurrent(window);
#endif

    SciterWindowAttachEventHandler(window, DomEventProc, NULL, HANDLE_SCRIPTING_METHOD_CALL);

    // setup event callbacks:
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_move_callback);
    glfwSetCursorEnterCallback(window, mouse_enter_leave_callback);
    glfwSetScrollCallback(window, mouse_wheel_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetWindowFocusCallback(window, focus_callback);

    context ctx(window);

    int used_width = 0, used_height = 0;

    auto resize_buffer = [&]() {
      ctx.resize(used_width, used_height);
      SL_SURFACE surface = {};
#if defined(SCITER_LITE_OPENGL)
      ; // no dedicated surface handle - drawing on default GL context
#elif defined(SCITER_LITE_D11)
      surface.texture = ctx.texture();
#elif defined(SCITER_LITE_D9)
      surface.texture = ctx.texture();
#elif defined(SCITER_LITE_BITMAP)
      surface.bitmap.pixels = ctx.dib.bits;
      surface.bitmap.stride = ctx.dib.stride;
#endif
      SciterProcX(window, SCITER_X_MSG_SIZE(used_width, used_height, surface));
      to_paint = { 0,0,used_width, used_height };
    };

    // SCITER+
      // create the engine and associate it with window:

#if defined(SCITER_LITE_OPENGL)
      // NOTE: In case of OpenGL we are passing "OpenGL interface" pointer so Sciter will use the same GL implementation 
    SciterProcX(window, SCITER_X_MSG_CREATE(SL_TARGET_OPENGL, (LPVOID)GLGetProcAddress));
#elif defined(SCITER_LITE_D11)
      SciterProcX(window, SCITER_X_MSG_CREATE(SL_TARGET_DX11_TEXTURE,ctx.d11_device.Get() ));
#elif defined(SCITER_LITE_D9)
      SciterProcX(window, SCITER_X_MSG_CREATE(SL_TARGET_DX9_TEXTURE, ctx.device()));
#elif defined(SCITER_LITE_BITMAP)
      SciterProcX(window, SCITER_X_MSG_CREATE(SL_TARGET_BITMAP, NULL));
#endif

      glfwGetFramebufferSize(window, &used_width, &used_height);
      resize_buffer();

      // set device resoultion ( pixels per inch )
      float xscale, yscale;
      glfwGetWindowContentScale(window, &xscale, &yscale);
      SciterProcX(window, SCITER_X_MSG_RESOLUTION(UINT(96 * xscale)));
      // set surface size


      SciterSetCallback(window, handle_notification, NULL);

      // load file:

      sciter::archive::instance().open(aux::elements_of(resources)); // bind resources[] (defined in "facade-resources.cpp") with the archive
      SciterLoadFile(window, WSTR("this://app/main.htm"));
      //const char* html = "<html style='background:rgba(255,0,0,0.5);font-size:20pt;font-weight:bold'><body>Hello world!</body></html>";
      //SciterLoadHtml(window, (LPCBYTE)html, strlen(html),WSTR("about:blank"));

    // SCITER-

    // NOTE: error checks have been omitted for brevity

    while (!glfwWindowShouldClose(window))
    {
      UINT ticks = UINT(glfwGetTime() * 1000); // in milliseconds
      // give sciter a chance to process animations, timers and other timed things
      SciterProcX(window, SCITER_X_MSG_HEARTBIT(ticks));

      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      if (width != used_width || height != used_height) {
        used_width = width;
        used_height = height;
        resize_buffer();
      }

      // do native drawing
      ctx.draw_scene();

      RECT update_rc = to_paint;

      //if (sciter_needs_drawing) //- suppressed this for a while to see FPS counter, enable this to minimize CPU consumption
      {
        sciter_needs_drawing = false;
        // SCITER - render Sciter on the texture
        SciterProcX(window, SCITER_X_MSG_PAINT(to_paint));
        to_paint = { 0,0,0,0 };
      }

      ctx.draw_texture(update_rc);
      ctx.swap_buffers();


      //glfwWaitEventsTimeout(0.016); // 60 FPS max - enable this to minimize CPU consumption
      glfwPollEvents();
    }

    SciterProcX(window, SCITER_X_MSG_DESTROY());

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

// SCITER+

static MOUSE_BUTTONS mbutton = MOUSE_BUTTONS(0);

static KEYBOARD_STATES key_states(GLFWwindow* window) {
  UINT ks = 0;
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ks |= KEYBOARD_STATE_LSHIFT;
  if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) ks |= KEYBOARD_STATE_RSHIFT;
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ks |= KEYBOARD_STATE_LCONTROL;
  if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) ks |= KEYBOARD_STATE_RCONTROL;
  if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) ks |= KEYBOARD_STATE_LALT;
  if (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) ks |= KEYBOARD_STATE_RALT;
  if (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) ks |= KEYBOARD_STATE_LCOMMAND;
  if (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS) ks |= KEYBOARD_STATE_RCOMMAND;

  if (glfwGetKey(window, GLFW_KEY_NUM_LOCK) == GLFW_PRESS) ks |= KEYBOARD_STATE_NUM;
  if (glfwGetKey(window, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS) ks |= KEYBOARD_STATE_CAPS;
  return KEYBOARD_STATES(ks);
}


static void mouse_button_callback(GLFWwindow * window, int button, int action, int modifiers)
{
  MOUSE_EVENTS  me = action == GLFW_PRESS ? MOUSE_DOWN : MOUSE_UP;
  switch (button) {
    default:
    case GLFW_MOUSE_BUTTON_1: mbutton = MAIN_MOUSE_BUTTON; break;
    case GLFW_MOUSE_BUTTON_2: mbutton = PROP_MOUSE_BUTTON; break;
    case GLFW_MOUSE_BUTTON_3: mbutton = MIDDLE_MOUSE_BUTTON; break;
  }

  double x, y; glfwGetCursorPos(window, &x, &y);
  POINT pos = {int(x),int(y)};

  SciterProcX(window, SCITER_X_MSG_MOUSE(me, mbutton, key_states(window), pos));
  if (me == MOUSE_UP)
    mbutton = MOUSE_BUTTONS(0);
}

static void mouse_move_callback(GLFWwindow * window, double x, double y)
{
  MOUSE_EVENTS  me = MOUSE_MOVE;
  POINT pos = { int(x),int(y) };
  SciterProcX(window, SCITER_X_MSG_MOUSE(me, mbutton, KEYBOARD_STATES(0), pos));
}

static void mouse_enter_leave_callback(GLFWwindow * window, int enter) {
  MOUSE_EVENTS  me = enter ? MOUSE_ENTER : MOUSE_LEAVE;
  UINT ks = 0;
  double x, y; glfwGetCursorPos(window, &x, &y);
  POINT pos = { int(x),int(y) };
  SciterProcX(window, SCITER_X_MSG_MOUSE(me, MOUSE_BUTTONS(0), KEYBOARD_STATES(ks), pos));
}

static void mouse_wheel_callback(GLFWwindow * window, double dx, double dy)
{
  MOUSE_EVENTS  me = MOUSE_WHEEL;
  double x, y; glfwGetCursorPos(window, &x, &y);
  POINT pos = { int(x),int(y) };
  dx *= 120;
  dy *= 120;
  UINT  deltas = ((uint16_t)(int16_t)dx) << 16 | (uint16_t)(int16_t)dy;
  SciterProcX(window, SCITER_X_MSG_MOUSE(me, MOUSE_BUTTONS(deltas), KEYBOARD_STATES(0), pos));
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  KEY_EVENTS me = GLFW_RELEASE == action ? KEY_UP : KEY_DOWN;

  SciterProcX(window, SCITER_X_MSG_KEY(me, UINT(key), key_states(window)));
}

static void char_callback(GLFWwindow* window, unsigned int codepoint)
{
  KEY_EVENTS me = KEY_CHAR;
  SciterProcX(window, SCITER_X_MSG_KEY(me, codepoint, KEYBOARD_STATES(0)));
}

static void focus_callback(GLFWwindow* window, int got_focus)
{
  SciterProcX(window, SCITER_X_MSG_FOCUS(!!got_focus));
}

UINT on_load_data(LPSCN_LOAD_DATA pnmld) {

  // your custom loader is here

  LPCBYTE pb = 0; UINT cb = 0;
  aux::wchars wu = aux::chars_of(pnmld->uri);

  if (wu.like(WSTR("this://app/*"))) {
    // try to get them from archive first
    aux::bytes adata = sciter::archive::instance().get(wu.start + 11);
    if (adata.length)
      SciterDataReady(pnmld->hwnd, pnmld->uri, adata.start, adata.length);
  }
  return LOAD_OK;
}

UINT on_data_loaded(LPSCN_DATA_LOADED pnm) { return 0; }

UINT attach_behavior(LPSCN_ATTACH_BEHAVIOR lpab) {
  // attach native behaviors (if we have any)
  sciter::event_handler *pb = sciter::behavior_factory::create(lpab->behaviorName, lpab->element);
  if (pb) {
    lpab->elementTag  = pb;
    lpab->elementProc = sciter::event_handler::element_proc;
    return 1;
  }
  return 0;
}

UINT on_invalidate_rect(LPSCN_INVALIDATE_RECT pnm) {
  sciter_needs_drawing = true;
#ifdef SCITER_LITE_BITMAP
  UnionRect(&to_paint, &to_paint, &pnm->invalidRect);
#endif
  glfwPostEmptyEvent();
  return 0;
}

UINT on_set_cursor(LPSCN_SET_CURSOR pnm) {

  /*typedef enum CURSOR_TYPE
  {
    CURSOR_ARROW, //0
    CURSOR_IBEAM, //1
    CURSOR_WAIT,  //2
    CURSOR_CROSS, //3
    CURSOR_UPARROW,  //4
    CURSOR_SIZENWSE, //5
    CURSOR_SIZENESW, //6
    CURSOR_SIZEWE,   //7
    CURSOR_SIZENS,   //8
    CURSOR_SIZEALL,  //9
    CURSOR_NO,       //10
    CURSOR_APPSTARTING, //11
    CURSOR_HELP,        //12
    CURSOR_HAND,        //13
    CURSOR_DRAG_MOVE,   //14
    CURSOR_DRAG_COPY,   //15
  } CURSOR_TYPE;*/

  static GLFWcursor* arrow_cursor   = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  static GLFWcursor* ibeam_cursor   = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
  static GLFWcursor* cross_cursor   = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
  static GLFWcursor* hand_cursor    = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
  static GLFWcursor* hresize_cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
  static GLFWcursor* vresize_cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);

  switch (pnm->cursorId) {
    case CURSOR_ARROW:  glfwSetCursor((GLFWwindow*)pnm->hwnd, arrow_cursor); break;
    case CURSOR_IBEAM:  glfwSetCursor((GLFWwindow*)pnm->hwnd, ibeam_cursor); break;
    case CURSOR_CROSS:  glfwSetCursor((GLFWwindow*)pnm->hwnd, cross_cursor); break;
    case CURSOR_HAND:   glfwSetCursor((GLFWwindow*)pnm->hwnd, hand_cursor); break;
    case CURSOR_SIZEWE: glfwSetCursor((GLFWwindow*)pnm->hwnd, hresize_cursor); break;
    case CURSOR_SIZENS: glfwSetCursor((GLFWwindow*)pnm->hwnd, vresize_cursor); break;
    default:
      glfwSetCursor((GLFWwindow*)pnm->hwnd, arrow_cursor); break;
  }
  return 0;
}


// notifiaction cracker
UINT SC_CALLBACK handle_notification(LPSCITER_CALLBACK_NOTIFICATION pnm, LPVOID callbackParam)
{
  // Crack and call appropriate method
  // here are all notifiactions
  switch (pnm->code) {
    case SC_LOAD_DATA: return on_load_data((LPSCN_LOAD_DATA)pnm);
    case SC_DATA_LOADED: return on_data_loaded((LPSCN_DATA_LOADED)pnm);
    case SC_ATTACH_BEHAVIOR: return attach_behavior((LPSCN_ATTACH_BEHAVIOR)pnm);
    case SC_INVALIDATE_RECT: return on_invalidate_rect((LPSCN_INVALIDATE_RECT)pnm);
    case SC_SET_CURSOR: return on_set_cursor((LPSCN_SET_CURSOR)pnm);
    case SC_ENGINE_DESTROYED: break;
  }
  return 0;
}



// SCITER-


