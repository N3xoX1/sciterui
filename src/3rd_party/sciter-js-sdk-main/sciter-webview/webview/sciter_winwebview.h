#ifndef _sciter_winwebview_h_
#define _sciter_winwebview_h_

#include <string>
#include <functional>

#include "webview_callbacks.h"

namespace webview
{

    class sciter_winwebview
    {
    public:
        sciter_winwebview(bool debug = false, void *wnd = nullptr);
        ~sciter_winwebview();

        void load_engine(const completion_fn_t& completion);

        void navigate(const std::string &url);
        void reload();
        void go_back();
        void go_forward();
        void stop();

        void advance_focus(bool forward);

        void *window();
        void set_size(int width, int height, int hints);
        void set_title(const std::string &url);

        void init(const std::string &js);
        void eval(const std::string &js);
        void set_html(const std::string& html);
        void dispatch(std::function<void()> f);

        void set_navigation_callback(const navigation_callback_t &cb);
        void set_msg_callback(const msg_callback_t &cb);
        void set_allowWindowOpen(const std::string& val);
        void set_allowContextMenu(const std::string& val);

        std::string currentSrc();

    protected:
        void* m_winbrowser = nullptr;
        bool m_isEdge = true;;
    };

}

#endif _sciter_winwebview_h_