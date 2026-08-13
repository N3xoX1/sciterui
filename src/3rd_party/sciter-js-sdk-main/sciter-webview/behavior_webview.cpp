#include "sciter-x.h"
#include "sciter-x-behavior.h"
#include "webview/sciter_webview.h"

#include <algorithm>

namespace sciter
{
	/*
	BEHAVIOR: webview
	   - provides web pages
	COMMENTS:
	   <webview style="behavior:webview" />
	SAMPLE:
	   See: samples/webview/webview.htm
	*/

  #define INIT_WEBVIEW (FIRST_APPLICATION_EVENT_CODE + 1)

	struct sciter_webview : public event_handler
	{
		webview::webview *this_webview;
		HELEMENT this_element;

		// ctor
		sciter_webview() : this_webview(nullptr), this_element(0) {}
		virtual ~sciter_webview() {}

		virtual bool subscription(HELEMENT he, UINT &event_groups) override
		{
			event_groups = HANDLE_BEHAVIOR_EVENT | HANDLE_FOCUS | HANDLE_ATTRIBUTE_CHANGE | HANDLE_SIZE;
			return true;
		}

		void attached(HELEMENT he) override
		{
			this_element = he;

			dom::element self = dom::element(this_element);

			// NOTE: the attached() method 
			// a) should return as soon as possible - do not perform heavy and time consuming operations.
			// b) is not supposed to draw anything, but WebView forces parent window to be redrawn synchronously. 
			// so we delay WebView initialization until DOM build completion:
			self.post_event(INIT_WEBVIEW);
		}

		bool on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason) override { 
			if (type == INIT_WEBVIEW) {
				init_webview();
				return true;
			}
			return false; 
		}

		void detached(HELEMENT he) override
		{
			if (nullptr != this_webview)
			{
				delete this_webview;
				this_webview = nullptr;
			}
			asset_release();
		}

		virtual void on_size(HELEMENT he) override
		{
			if (nullptr == this_webview)
			{
				return;
			}
			dom::element self = dom::element(he);
			RECT rc = self.get_location(CONTENT_BOX);
			int width = rc.right - rc.left; if (!width) width = 10;
			int height = rc.bottom - rc.top; if (!height) height = 10;
			this_webview->set_size(width, height, WEBVIEW_HINT_FIXED);
		}

		virtual bool handle_focus(HELEMENT he, FOCUS_PARAMS& params) override
		{ 
			if (nullptr == this_webview)
			{
				return false;
			}
			switch (params.cmd) {
				case FOCUS_ADVANCE_REQUEST:
					this_webview->advance_focus(params.cause == FOCUS_RQ_NEXT);
					return true;
			}
			return false; 
		}

		virtual void on_attribute_change(HELEMENT he, LPCSTR name, LPCWSTR value) override
		{
			sciter::string str_val = (nullptr != value) ? value : WSTR("");
			if (0 == strcmp(name, "src")) {
				on_src_changed();
			}
			else if (0 == strcmp(name, "allowWindowOpen")) {
				on_allowWindowOpen_changed();
			}
			else if (0 == strcmp(name, "allowContextMenu")) {
				on_allowContextMenu_changed();
			}
		}

		void init_webview() {
			dom::element self = dom::element(this_element);

			HWINDOW parent = self.get_element_hwnd(false);
			this_webview = new webview::webview(false, parent);
			HWINDOW window = (HWINDOW)this_webview->window();
			self.attach_hwnd(window);

			this_webview->set_navigation_callback([=](webview::navigation_event evt, const std::string& param) -> int
				{
					switch (evt) {
						case webview::navigation_event::will_navigate:
							self.send_event(WSTR("webview-will-navigate"), sciter::value(param));
							break;
						case webview::navigation_event::did_navigate:
							self.send_event(WSTR("webview-did-navigate"), sciter::value(param));
							break;
						case webview::navigation_event::navigate_failure:
							self.send_event(WSTR("webview-navigate-failure"), sciter::value(param));
							break;
						case webview::navigation_event::title_did_change:
							self.send_event(WSTR("webview-title-change"), sciter::value(param));
							break;
					}
					//sciter::value strEvt = sciter::value::make_string(aux::utf2w(evt));
					//sciter::value strParam = sciter::value::make_string(aux::utf2w(param));
					//sciter::value ret = self.call_method("onNavigation", strEvt, strParam);
					//return ret.get(0);
					return 0;
				});

			this_webview->load_engine([=](bool succeed) -> void {

				if (succeed) {
					bindSciterJSCall();
					on_allowWindowOpen_changed();
					on_allowContextMenu_changed();
					on_src_changed();
					self.send_event(WSTR("webview-ready"));
				} else
					self.send_event(WSTR("webview-unavailable"));
				
			});
		}

