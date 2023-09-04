

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
#include "winapi.h"

webview::webview *w=nullptr;

void create_binds(webview::webview &w) {
  // Change window title
  w.bind("webapp_get_title", [&](const std::string &seq, const std::string &req, void *) {
     std::thread([&, seq, req] {
        std::string prev_title="";
#ifdef _WIN32
        prev_title=GetWindowText((HWND)w.window());
#endif
        auto result="{\"value\": \""+prev_title+"\"}";
        //auto result=res_cmd;
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );

  w.bind("webapp_title", [&](const std::string &seq, const std::string &req, void *) {
      std::thread([&, seq, req] {
        std::string prev_title="";
#ifdef _WIN32
        prev_title=GetWindowText((HWND)w.window());
#endif
        auto title=webview::detail::json_parse(req, "", 0);
        w.set_title(title);
        auto result="{\"value\": \""+prev_title+"\"}";
        //auto result=res_cmd;
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );

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


  // Run a local command and return an eventual result at a later time.
  w.bind("webapp_exec", [&](const std::string &seq, const std::string &req, void *) {
      std::thread([&, seq, req] {
        auto cmd=webview::detail::json_parse(req, "", 0);
        std::string res_cmd=exec_cmd(cmd);
        //std::cout << res_cmd << std::endl;
        std::string bs;
        bs=(char)92;
        replace_all(res_cmd, bs, "##BACKSLASH_CODE##");
        rep_crlf(res_cmd);
        replace_all(res_cmd, "##BACKSLASH_CODE##", bs+bs);
        auto result="{\"value\": \""+res_cmd+"\"}";
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );

  // Local file system function
  w.bind("ls", [&](const std::string &seq, const std::string &req, void *) {
     std::thread([&, seq, req] {
        auto dir=webview::detail::json_parse(req, "", 0);
        auto res=listdir(dir);
        std::cout << res << std::endl;
        rep_bs(res);
        auto result="{\"value\": \""+res+"\"}";
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );

  w.bind("lsi", [&](const std::string &req) -> std::string {
    auto res=listdir("");
    rep_bs(res);
    return "{\"value\": \""+res+"\"}";
  });


}

void *webview_set(bool devmode, int width, int height, int hints) {
  void *wnd=nullptr;

  if (w == nullptr) {
    w=new webview::webview(devmode, wnd);
    w->set_size(width, height, hints);
    create_binds(*w);
  }

  return wnd;
}

void webview_run(std::string url, std::string title, std::string init_js) {
  w->set_title(title);
  if (!init_js.empty()) w->init(init_js);

  if (url.starts_with("html://")) {
    w->set_html(url);
  } else {
    w->navigate(url);
  }

  w->run();
}


