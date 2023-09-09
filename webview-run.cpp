

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

void write_cons(std::string s, std::ostream& out=std::cout) {
#ifdef _WIN32
  char title[256];
  std::string tit="";
  if (GetConsoleTitle(title, 256) > 0) tit=title;

  if (tit.find("invisible cygwin console") != std::string::npos) {
    out << s; out.flush();
  } else {
    HANDLE ho;
    if (&out == &std::cerr) ho=GetStdHandle(STD_ERROR_HANDLE);
    else ho=GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsole(ho, s.c_str(), (DWORD)s.size(), nullptr, nullptr);
#else
    out << s; out.flush();
#endif
  }

  if (w != nullptr) w->eval("console.log('"+s+"');");
}

void lsdir(const std::string seq, const std::string req, void *arg) {
  auto dir=webview::detail::json_parse(req, "", 0);
  auto res=listdir(dir);
  std::cout << res << std::endl;
  rep_bs(res);
  auto result="{\"value\": \""+res+"\"}";
  w->resolve(seq, 0, result);
}

void create_binds(webview::webview &w) {
  w.bind("toto", [&](const std::string & ) -> std::string { return "{\"value\":\"tutu\"}"; });

  // Local file system function
  w.bind("ls", [&](const std::string &seq, const std::string &req, void *) { lsdir(seq, req, nullptr); }, nullptr);

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


  w.bind("write", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s);
    return "";
  });

  w.bind("writeln", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s);
    write_cons("\n");
    return "";
  });

  w.bind("ewrite", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s, std::cerr);
    return "";
  });

  w.bind("ewriteln", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s, std::cerr);
    write_cons("\n", std::cerr);
    return "";
  });
}

bool run_and_exit=false;
void webview_set(bool devmode, int width, int height, int hints, bool _run_and_exit) {
  if (w == nullptr) {
    void *wnd=nullptr;
    run_and_exit=_run_and_exit;

    if (run_and_exit) {
#ifdef _WIN32
      if (AttachConsole(ATTACH_PARENT_PROCESS) == 0) {
        run_and_exit=false;
        devmode=true;
      } else {
        HWND hwnd;
        extern HWND CreateWin();
        hwnd=CreateWin();
        wnd=&hwnd;
      }
#endif
    }

    w=new webview::webview(devmode, (void *)wnd);
    w->set_size(width, height, hints);
    create_binds(*w);
  }
}

void webview_run(std::string url, std::string title, std::string init_js) {
  w->set_title(title);

  if (!init_js.empty()) w->init(init_js);
  if (run_and_exit) {
    w->init("webapp_exit();");
    w->set_html("");
  } else {
    if (url.starts_with("html://")) {
      w->set_html(url);
    } else {
      w->navigate(url);
    }
  }

  w->run();
  delete w;
}