		void on_src_changed() {
			dom::element self = dom::element(this_element);
			sciter::string src = self.get_attribute("src");
			if (0 == src.size()) {
				return;
			}
			sciter::astring utfSrc(aux::w2utf(src.c_str()));
			this_webview->navigate(utfSrc.c_str());
		}

		void on_allowWindowOpen_changed() {
			dom::element self = dom::element(this_element);
			sciter::string val = self.get_attribute("allowWindowOpen", WSTR("nopopup"));
			if (0 == val.length()) {
				this_webview->set_allowWindowOpen("nopopup");
			}
			else {
				aux::w2utf strVal(val.c_str());
				this_webview->set_allowWindowOpen(strVal.c_str());
			}
		}

		void on_allowContextMenu_changed() {
			dom::element self = dom::element(this_element);
			sciter::string val = self.get_attribute("allowContextMenu", WSTR("true"));
			if (0 == val.length()) {
				this_webview->set_allowContextMenu("false");
			}
			else {
				aux::w2utf strVal(val.c_str());
				this_webview->set_allowContextMenu(strVal.c_str());
			}
		}

		void bindSciterJSCall()
		{
			if (nullptr == this_webview)
			{
				return;
			}
			this_webview->bind(
				"jsBridgeCall", [](std::string seq, std::string req, void *arg) -> void
				{
				sciter_webview* elem_webview = (sciter_webview*)arg;
				if (nullptr == elem_webview) {
					return;
				}
				sciter::value jsonCall = sciter::value::from_string(aux::utf2w(req).as_chars(), CVT_JSON_LITERAL);
				dom::element self = dom::element(elem_webview->this_element);
				sciter::string ret = self.call_method("jsBridgeCall", jsonCall).to_string();
				sciter::astring utfRet(aux::w2utf(ret.c_str()));
				elem_webview->this_webview->resolve(seq, 0, utfRet.c_str()); },
				this);
		}

		void unbindSciterJSCall()
		{
			if (nullptr == this_webview)
			{
				return;
			}
			this_webview->unbind("jsBridgeCall");
		}

		void loadUrl(const sciter::value &url)
		{
			if (nullptr == this_webview)
			{
				return;
			}
			sciter::string strUrl = url.to_string();
			sciter::astring utfUrl(aux::w2utf(strUrl.c_str()));
			this_webview->navigate(utfUrl.c_str());
		}

		void loadHtml(const sciter::value &html)
		{
			if (nullptr == this_webview)
			{
				return;
			}
			sciter::string strHtml = html.to_string();
			sciter::astring utfHtml(aux::w2utf(strHtml.c_str()));
			this_webview->set_html(utfHtml.c_str());
		}

		void evaluateJavaScript(const sciter::value &jsStr)
		{
			if (nullptr == this_webview)
			{
				return;
			}
			sciter::string strJS = jsStr.to_string();
			sciter::astring utfJS(aux::w2utf(strJS.c_str()));
			this_webview->eval(utfJS.c_str());
		}

		void reload()
		{
			if (nullptr == this_webview)
			{
				return;
			}
			this_webview->reload();
		}

		void goBack()
		{
			if (nullptr == this_webview)
			{
				return;
			}
			this_webview->go_back();
		}

		void goForward()
		{
			if (nullptr == this_webview)
			{
				return;
			}
			this_webview->go_forward();
		}

		void stop()
		{
			if (nullptr == this_webview)
			{
				return;
			}
		}

