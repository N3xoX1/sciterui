#include "sciter_wkwebview.h"

#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

@interface SciterWKWebViewDelegate : NSObject <NSWindowDelegate, WKUIDelegate, WKNavigationDelegate, WKScriptMessageHandler>

@property (nonatomic, assign) webview::sciter_wkwebview* webEngine;

@end

@implementation SciterWKWebViewDelegate

- (void)windowWillClose:(NSNotification *)notification {
    delete self.webEngine;
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    [sender orderOut:nil];
    return true;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context {
    if ([keyPath isEqualToString:@"title"]) {
        NSString* title = [change objectForKey:NSKeyValueChangeNewKey];
        WKWebView* webview = (WKWebView *)object;
        if (nullptr != self.webEngine->m_navigationCallback) {
            self.webEngine->m_navigationCallback(webview::navigation_event::title_did_change, title.UTF8String);
        }
        else {
            webview.window.title = title;
        }
    }
}

- (nullable WKWebView *)webView:(WKWebView *)webView createWebViewWithConfiguration:(nonnull WKWebViewConfiguration *)configuration forNavigationAction:(nonnull WKNavigationAction *)navigationAction windowFeatures:(nonnull WKWindowFeatures *)windowFeatures {
    const char* allow = self.webEngine->m_allowWindowOpen.c_str();
    if (0 == strcmp("false", allow)) {
        return nil;
    }
    else if (0 == strcmp("popup", allow)) {
        NSRect frame = CGRectMake(0, 0, 640, 480);
        if (nil != windowFeatures.x) {
            frame.origin.x = windowFeatures.x.floatValue;
        }
        if (nil != windowFeatures.y) {
            frame.origin.y = windowFeatures.y.floatValue;
        }
        if (nil != windowFeatures.width) {
            frame.size.width = windowFeatures.width.floatValue;
        }
        if (nil != windowFeatures.height) {
            frame.size.height = windowFeatures.height.floatValue;
        }
        
        NSUInteger style =  NSTitledWindowMask | NSClosableWindowMask |NSMiniaturizableWindowMask;
        if (windowFeatures.allowsResizing) {
            style |= NSResizableWindowMask;
        }
        
        NSWindow *window = [[NSWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:YES];
        window.releasedWhenClosed = false;
        
        std::string url = navigationAction.request.URL.absoluteString.UTF8String;
        webview::sciter_wkwebview* webview = new webview::sciter_wkwebview(self.webEngine->m_debugtools, (__bridge void*)window.contentView);
        NSView* view = (__bridge NSView *)webview->window();
        [window.contentView addSubview:view];
        webview->load_engine([=](bool succeed) -> void {
            if (succeed) {
                window.delegate = (__bridge SciterWKWebViewDelegate*)webview->m_webviewDelegate;
                webview->navigate(url);
            }
        });
        webview->set_allowWindowOpen(self.webEngine->m_allowWindowOpen);
        [window center];
        [window orderFront:nil];
        
        return nil;
    }
    else {
        [webView loadRequest:navigationAction.request];
        return nil;
    }
}

- (void)webViewDidClose:(WKWebView *)webView {
    if (nil == self.webEngine->m_navigationCallback) {
        [webView.window close];
    }
}

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(null_unspecified WKNavigation *)navigation {
    if (nullptr == self.webEngine->m_navigationCallback) {
        return;
    }
    self.webEngine->m_navigationCallback(webview::navigation_event::will_navigate, webView.URL.absoluteString.UTF8String);
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(null_unspecified WKNavigation *)navigation withError:(nonnull NSError *)error {
    if (nullptr == self.webEngine->m_navigationCallback) {
        return;
    }
    self.webEngine->m_navigationCallback(webview::navigation_event::navigate_failure, "");
}

- (void)webView:(WKWebView *)webView didCommitNavigation:(null_unspecified WKNavigation *)navigation {
    //Do nothing
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(null_unspecified WKNavigation *)navigation {
    if (nullptr == self.webEngine->m_navigationCallback) {
        return;
    }
    self.webEngine->m_navigationCallback(webview::navigation_event::did_navigate, "");
}

- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation withError:(NSError *)error {
    if (nullptr == self.webEngine->m_navigationCallback) {
        return;
    }
    self.webEngine->m_navigationCallback(webview::navigation_event::navigate_failure, "");
}

- (void)webView:(WKWebView *)webView runJavaScriptAlertPanelWithMessage:(nonnull NSString *)message initiatedByFrame:(nonnull WKFrameInfo *)frame completionHandler:(nonnull void (^)())completionHandler {
    NSAlert* alert = [[NSAlert alloc] init];
    alert.messageText = message;
    [alert runModal];
    completionHandler();
}

- (void)webView:(WKWebView *)webView runJavaScriptConfirmPanelWithMessage:(NSString *)message initiatedByFrame:(WKFrameInfo *)frame completionHandler:(void (^)(BOOL))completionHandler {
    NSAlert* alert = [[NSAlert alloc] init];
    alert.messageText = message;
    [alert addButtonWithTitle:@"Confirm"];
    [alert addButtonWithTitle:@"Cancel"];
    NSModalResponse resp = [alert runModal];
    completionHandler(resp == NSAlertFirstButtonReturn);
}

- (void)webView:(WKWebView *)webView runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt defaultText:(NSString *)defaultText initiatedByFrame:(WKFrameInfo *)frame completionHandler:(void (^)(NSString * _Nullable))completionHandler {
    NSTextField* field = [[NSTextField alloc] initWithFrame:CGRectMake(0, 0, 200, 24)];
    NSAlert* alert = [[NSAlert alloc] init];
    alert.informativeText = prompt;
    alert.messageText = defaultText;
    alert.accessoryView = field;
    [alert addButtonWithTitle:@"Confirm"];
    [alert addButtonWithTitle:@"Cancel"];
    NSModalResponse resp = [alert runModal];
    completionHandler(resp == NSAlertFirstButtonReturn ? field.stringValue : nullptr);
}

- (void)webView:(WKWebView *)webView runOpenPanelWithParameters:(WKOpenPanelParameters *)parameters initiatedByFrame:(WKFrameInfo *)frame completionHandler:(void (^)(NSArray<NSURL *> * _Nullable))completionHandler {
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    panel.allowsMultipleSelection = parameters.allowsMultipleSelection;
    panel.canChooseDirectories = parameters.allowsDirectories;
    NSModalResponse result = [panel runModal];
    if (NSModalResponseOK == result) {
        completionHandler(panel.URLs);
    }
    else {
        completionHandler(nil);
    }
}

- (void)userContentController:(WKUserContentController *)userContentController didReceiveScriptMessage:(WKScriptMessage *)message {
    if (nullptr == self.webEngine->m_msgCallback) {
        return;
    }
    NSString* body = message.body;
    self.webEngine->m_msgCallback(body.UTF8String);
}

@end

namespace webview
{

sciter_wkwebview::sciter_wkwebview(bool debug /*= false*/, void *parent /*= nullptr*/) {
    m_debugtools = debug;
    NSWindow* nsParentWindow = (__bridge NSWindow*)parent;
    NSView* nsParentView = [nsParentWindow contentView];
    NSView* containerView = [[NSView alloc] initWithFrame:nsParentView.bounds];
    [nsParentView addSubview: containerView];
    m_containerView = (__bridge void*)containerView;
}

sciter_wkwebview::~sciter_wkwebview() {
    SciterWKWebViewDelegate* webviewDelegate = (__bridge_transfer SciterWKWebViewDelegate*)m_webviewDelegate;
    WKWebView* webview = (__bridge WKWebView*)m_webview;
    [webview removeObserver:webviewDelegate forKeyPath:@"title"];
    webview.navigationDelegate = nil;
    webview.UIDelegate = nil;
    webviewDelegate = nil;
    m_controller = nullptr;
    m_webview = nullptr;
    m_containerView = nullptr;
}

void sciter_wkwebview::load_engine(const completion_fn_t &completion) {
    NSView* nsParentView = (__bridge NSView*)m_containerView;
    WKWebViewConfiguration* config = [[WKWebViewConfiguration alloc] init];
    WKUserContentController* controller = config.userContentController;
    [config.preferences setValue:[NSNumber numberWithBool:m_debugtools] forKey:@"developerExtrasEnabled"];
    [config.preferences setValue:[NSNumber numberWithBool:true] forKey:@"fullScreenEnabled"];
    [config.preferences setValue:[NSNumber numberWithBool:true] forKey:@"javaScriptCanAccessClipboard"];
    [config.preferences setValue:[NSNumber numberWithBool:true] forKey:@"DOMPasteAllowed"];
    config.preferences.javaScriptEnabled = YES;
    config.preferences.javaScriptCanOpenWindowsAutomatically = YES;
    
    SciterWKWebViewDelegate* wkDelegate = [[SciterWKWebViewDelegate alloc] init];
    wkDelegate.webEngine = this;
    
    [controller addScriptMessageHandler:wkDelegate name:@"external"];
    
    WKWebView* webview = [[WKWebView alloc] initWithFrame:nsParentView.bounds configuration:config];
    webview.navigationDelegate = wkDelegate;
    webview.UIDelegate = wkDelegate;
    webview.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [webview addObserver:wkDelegate forKeyPath:@"title" options:NSKeyValueObservingOptionNew context:nil];
    [nsParentView addSubview:webview];
    
    m_controller = (__bridge void*)controller;
    m_webview = (__bridge void*)webview;
    m_webviewDelegate = (__bridge_retained void*)wkDelegate;
    
    init("{window.external={invoke:(s)=>{window.webkit.messageHandlers.external.postMessage(s)}}}");
    
    completion_fn_t* handler = new completion_fn_t(completion);
    dispatch_async(dispatch_get_main_queue(), ^{
        (*handler)(true);
        delete handler;
    });
}

void sciter_wkwebview::navigate(const std::string &url) {
    WKWebView* webview = (__bridge WKWebView*)m_webview;
    NSString* strUrl = [NSString stringWithUTF8String:url.c_str()];
    [webview loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:strUrl]]];
}

void sciter_wkwebview::reload() {
    WKWebView* webview = (__bridge WKWebView*)m_webview;
    [webview reload];
}

void sciter_wkwebview::go_back() {
    WKWebView* webview = (__bridge WKWebView*)m_webview;
    [webview goBack];
}

void sciter_wkwebview::go_forward() {
    WKWebView* webview = (__bridge WKWebView*)m_webview;
    [webview goForward];
}

void sciter_wkwebview::stop() {
    WKWebView* webview = (__bridge WKWebView*)m_webview;
    [webview stopLoading];
}

void* sciter_wkwebview::window() {
    return m_containerView;
}

void sciter_wkwebview::set_size(int width, int height, int hints) {
    //Do nothing with autoresizingMask has been set to auo;
}

void sciter_wkwebview::init(const std::string &js) {
    NSString* strJS = [NSString stringWithUTF8String:js.c_str()];
    WKUserScript* userScript = [[WKUserScript alloc] initWithSource:strJS injectionTime:WKUserScriptInjectionTimeAtDocumentStart forMainFrameOnly:YES];
    WKUserContentController* userCtrl = (__bridge WKUserContentController*)m_controller;
    [userCtrl addUserScript:userScript];
}

void sciter_wkwebview::eval(const std::string &js) {
    NSString* strJS = [NSString stringWithUTF8String:js.c_str()];
    WKWebView* webview = (__bridge WKWebView*)m_webview;
    [webview evaluateJavaScript:strJS completionHandler:nil];
}

void sciter_wkwebview::set_html(const std::string& html) {
    WKWebView* webview = (__bridge WKWebView*)m_webview;
    NSString* strHtmlContent = [NSString stringWithUTF8String:html.c_str()];
    [webview loadHTMLString:strHtmlContent baseURL:NSBundle.mainBundle.bundleURL];
}

void sciter_wkwebview::dispatch(std::function<void()> f) {
    dispatch_async(dispatch_get_main_queue(), ^{
        f();
    });
}

void sciter_wkwebview::set_navigation_callback(const navigation_callback_t &cb) {
    m_navigationCallback = cb;
}

void sciter_wkwebview::set_msg_callback(const msg_callback_t& cb) {
    m_msgCallback = cb;
}

void sciter_wkwebview::set_allowWindowOpen(const std::string& val) {
    m_allowWindowOpen = val;
}

void sciter_wkwebview::set_allowContextMenu(const std::string& val) {
    //m_allowContextMenu = val; ???
}

std::string sciter_wkwebview::currentSrc() {
    WKWebView* webview = (__bridge WKWebView*)m_webview;
    NSString* url = webview.URL.absoluteString;
    return url.UTF8String;
}

}
