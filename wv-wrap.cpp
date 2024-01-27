
#include <iostream>
#include <string>

#include <webview.h>

#include "wv-util.h"
#ifdef _WIN32
#include "wv-winapi.h"
#endif
#include "wv-wrap.h"

#define WP ((webview::webview *)w)


/*
webview.h 
line 879 : class gtk_webkit_engine
line 1077 : using browser_engine = detail::gtk_webkit_engine;

line 1170 : class cocoa_wkwebview_engine
line 1565 : using browser_engine = detail::cocoa_wkwebview_engine;

line 2484 : class win32_edge_engine
line 2890 : using browser_engine = detail::win32_edge_engine;
*/

#ifdef _WIN32
#include <oleacc.h>
#pragma comment(lib,"Oleacc.lib")
webview_wrapper *webview_wrapper::me;

void DisplayWindowRect(HWND hw) {
  RECT rc;
  GetWindowRect(hw, &rc);
  std::cout << "left:" << rc.left << ", top:" << rc.top << ", right:" << rc.right << ", bottom:" << rc.bottom << std::endl;
}

// Callback functions that handles events.
void CALLBACK webview_wrapper::HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
  if (me && me->g_hook == hook && ((webview::webview *)(me->w))->window() == hwnd) {
    static bool firstEvent=true, startCloseButton=false;
    if (firstEvent) {
      firstEvent=false;
    }

    if (event == EVENT_OBJECT_STATECHANGE && idChild == 5) startCloseButton=true;
    else {
      if (startCloseButton && event == RI_MOUSE_RIGHT_BUTTON_UP && idChild == 0) {
        //std::cout << "bye bye" << std::endl;
        me->terminate();
      }
      startCloseButton=false;
    }
  
    //std::cout << std::hex << "hwnd " << hwnd << "event:" << event << ", idObject:" << idObject << ", idChild:" << idChild << std::endl;
    //<< ", dwEventThread:" << dwEventThread << ", dwmsEventTime:" << dwmsEventTime << std::endl << std::dec;
    //DisplayWindowRect(hwnd);
  }
}

void webview_wrapper::InitializeMSAA()
{
  /*HRESULT hrCoInit = */CoInitialize(NULL);
  g_hook = SetWinEventHook(EVENT_MIN, EVENT_MAX, NULL, &HandleWinEvent, GetProcessId(GetCurrentProcess()), 0, 0);
  me=this;
}

// Unhooks the event and shuts down COM.
void webview_wrapper::ShutdownMSAA()
{
    UnhookWinEvent(g_hook);
    CoUninitialize();
}

#endif

void webview_wrapper::create(bool debug, void *wnd)
{
  if (w != nullptr)
    return;
  std::cout << "Befor constructor " << std::hex << w << std::endl;
  w = new webview::webview(debug, wnd);
  std::cout << "After constructor " << std::hex << w << std::endl;

#ifdef _WIN32
  InitializeMSAA();
#endif

  bind_doc("webapp_help", "return a help message.", [&](const std::string &req) -> std::string {
    auto arg1 = json_parse(req, "", 0);
    std::string res, s = "";

    if (arg1 == "json")
    {
      for (auto fh : func_help)
      {
        s += "\"" + fh.first + "\":\"" + fh.second + "\",";
      }

      // Remove last comma
      s.pop_back();
      res = "{" + s + "}";
    }
    else
    {
      for (auto fh : func_help)
      {
        s += fh.first + ':' + fh.second + '\n';
      }
      res = '"' + to_htent(s) + '"';
    }

    // std::cout << res << std::endl;
    return res;
  });
}


webview_wrapper::webview_wrapper(bool debug, void *wnd)
{
  create(debug, wnd);
}


webview_wrapper::webview_wrapper()
{
}


webview_wrapper::~webview_wrapper()
{
  delete WP;
}


void webview_wrapper::navigate(const std::string &url)
{
  WP->navigate(url);
}

