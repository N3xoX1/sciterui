
#ifdef SCITER_LITE_OPENGL

#include "GLFW/glfw3.h"

struct context {
  GLFWwindow* window;

  context(GLFWwindow* win): window(win) {
  }

  void resize(UINT w, UINT h) {
  }

  void draw_texture(const RECT& update_rc) {
  }

  void swap_buffers() { 
    glfwSwapBuffers(window);
  }
  void draw_scene() {}

};

#endif
