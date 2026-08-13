/*
 * MIT License
 *
 * Copyright (c) 2017 Serge Zaitsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef WEBVIEW_H
#define WEBVIEW_H

#ifndef WEBVIEW_API
#define WEBVIEW_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void *webview_t;

// Creates a new webview instance. If debug is non-zero - developer tools will
// be enabled (if the platform supports them). Window parameter can be a
// pointer to the native window handle. If it's non-null - then child WebView
// is embedded into the given parent window. Otherwise a new window is created.
// Depending on the platform, a GtkWindow, NSWindow or HWND pointer can be
// passed here.
WEBVIEW_API webview_t webview_create(int debug, void *window);

// Destroys a webview and closes the native window.
WEBVIEW_API void webview_destroy(webview_t w);

// Posts a function to be executed on the main thread. You normally do not need
// to call this function, unless you want to tweak the native window.
WEBVIEW_API void webview_dispatch(webview_t w, void (*fn)(webview_t w, void *arg), void *arg);

// Returns a native window handle pointer. When using GTK backend the pointer
// is GtkWindow pointer, when using Cocoa backend the pointer is NSWindow
// pointer, when using Win32 backend the pointer is HWND pointer.
WEBVIEW_API void *webview_get_window(webview_t w);

// Window size hints
#define WEBVIEW_HINT_NONE 0  // Width and height are default size
#define WEBVIEW_HINT_MIN 1   // Width and height are minimum bounds
#define WEBVIEW_HINT_MAX 2   // Width and height are maximum bounds
#define WEBVIEW_HINT_FIXED 3 // Window size can not be changed by a user
// Updates native window size. See WEBVIEW_HINT constants.
WEBVIEW_API void webview_set_size(webview_t w, int width, int height,
                                  int hints);

// Navigates webview to the given URL. URL may be a data URI, i.e.
// "data:text/html,<html>...</html>". It is often ok not to url-encode it
// properly, webview will re-encode it for you.
WEBVIEW_API void webview_navigate(webview_t w, const char *url);

// webview reload current page
WEBVIEW_API void webview_reload(webview_t w);

// Navigates webview to previous page
WEBVIEW_API void webview_go_back(webview_t w);

// Navigates webview to next page
WEBVIEW_API void webview_go_forward(webview_t w);

// webview to stop loading current page
WEBVIEW_API void webview_stop(webview_t w);

// Set webview HTML directly.
WEBVIEW_API void webview_set_html(webview_t w, const char *html);

// Injects JavaScript code at the initialization of the new page. Every time
// the webview will open a the new page - this initialization code will be
// executed. It is guaranteed that code is executed before window.onload.
WEBVIEW_API void webview_init(webview_t w, const char *js);

// Evaluates arbitrary JavaScript code. Evaluation happens asynchronously, also
// the result of the expression is ignored. Use RPC bindings if you want to
// receive notifications about the results of the evaluation.
WEBVIEW_API void webview_eval(webview_t w, const char *js);

// Binds a native C callback so that it will appear under the given name as a
// global JavaScript function. Internally it uses webview_init(). Callback
// receives a request string and a user-provided argument pointer. Request
// string is a JSON array of all the arguments passed to the JavaScript
// function.
WEBVIEW_API void webview_bind(webview_t w, const char *name,
                              void (*fn)(const char *seq, const char *req,
                                         void *arg),
                              void *arg);

// Removes a native C callback that was previously set by webview_bind.
WEBVIEW_API void webview_unbind(webview_t w, const char *name);

// Allows to return a value from the native binding. Original request pointer
// must be provided to help internal RPC engine match requests with responses.
// If status is zero - result is expected to be a valid JSON result value.
// If status is not zero - result is an error JSON object.
WEBVIEW_API void webview_return(webview_t w, const char *seq, int status,
                                const char *result);

#ifdef __cplusplus
}
#endif

#ifndef WEBVIEW_HEADER

#if !defined(WEBVIEW_GTK) && !defined(WEBVIEW_COCOA) && !defined(WEBVIEW_WIN)
#if defined(__APPLE__)
#define WEBVIEW_COCOA
#elif defined(__unix__)
#define WEBVIEW_GTK
#elif defined(_WIN32)
#define WEBVIEW_WIN
#else
#error "please, specify webview backend"
#endif
#endif

#include <atomic>
#include <functional>
#include <future>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cstring>

#include "webview_callbacks.h"

namespace webview {


// Convert ASCII hex digit to a nibble (four bits, 0 - 15).
//
// Use unsigned to avoid signed overflow UB.
static inline unsigned char hex2nibble(unsigned char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return 10 + (c - 'a');
  } else if (c >= 'A' && c <= 'F') {
    return 10 + (c - 'A');
  }
  return 0;
}

// Convert ASCII hex string (two characters) to byte.
//
// E.g., "0B" => 0x0B, "af" => 0xAF.
static inline char hex2char(const char *p) {
  return hex2nibble(p[0]) * 16 + hex2nibble(p[1]);
}

inline std::string url_encode(const std::string s) {
  std::string encoded;
  for (unsigned int i = 0; i < s.length(); i++) {
    auto c = s[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded = encoded + c;
    } else {
      char hex[4];
      snprintf(hex, sizeof(hex), "%%%02x", c);
      encoded = encoded + hex;
    }
  }
  return encoded;
}

inline std::string url_decode(const std::string st) {
  std::string decoded;
  const char *s = st.c_str();
  size_t length = strlen(s);
  for (unsigned int i = 0; i < length; i++) {
    if (s[i] == '%') {
      decoded.push_back(hex2char(s + i + 1));
      i = i + 2;
    } else if (s[i] == '+') {
      decoded.push_back(' ');
    } else {
      decoded.push_back(s[i]);
    }
  }
  return decoded;
}

inline std::string html_from_uri(const std::string s) {
  if (s.substr(0, 15) == "data:text/html,") {
    return url_decode(s.substr(15));
  }
  return "";
}

inline int json_parse_c(const char *s, size_t sz, const char *key, size_t keysz,
                        const char **value, size_t *valuesz) {
  enum {
    JSON_STATE_VALUE,
    JSON_STATE_LITERAL,
    JSON_STATE_STRING,
    JSON_STATE_ESCAPE,
    JSON_STATE_UTF8
  } state = JSON_STATE_VALUE;
  const char *k = NULL;
  int index = 1;
  int depth = 0;
  int utf8_bytes = 0;

  if (key == NULL) {
    index = (int)keysz;
    keysz = 0;
  }

  *value = NULL;
  *valuesz = 0;

  for (; sz > 0; s++, sz--) {
    enum {
      JSON_ACTION_NONE,
      JSON_ACTION_START,
      JSON_ACTION_END,
      JSON_ACTION_START_STRUCT,
      JSON_ACTION_END_STRUCT
    } action = JSON_ACTION_NONE;
    unsigned char c = *s;
    switch (state) {
    case JSON_STATE_VALUE:
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' ||
          c == ':') {
        continue;
      } else if (c == '"') {
        action = JSON_ACTION_START;
        state = JSON_STATE_STRING;
      } else if (c == '{' || c == '[') {
        action = JSON_ACTION_START_STRUCT;
      } else if (c == '}' || c == ']') {
        action = JSON_ACTION_END_STRUCT;
      } else if (c == 't' || c == 'f' || c == 'n' || c == '-' ||
                 (c >= '0' && c <= '9')) {
        action = JSON_ACTION_START;
        state = JSON_STATE_LITERAL;
      } else {
        return -1;
      }
      break;
    case JSON_STATE_LITERAL:
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' ||
          c == ']' || c == '}' || c == ':') {
        state = JSON_STATE_VALUE;
        s--;
        sz++;
        action = JSON_ACTION_END;
      } else if (c < 32 || c > 126) {
        return -1;
      } // fallthrough
    case JSON_STATE_STRING:
      if (c < 32 || (c > 126 && c < 192)) {
        return -1;
      } else if (c == '"') {
        action = JSON_ACTION_END;
        state = JSON_STATE_VALUE;
      } else if (c == '\\') {
        state = JSON_STATE_ESCAPE;
      } else if (c >= 192 && c < 224) {
        utf8_bytes = 1;
        state = JSON_STATE_UTF8;
      } else if (c >= 224 && c < 240) {
        utf8_bytes = 2;
        state = JSON_STATE_UTF8;
      } else if (c >= 240 && c < 247) {
        utf8_bytes = 3;
        state = JSON_STATE_UTF8;
      } else if (c >= 128 && c < 192) {
        return -1;
      }
      break;
    case JSON_STATE_ESCAPE:
      if (c == '"' || c == '\\' || c == '/' || c == 'b' || c == 'f' ||
          c == 'n' || c == 'r' || c == 't' || c == 'u') {
        state = JSON_STATE_STRING;
      } else {
        return -1;
      }
      break;
    case JSON_STATE_UTF8:
      if (c < 128 || c > 191) {
        return -1;
      }
      utf8_bytes--;
      if (utf8_bytes == 0) {
        state = JSON_STATE_STRING;
      }
      break;
    default:
      return -1;
    }

    if (action == JSON_ACTION_END_STRUCT) {
      depth--;
    }

    if (depth == 1) {
      if (action == JSON_ACTION_START || action == JSON_ACTION_START_STRUCT) {
        if (index == 0) {
          *value = s;
        } else if (keysz > 0 && index == 1) {
          k = s;
        } else {
          index--;
        }
      } else if (action == JSON_ACTION_END ||
                 action == JSON_ACTION_END_STRUCT) {
        if (*value != NULL && index == 0) {
          *valuesz = (size_t)(s + 1 - *value);
          return 0;
        } else if (keysz > 0 && k != NULL) {
          if (keysz == (size_t)(s - k - 1) && memcmp(key, k + 1, keysz) == 0) {
            index = 0;
          } else {
            index = 2;
          }
          k = NULL;
        }
      }
    }

    if (action == JSON_ACTION_START_STRUCT) {
      depth++;
    }
  }
  return -1;
}

inline std::string json_escape(std::string s) {
  // TODO: implement
  return '"' + s + '"';
}

inline int json_unescape(const char *s, size_t n, char *out) {
  int r = 0;
  if (*s++ != '"') {
    return -1;
  }
  while (n > 2) {
    char c = *s;
    if (c == '\\') {
      s++;
      n--;
      switch (*s) {
      case 'b':
        c = '\b';
        break;
      case 'f':
        c = '\f';
        break;
      case 'n':
        c = '\n';
        break;
      case 'r':
        c = '\r';
        break;
      case 't':
        c = '\t';
        break;
      case '\\':
        c = '\\';
        break;
      case '/':
        c = '/';
        break;
      case '\"':
        c = '\"';
        break;
      default: // TODO: support unicode decoding
        return -1;
      }
    }
    if (out != NULL) {
      *out++ = c;
    }
    s++;
    n--;
    r++;
  }
  if (*s != '"') {
    return -1;
  }
  if (out != NULL) {
    *out = '\0';
  }
  return r;
}

inline std::string json_parse(const std::string s, const std::string key,
                              const int index) {
  const char *value;
  size_t value_sz;
  if (key == "") {
    json_parse_c(s.c_str(), s.length(), nullptr, index, &value, &value_sz);
  } else {
    json_parse_c(s.c_str(), s.length(), key.c_str(), key.length(), &value,
                 &value_sz);
  }
  if (value != nullptr) {
    if (value[0] != '"') {
      return std::string(value, value_sz);
    }
    int n = json_unescape(value, value_sz, nullptr);
    if (n > 0) {
      char *decoded = new char[n + 1];
      json_unescape(value, value_sz, decoded);
      std::string result(decoded, n);
      delete[] decoded;
      return result;
    }
  }
  return "";
}

} // namespace webview

#ifdef WEBVIEW_WIN

  #include "sciter_winwebview.h"
  //#include "sciter_winwebview.cpp"

namespace webview {
    using browser_engine = sciter_winwebview;
}

#elif defined(WEBVIEW_COCOA)

  #include "sciter_wkwebview.h"
  //add "sciter_wkwebview.mm" to project source file

namespace webview {
    using browser_engine = sciter_wkwebview;
}

#elif defined(WEBVIEW_GTK) 

  #include "sciter_webkitgtk.h"
  //#include "sciter_webkitgtk.cpp"

namespace webview {
    using browser_engine = sciter_webkitgtk;
}

#endif

namespace webview {

class webview {
public:
  webview(bool debug = false, void *parent = nullptr) {
      engine_instance = new browser_engine(debug, parent);
      engine_instance->set_msg_callback([&](const std::string& msg) {
        on_message(msg);
      });
  }

  ~webview() {
      delete engine_instance;
  }

  void load_engine(const completion_fn_t& completion) {
      engine_instance->load_engine(completion);
  }

  void navigate(const std::string& url) {
      engine_instance->navigate(url);
  }

  void reload() {
      engine_instance->reload();
  }

  void go_back() {
      engine_instance->go_back();
  }

  void go_forward() {
      engine_instance->go_forward();
  }

  void stop() {
      engine_instance->stop();
  }

  void advance_focus(bool forward) {
#ifdef WINDOWS
      engine_instance->advance_focus(forward);
#endif
  }

  using binding_t = std::function<void(std::string, std::string, void *)>;
  using binding_ctx_t = std::pair<binding_t *, void *>;

  using sync_binding_t = std::function<std::string(std::string)>;
  using sync_binding_ctx_t = std::pair<webview *, sync_binding_t>;

  void bind(const std::string& name, sync_binding_t fn) {
    bind(
        name,
        [](std::string seq, std::string req, void *arg) {
          auto pair = static_cast<sync_binding_ctx_t *>(arg);
          pair->first->resolve(seq, 0, pair->second(req));
        },
        new sync_binding_ctx_t(this, fn));
  }

  void bind(const std::string& name, binding_t f, void *arg) {
    auto js = "(function() { var name = '" + name + "';" + R"(
      var RPC = window._rpc = (window._rpc || {nextSeq: 1});
      window[name] = function() {
        var seq = RPC.nextSeq++;
        var promise = null;
        if (undefined != window.Promise) {
            promise = new Promise(function(resolve, reject) {
              RPC[seq] = {
                resolve: resolve,
                reject: reject,
              };
            });
        }
        else {
            promise = {
              onResolved: undefined,
              onRejected: undefined,
              onFinally: undefined,
              then: function (onResolved) {
                this.onResolved = onResolved;
              },
              catch: function (onRejected) {
                this.onRejected = onRejected;
              },
              finally: function (onFinally) {
              }
            };
            RPC[seq] = {
                resolve: function(result) {
                    if (promise.onResolved) {
                        promise.onResolved(result);
                    }
                    if (promise.onFinally) {
                        promise.onFinally(result);
                    }
                },
                reject: function(reason) {
                    if (promise.catch) {
                        promise.catch(reason);
                    }
                    if (promise.onFinally) {
                        promise.onFinally(reason);
                    }
                }
            }
        }
        window.external.invoke(JSON.stringify({
          id: seq,
          method: name,
          params: Array.prototype.slice.call(arguments),
        }));
        return promise;
      }
    })())";
    init(js);
    bindings[name] = new binding_ctx_t(new binding_t(f), arg);
  }

  void unbind(const std::string& name) {
    if (bindings.find(name) != bindings.end()) {
      auto js = "delete window['" + name + "'];";
      init(js);
      eval(js);
      delete bindings[name]->first;
      delete static_cast<sync_binding_ctx_t *>(bindings[name]->second);
      delete bindings[name];
      bindings.erase(name);
    }
  }

  void resolve(const std::string seq, int status, const std::string result, bool dispatch_async = true) {
    if (dispatch_async) {
      dispatch([=]() {
        if (status == 0) {
          eval("window._rpc[" + seq + "].resolve(" + result +
              "); delete window._rpc[" + seq + "]");
        }
        else {
          eval("window._rpc[" + seq + "].reject(" + result +
              "); delete window._rpc[" + seq + "]");
        }
      });
    }
    else {
      if (status == 0) {
        eval("window._rpc[" + seq + "].resolve(" + result +
            "); delete window._rpc[" + seq + "]");
      }
      else {
        eval("window._rpc[" + seq + "].reject(" + result +
            "); delete window._rpc[" + seq + "]");
      }
    }
  }

  void* window() {
      return engine_instance->window();
  }
  void set_size(int width, int height, int hints) {
      engine_instance->set_size(width, height, hints);
  }

  void init(const std::string& js) {
      engine_instance->init(js);
  }
  void eval(const std::string& js) {
      engine_instance->eval(js);
  }
  void set_html(const std::string& html) {
      engine_instance->set_html(html);
  }
  void dispatch(dispatch_fn_t f) {
      engine_instance->dispatch(f);
  }
  
  void set_navigation_callback(const navigation_callback_t& cb) {
      engine_instance->set_navigation_callback(cb);
  }

  void set_allowWindowOpen(const std::string& val) {
      engine_instance->set_allowWindowOpen(val);
  }

  void set_allowContextMenu(const std::string& val) {
      engine_instance->set_allowContextMenu(val);
  }

  std::string currentSrc() {
      return engine_instance->currentSrc();
  }

private:
  void on_message(const std::string msg) {
    auto seq = json_parse(msg, "id", 0);
    auto name = json_parse(msg, "method", 0);
    auto args = json_parse(msg, "params", 0);
    if (bindings.find(name) == bindings.end()) {
      return;
    }
    auto fn = bindings[name];
    (*fn->first)(seq, args, fn->second);
  }
  std::map<std::string, binding_ctx_t *> bindings;
  browser_engine* engine_instance;
};
} // namespace webview

WEBVIEW_API webview_t webview_create(int debug, void *parent) {
  return new webview::webview(debug, parent);
}

WEBVIEW_API void webview_destroy(webview_t w) {
  delete static_cast<webview::webview *>(w);
}

WEBVIEW_API void webview_dispatch(webview_t w, void (*fn)(webview_t, void *),
                                  void *arg) {
  static_cast<webview::webview *>(w)->dispatch([=]() { fn(w, arg); });
}

WEBVIEW_API void *webview_get_window(webview_t w) {
  return static_cast<webview::webview *>(w)->window();
}

WEBVIEW_API void webview_set_size(webview_t w, int width, int height,
                                  int hints) {
  static_cast<webview::webview *>(w)->set_size(width, height, hints);
}

WEBVIEW_API void webview_navigate(webview_t w, const char *url) {
  static_cast<webview::webview *>(w)->navigate(url);
}

WEBVIEW_API void webview_reload(webview_t w) {
    static_cast<webview::webview*>(w)->reload();
}

WEBVIEW_API void webview_go_back(webview_t w) {
    static_cast<webview::webview*>(w)->go_back();
}

WEBVIEW_API void webview_go_forward(webview_t w) {
    static_cast<webview::webview*>(w)->go_forward();
}

WEBVIEW_API void webview_stop(webview_t w) {
    static_cast<webview::webview*>(w)->stop();
}

WEBVIEW_API void webview_set_html(webview_t w, const char *html) {
  static_cast<webview::webview *>(w)->set_html(html);
}

WEBVIEW_API void webview_init(webview_t w, const char *js) {
  static_cast<webview::webview *>(w)->init(js);
}

WEBVIEW_API void webview_eval(webview_t w, const char *js) {
  static_cast<webview::webview *>(w)->eval(js);
}

WEBVIEW_API void webview_bind(webview_t w, const char *name,
                              void (*fn)(const char *seq, const char *req,
                                         void *arg),
                              void *arg) {
  static_cast<webview::webview *>(w)->bind(
      name,
      [=](std::string seq, std::string req, void *arg) {
        fn(seq.c_str(), req.c_str(), arg);
      },
      arg);
}

WEBVIEW_API void webview_unbind(webview_t w, const char *name) {
  static_cast<webview::webview *>(w)->unbind(name);
}

WEBVIEW_API void webview_return(webview_t w, const char *seq, int status,
                                const char *result, bool dispatch_async = true) {
  static_cast<webview::webview *>(w)->resolve(seq, status, result, dispatch_async);
}

WEBVIEW_API void webview_set_navigation_callback(webview_t w, const webview::navigation_callback_t& cb) {
    static_cast<webview::webview*>(w)->set_navigation_callback(cb);
}

WEBVIEW_API std::string webview_currentSrc(webview_t w) {
    return static_cast<webview::webview*>(w)->currentSrc();
}

#endif /* WEBVIEW_HEADER */

#endif /* WEBVIEW_H */
