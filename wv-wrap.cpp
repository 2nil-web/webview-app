
#include <iostream>
#include <string>
#include <filesystem>

#include "my_webview.h"

#include "wv-util.h"
#ifdef _WIN32
#include "wv-winapi.h"
#include <commctrl.h>
#include "wv-wm_map.h"
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
    //static bool firstEvent=true; if (firstEvent) { firstEvent=false; }

    // Cf. winuser.h
    static bool startCloseButton=false;
    if (event == EVENT_OBJECT_STATECHANGE && idChild == 5) startCloseButton=true;
    else {
      if (startCloseButton && event == RI_MOUSE_RIGHT_BUTTON_UP && idChild == 0) {
        std::cout << "bye bye" << std::endl;
        if (me->on_exit_func != "") {
          std::cout << "bye bye " << me->on_exit_func << std::endl;
          me->eval(me->on_exit_func);
        }
        //me->terminate();
      }
      startCloseButton=false;
    }
  }
}

bool TryToChangeIcon(HWND hw, std::string icPfx, int icMet, int icTyp) {
  std::string icFn=icPfx;
  if (icMet > 0) icFn+=std::to_string(GetSystemMetrics(icMet));
  icFn+=".ico";

  std::string icFnAbs = std::filesystem::absolute(icFn).generic_string();

  if (std::filesystem::is_regular_file(icFnAbs)) {
    HICON hIc=(HICON)LoadImage(GetModuleHandle(NULL), icFnAbs.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (hIc) {
      std::cout << "Changing app icon type " << icTyp << " with file " << icFnAbs << std::endl;
      SendMessage(hw, WM_SETICON, icTyp, (LPARAM)hIc);
      SendMessage(GetWindow(hw, GW_OWNER), WM_SETICON, icTyp, (LPARAM)hIc);
      return true;
    }
  }
  return false;
}

  

void webview_wrapper::InitSpy()
{
#ifdef DO_WINHOOK
  CoInitialize(NULL);
  g_hook = SetWinEventHook(EVENT_MIN, EVENT_MAX, NULL, &HandleWinEvent, GetProcessId(GetCurrentProcess()), 0, 0);
#else
  HWND hw=(HWND)WP->window();
  std::cout << "Create HWND " << hw << std::endl;
  //if (!TryToChangeIcon(hw, "app_", SM_CXICON, ICON_BIG)) TryToChangeIcon(hw, "app", 0, ICON_BIG);
  //if (!TryToChangeIcon(hw, "app_", SM_CXSMICON, ICON_SMALL)) TryToChangeIcon(hw, "app", 0, ICON_SMALL);
  SetWindowSubclass(hw, myWindowProc, 0, 0);
  me=this;
#endif
}

// Unhooks the event and shuts down COM.
void webview_wrapper::ExitSpy()
{
#ifdef DO_WINHOOK
    UnhookWinEvent(g_hook);
    CoUninitialize();
#endif
}

LRESULT webview_wrapper::myWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
  if (me && ((webview::webview *)(me->w))->window() == hWnd && wm_map.count(uMsg)) {
    //std::cout << wm_map[uMsg] << std::endl;
    switch (uMsg) {
      case WM_CLOSE:
      case WM_QUIT:
      case WM_SYSCOMMAND:
      case WM_DESTROY:
        if (me->on_exit_func != "") {
          std::cout << "bye bye " << me->on_exit_func << std::endl;
          //me->init(me->on_exit_func);
          me->eval(me->on_exit_func);
        }
        break;
        /*
      case WM_SETICON:
      {
        std::string ico_fn = std::filesystem::absolute("app.ico").generic_string();
        if (std::filesystem::is_regular_file(ico_fn)) {
        if (wParam == ICON_BIG) { } else { }
          HICON hic=(HICON)LoadImage(GetModuleHandle(NULL), ico_fn.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
          std::cout << "Loading icon image " << ico_fn << std::endl;
          return (LRESULT)hic;
        }
      }
      return NULL;*/

      case WM_MOVE:
        if (me->on_move_func != "") {
          std::cout << "on move " << me->on_move_func << std::endl;
          me->eval(me->on_move_func);
        }
        break;
      default :
        break;
    }
  }
  return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


#endif

void webview_wrapper::create(bool debug, void *wnd)
{
  if (w != nullptr)
    return;
  w = new webview::webview(debug, wnd);

#ifdef _WIN32
  InitSpy();
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
  if (on_exit_func != "") {
    std::cout << "terminate " << on_exit_func << std::endl;
    eval(me->on_exit_func);
//    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

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

bool LoadIconIfExists(HWND hw, std::string ico) {
  std::string icoAbs = std::filesystem::absolute(ico).generic_string();
  std::cout << "Trying to load icon file " << ico << '(' << icoAbs << ')' << std::endl;

  if (std::filesystem::is_regular_file(icoAbs)) {
    std::cout << "Found icon file " << icoAbs << std::endl;
    HICON hIc=(HICON)LoadImage(GetModuleHandle(NULL), icoAbs.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (hIc) {
      std::cout << "Changing app icon with file " << icoAbs << std::endl;
      SendMessage(hw, WM_SETICON, ICON_BIG, (LPARAM)hIc);
      SendMessage(GetWindow(hw, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIc);
      SendMessage(hw, WM_SETICON, ICON_SMALL, (LPARAM)hIc);
      SendMessage(GetWindow(hw, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIc);
      return true;
    }
  }
  return false;
}

void webview_wrapper::set_icon(std::string file) {
#ifdef _WIN32
  HWND hw = (HWND)WP->window();
  LoadIconIfExists(hw, file);
#endif
}


void webview_wrapper::hide()
{
#ifdef _WIN32
  ShowWindow((HWND)WP->window(), SW_HIDE);
#endif
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

void webview_wrapper::get_size(int& wi, int& he)
{
#ifdef _WIN32
  HWND hw=(HWND)WP->window();
  RECT rc;
  GetWindowRect(hw, &rc);
  wi=rc.right-rc.left;
  he=rc.bottom-rc.top;
#endif
}


void webview_wrapper::set_size(int width, int height, int hints)
{
  //std::cout << "set_size w " << width << ", h " << height << ", hints " << hints << std::endl;
  if (hints < 0) {
#ifdef _WIN32
  //HWND hw=(HWND)WP->window();
  //SetWindowPos(hw, NULL, 0, 0, width, height, SWP_NOREPOSITION | SWP_NOZORDER);
  WP->my_set_size(width, height);
#else
  hints=0;
#endif
  } else WP->set_size(width, height, hints);
}

void webview_wrapper::set_hints(int hints)
{
  if (hints > -1 && hints < 4) {
#ifdef _WIN32
    RECT rc;
    HWND hw=(HWND)WP->window();
    GetWindowRect(hw, &rc);
    WP->set_size(rc.right-rc.left, rc.bottom-rc.top, hints);
#endif
  }
}

void webview_wrapper::set_on_move(const std::string js)
{
  on_move_func=js;
  std::cout << "set_on_move " << on_move_func << std::endl;
}

void webview_wrapper::set_on_exit(const std::string js)
{
  on_exit_func=js;
  std::cout << "set_on_exit " << on_exit_func << std::endl;
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

