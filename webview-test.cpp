
#include "webview.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>


unsigned int count = 0;
void create_bind(webview::webview &w) {
  // A binding that increments a value and immediately returns the new value.
  w.bind("increment", [&](const std::string & /*req*/) -> std::string {
    auto count_string = std::to_string(++count);
    return "{\"count\": " + count_string + "}";
  });

  // An binding that creates a new thread and returns the result at a later time.
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


#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
#else
int main() {
#endif
  webview::webview w(false, nullptr);
//  w.set_size(480, 320, WEBVIEW_HINT_NONE);

  create_bind(w);

  // Run the html page
  std::string param=lpCmdLine;
  std::cout << "Param [" << param << "]" << std::endl;
  if (!param.empty()) {
    w.set_title(param);
    w.navigate(param);
  } else {
    w.set_title("Missing parameter");
    w.set_html("Pass a url or an html file as parameter to the program.");
  }

  w.run();

  return 0;
}