		bool setSrc(const sciter::value& src) {
			if (src == getSrc()) {
				return false;
			}
            sciter::string strSrc = src.to_string();
			dom::element self = dom::element(this_element);
			self.set_attribute("src", strSrc.c_str());
			//on_src_changed();
			return true;
		}

		sciter::string getSrc() {
			dom::element self = dom::element(this_element);
			return self.get_attribute("src");
		}

		sciter::string getCurrentSrc() {
			if (nullptr == this_webview) {
				return sciter::string(WSTR(""));
			}
			std::string src = this_webview->currentSrc();
			aux::utf2w wSrc(src.c_str());
			return sciter::string(wSrc.c_str());
		}

		sciter::string getAllowWindowOpen() {
			dom::element self = dom::element(this_element);
			return self.get_attribute("allowWindowOpen");
		}

		bool setAllowWindowOpen(const sciter::value& val) {
			if (val == getAllowWindowOpen()) {
				return false;
			}
            sciter::string strVal = val.to_string();
			dom::element self = dom::element(this_element);
			self.set_attribute("allowWindowOpen", strVal.c_str());
			on_allowWindowOpen_changed();
			return true;
		}

		sciter::string getAllowContextMenu() {
			dom::element self = dom::element(this_element);
			return self.get_attribute("allowContextMenu", WSTR("true"));
		}

		bool setAllowContextMenu(const sciter::value& val) {
			if (val == getAllowContextMenu()) {
				return false;
			}
            sciter::string strVal = val.to_string();
			dom::element self = dom::element(this_element);
			self.set_attribute("allowContextMenu", strVal.c_str());
			on_allowContextMenu_changed();
			return true;
		}

		SOM_PASSPORT_BEGIN_EX(webview, sciter_webview)
			SOM_FUNCS(
				SOM_FUNC(loadUrl),
				SOM_FUNC(loadHtml),
				SOM_FUNC(reload),
				SOM_FUNC(goBack),
				SOM_FUNC(goForward),
				SOM_FUNC(stop),
				SOM_FUNC(evaluateJavaScript)
			)
			SOM_PROPS(
				SOM_VIRTUAL_PROP(src, getSrc, setSrc),
				SOM_VIRTUAL_PROP(allowWindowOpen, getAllowWindowOpen, setAllowWindowOpen),
				SOM_VIRTUAL_PROP(allowContextMenu, getAllowContextMenu, setAllowContextMenu),
				SOM_RO_VIRTUAL_PROP(currentSrc, getCurrentSrc)
			)
		SOM_PASSPORT_END
	};

}

#ifdef SCITERWEBVIEW_EXPORTS

/** The SciterBehaviorFactory of this module */
SBOOL SCAPI ThisBehaviorFactory(LPCSTR name, HELEMENT he, LPElementEventProc *elProc, LPVOID *elTag)
{

	if (aux::chars_of(name) != const_chars("webview"))
	{
		return FALSE;
	}
	sciter::sciter_webview *webview = new sciter::sciter_webview();
	webview->asset_add_ref();
	*elProc = webview->element_proc;
	*elTag = webview;
	return TRUE;
}

extern "C"
{
	/** SciterBehaviorFactoryInit is the only exported function that Sciter runtime will call to initialize Sciter component extension library.
	 * \param psapi[in] \b ISciterAPI*, instance of Sciter API interface provided by Sciter runtime to the library.
	 * \param pfactory[out] \b SciterBehaviorFactory**, instance of behavior factory that will create behaviors of this library.
	 * \return TRUE on success.
	 **/
#ifndef WINDOWS
	__attribute__((visibility("default")))
#endif
	SBOOL SCAPI SciterBehaviorFactoryInit(ISciterAPI *psapi, SciterBehaviorFactory **pfactory)
	{
		_SAPI(psapi); // set reference to Sciter API provided by host application including scapp(quark)
		*pfactory = ThisBehaviorFactory;
		return TRUE;
	}
}

#else

namespace sciter
{
    struct webview_factory : public behavior_factory
    {

        webview_factory() : behavior_factory("webview") {}

        // the only behavior_factory method:
        virtual event_handler *create(HELEMENT he)
        {
            return new sciter_webview();
        }
    };

    // instantiating and attaching it to the global list
    webview_factory webview_factory_instance;
}

#endif
