
#include "webview.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>


void create_bind(webview::webview &w) {
static unsigned int count = 0;
  // A binding that increments a value and immediately returns the new value.
  w.bind("increment", [&](const std::string & /*req*/) -> std::string {
    auto count_string = std::to_string(++count);
    return "{\"count\": " + count_string + "}";
  });

  // A binding that creates a new thread and returns the result at a later time.
  w.bind(
    "compute",
    [&](const std::string &seq, const std::string &req, void * /*arg*/) {
      // Create a thread and forget about it for the sake of simplicity.
      std::thread([&, seq, req] {
        // Simulate load.
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        // json_parse() is an implementation detail and is only used here
        // to provide a working example.
        auto left = std::stoll(webview::detail::json_parse(req, "", 0));
        auto right = std::stoll(webview::detail::json_parse(req, "", 1));
        auto result = std::to_string(left + right);
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

  w.run();
}


