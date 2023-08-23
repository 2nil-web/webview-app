
#include "webview.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>
#include <algorithm>
#include <filesystem>

#include "util.h"


void create_bind(webview::webview &w) {
  // A binding that return the result of a command exec at a later time.
  w.bind(
    "exec_cmd",
    [&](const std::string &seq, const std::string &req, void *) {
      std::thread([&, seq, req] {
        auto cmd=webview::detail::json_parse(req, "", 0);
        std::string res_cmd=exec_cmd(cmd);
        rep_crlf(res_cmd);
        auto result="{\"value\": \""+res_cmd+"\"}";
        std::cout << "SEQ " << seq << ", REQ " << cmd << std::endl << result << std::endl;
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );

  w.bind(
    "increment",
    [&](const std::string & /*req*/) -> std::string {
      static unsigned int count = 0;
      auto count_string = std::to_string(++count);
      return "{\"count\": " + count_string + "}";
    });

  // A binding that creates a new thread and returns the result at a later time.
  w.bind(
    "compute",
    [&](const std::string &seq, const std::string &req, void * /*arg*/) {
      // Create a thread and forget about it for the sake of simplicity.
      std::thread([&, seq, req] {
        std::cout << "SEQ " << seq << ", REQ " << req << std::endl;
        // Simulate load.
        //std::this_thread::sleep_for(std::chrono::seconds(1));
/*        auto left = std::stoll(webview::detail::json_parse(req, "", 0));
        auto right = std::stoll(webview::detail::json_parse(req, "", 1));
        auto result = std::to_string(left + right);*/
        auto left = webview::detail::json_parse(req, "", 0);
        auto right = webview::detail::json_parse(req, "", 1);
        auto result = "{\"value\": \"res\"}";
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );
}

void run_webview(bool devmode, void *wnd, int width, int height, int hints, std::string url, std::string title, std::string init_js) {
  webview::webview w(devmode, wnd);

  if (width > -1 && height > -1) w.set_size(width, height, hints);

  create_bind(w);

  bool html=false;
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

  if (!init_js.empty()) w.init(init_js);

  w.run();
}


