

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>
#include <algorithm>
#include <filesystem>

#include <webview.h>
#include "util.h"


void create_binds(webview::webview &w) {
  // Change window title
  w.bind("webapp_title", [&](const std::string & req)  -> std::string {
      auto title=webview::detail::json_parse(req, "", 0);
      w.set_title(title);
      return "";
    });

  // Change window dimension and sizing behaviour
  w.bind("webapp_size", [&](const std::string & req) -> std::string {
      auto params=webview::detail::json_parse(req, "", 0);
      auto l_width = std::stoi(webview::detail::json_parse(req, "", 0));
      auto l_height = std::stoi(webview::detail::json_parse(req, "", 1));
      auto l_hints = std::stoi(webview::detail::json_parse(req, "", 2));
      w.set_size(l_width, l_height, l_hints);
      return "";
    });

  // Exit from the web application
  w.bind("webapp_exit", [&](const std::string &) -> std::string { w.terminate(); return ""; });

  // Exit from the web application on certain key
  w.bind("webapp_exit_on", [&](const std::string &req) -> std::string {
      auto k = std::stoi(webview::detail::json_parse(req, "", 27));
      w.terminate();
      return "";
    });

  // Run a local command and return an eventual result at a later time.
  w.bind("webapp_exec", [&](const std::string &seq, const std::string &req, void *) {
      std::thread([&, seq, req] {
        auto cmd=webview::detail::json_parse(req, "", 0);
        std::string res_cmd=exec_cmd(cmd);
        //std::cout << res_cmd << std::endl;
        replace_all(res_cmd, "\\", "<BACKSLASH_CODE>");
        rep_crlf(res_cmd);
        replace_all(res_cmd, "<BACKSLASH_CODE>", "\\\\");
        auto result="{\"value\": \""+res_cmd+"\"}";
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );
}

webview::webview *W=nullptr;
WNDPROC currentProc=nullptr;
LRESULT CALLBACK KeyProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
  switch(msg) {
    case WM_KEYDOWN:
      std::cout << "kproc:" << wParam << std::endl;

      if (wParam == VK_ESCAPE)
        if (W != nullptr) W->terminate();
      break;

    case WM_NOTIFY:
      std::cout << "kproc:" << wParam << std::endl;
      if (HIBYTE(GetKeyState(VK_ESCAPE)) != 0 && GetFocus() != hwnd)
        if (W != nullptr) W->terminate();
      break;
      default:
        if (currentProc != nullptr)
          return CallWindowProc((WNDPROC)currentProc, hwnd, msg, wParam, lParam);
  }

    return 0;
}

void run_webview(bool devmode, void *wnd, int width, int height, int hints, std::string url, std::string title, std::string init_js) {
  webview::webview w(devmode, wnd);

  if (width < 0) width=640;
  if (height < 0) height=480;
  w.set_size(width, height, hints);

  create_binds(w);

  if (!init_js.empty()) w.init(init_js);

  if (url.starts_with("html://")) {
    if (title == "") title="HTML string";
    url.erase(0, 7);
    w.set_title(title);
    w.set_html(url);
  } else {
    if (title == "") title=url;
    w.set_title(title);
    w.navigate(url);
  }
#ifdef _WIN32
  W=&w;
  HWND hw=(HWND)w.window();
  //currentProc=(WNDPROC)SetWindowLongPtr(hw, GWLP_WNDPROC, (long)KeyProc);
  currentProc=reinterpret_cast<WNDPROC>(SetWindowLongPtr(hw, GWLP_WNDPROC, (LONG_PTR)KeyProc));
  
#endif
  w.run();
}