void webview_wrapper::bind(const std::string &name, sync_binding_t fn)
{
  WP->bind(name, fn);
}

void webview_wrapper::bind(const std::string &name, binding_t fn, void *arg)
{
  WP->bind(name, fn, arg);
}

void webview_wrapper::unbind(const std::string &name)
{
  std::cout << "wrap_unbind" << std::endl;
  WP->unbind(name);
}

void webview_wrapper::resolve(const std::string &seq, int status, const std::string &result)
{
  WP->resolve(seq, status, result);
}

void webview_wrapper::run()
{
  WP->run();
}

void *webview_wrapper::window()
{
  return WP->window();
}

void webview_wrapper::terminate()
{
  if (onexit_func != "") {
    std::cout << "Run " << onexit_func << std::endl;
    eval(me->onexit_func);
  }

  std::cout << "wrap_terminate" << std::endl;
  WP->terminate();
}

void webview_wrapper::dispatch(std::function<void()> f)
{
  WP->dispatch(f);
}

void webview_wrapper::set_title(const std::string &title)
{
  WP->set_title(title);
}

void webview_wrapper::minimize()
{
#ifdef _WIN32
  ShowWindow((HWND)WP->window(), SW_SHOWMINIMIZED);
#endif
}

void webview_wrapper::restore()
{
#ifdef _WIN32
  ShowWindow((HWND)WP->window(), SW_RESTORE);
#endif
}

void webview_wrapper::get_pos(int& x, int& y)
{
#ifdef _WIN32
  HWND hw=(HWND)WP->window();
  RECT rc;
  GetWindowRect(hw, &rc);
  x=rc.left;
  y=rc.top;
#endif
}

void webview_wrapper::set_pos(int x, int y)
{
#ifdef _WIN32
  HWND hw=(HWND)WP->window();
  SetWindowPos(hw, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
#endif
}

void webview_wrapper::set_size(int width, int height, int hints)
{
  //std::cout << "set_size w " << width << ", h " << height << ", hints " << hints << std::endl;
  if (hints < 0) {
#ifdef _WIN32
  HWND hw=(HWND)WP->window();
  SetWindowPos(hw, NULL, 0, 0, width, height, SWP_NOREPOSITION | SWP_NOZORDER);
#else
  hints=0;
#endif
  }

  WP->set_size(width, height, hints);
}

void webview_wrapper::set_hints(int hints)
{
  if (hints > -1 && hints < 4) {
#ifdef _WIN32
    RECT rc;
    HWND hw=(HWND)WP->window();
    GetWindowRect(hw, &rc);
    set_size(rc.right-rc.left, rc.bottom-rc.top, hints);
#endif
  }
}

void webview_wrapper::set_onexit(const std::string js)
{
  onexit_func=js;
}

void webview_wrapper::set_html(const std::string &html)
{
  WP->set_html(html);
}

void webview_wrapper::init(const std::string &js)
{
  WP->init(js);
}

void webview_wrapper::eval(const std::string &js)
{
  WP->eval(js);
}

std::string json_parse(const std::string &s, const std::string &key, const int index)
{
  return webview::detail::json_parse(s, key, index);
}


void webview_wrapper::bind_doc(const std::string &name, const std::string desc, sync_binding_t fn, bool indoc)
{
  WP->bind(name, fn);
  if (indoc)
    func_help.push_back(make_pair(name, desc));
}


void webview_wrapper::bind_doc(const std::string &name, const std::string desc, binding_t fn, void *arg, bool indoc)
{
  WP->bind(name, fn, arg);
  if (indoc)
    func_help.push_back(make_pair(name, desc));
}


void webview_wrapper::bind_noprom(const std::string &name, sync_binding_t fn)
{
  WP->bind(name, fn);
}


std::string webview_wrapper::version()
{
  return std::string(webview_version()->version_number) + std::string(webview_version()->pre_release) +
         std::string(webview_version()->build_metadata);
}

