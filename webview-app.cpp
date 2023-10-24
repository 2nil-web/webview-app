
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include <thread>
#include <filesystem>

#include "wv-util.h"
#include "wv-opts.h"
#include "wv-binds.h"

bool devmode=false;
bool runjs_and_exit=false;
bool html_string=false;
int width=-1, height=-1, hints=0; /*
0 Width and height are default size
1 Width and height are minimum bounds
2 Width and height are maximum bounds
3 Window size is fixed
*/

std::string url, title="", init_js="";

// short_opt, long_opt, value of opt or empty string

void set_url(char, std::string, std::string val) {
//  url=val;
  url=std::filesystem::absolute(val).generic_string();
  if (title.empty()) title=url;
}

void set_html(char, std::string, std::string val) {
  url="html://";
  url+=val;
  html_string=true;
  if (title.empty()) title="HTML string";
}

void set_title(char, std::string, std::string val) {
  title=val;
}

void set_hints(char, std::string, std::string val) {
  hints=std::stoi(val);

  if (hints < 0 || hints > 3) {
    std::cerr << "hints value must be an integer value from 0 to 3, defaulting to zero." << std::endl;
    hints=0;
  }
}

std::vector<run_opt> r_opts = {
  { "url",    'f', opt_only,  required_argument, "Provide the url. Default is to search for index.html or index.js in the current directory, if there is no '-u' option this information might also be provided as the last argument of the command.", set_url },
  { "html",   'c', opt_only,  required_argument, "Provide an html string.", set_html },
  { "", '\0', 0, 0, "-f and -c are mutually exclusive.", nullptr },
  { "title",  't', opt_only,  required_argument, "Set the title of the webview windows, default is to display the url as title if it is provided or nothing if just an html string is provided.", set_title },
  { "js",     'j', opt_only,  required_argument, "Inject a javascript command before loading html page.", [] (char , std::string , std::string val) -> void { init_js=val; } },
  { "runjs",  'r', opt_only,  required_argument, "Run the provided javascript command and exit.", [] (char , std::string , std::string val) -> void {
      init_js=val;
      runjs_and_exit=true;
    }
  },
  { "debug",  'd', opt_only,  no_argument,       "Activate the developper mode in the webview.",  [] (char , std::string , std::string ) -> void { devmode=true; }},
  { "width",  'w', opt_only,  required_argument, "Set webview windows initial witdh (Default is 640).",  [] (char , std::string , std::string val) -> void { width=std::stoi(val); }},
  { "height", 'h', opt_only,  required_argument, "Set webview windows initial height (Default is 480).", [] (char , std::string , std::string val) -> void { height=std::stoi(val); }},
  { "hints",  'i', opt_only,  required_argument, "Set webview hints => 0: width and height are default size, 1 set them as minimum bound, 2 set them as maximum bound. 3 they are fixed. Any other value is defaulted to 0 with a warning.", set_hints }

/*
  { "", '\0', 0, 0, "\nAdditionnal help message.", nullptr },
  { "", '\0', 0, 0, "", nullptr },
  { "", '\0', 0, 0, "\n2nd Additional message.", nullptr }*/
};


std::string get_index() {
  //std::cout << "get_index" << std::endl;

  std::string idx=std::filesystem::absolute("index.html").generic_string();
  if (std::filesystem::is_regular_file(idx)) {
    std::cout << "html " << idx << std::endl;
    return idx;
  }

  idx=std::filesystem::absolute("index.js").generic_string();
  if (std::filesystem::is_regular_file(idx)) {
    std::cout << "js " << idx << std::endl;
    return idx;
  }

  return "";
}


// Live html test :
// ./webview-app.exe -c "<input type='button' value='Exit web app' onclick='exit_webapp()'>"
// ./webview-app.exe -f index.html
// ./webview-app.exe index.html
// ./webview-app.exe
// ./webview-app.exe -j "window.confirm('Hello')"
// ./webview-app.exe -r "for(i=0; i < 10; i++) writeln(i);" # under a cygwin/mingw mintty console
// start /wait webview-app.exe -r "for(i=0; i < 10; i++) writeln(i);" # under a windows cmd
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/) {
  LPSTR *argv;
  int argc;
  argv=CommandLineToArgv(GetCommandLine(), &argc);
#else
int main(int argc, char **argv, char **) {
#endif
  getopt_init(argc, argv, r_opts, "WebView app.", "", "(c) Denis LALANNE. Provided as is. NO WARRANTY of any kind.");

  if (url.empty()) {
    if (optind < argc) {
      url=argv[optind];

      if (url.starts_with("file://"))
        url=std::filesystem::absolute(url.substr(7)).generic_string();
      else if (!url.starts_with("html://") && !url.starts_with("http://") && !url.starts_with("https://"))
        url=std::filesystem::absolute(url).generic_string();
    } else {
      // Search for index.html or index.js in current directory
      url=get_index();

      if (url.empty()) {
        if (title.empty()) title="Missing parameter";
        url="html://";
        std::string hm=usage();
        replace_all(hm, "\r", "<br>");
        replace_all(hm, "\n", "<br>");
        if (!hm.empty()) url+="<pre>"+hm+"</pre>";
      }
    }
  }

  if (width < 0) width=640;
  if (height < 0) height=480;
  webview_set(devmode, width, height, hints, runjs_and_exit);

  if (url.starts_with("html://")) {
    if (title == "") title="HTML string";
    url.erase(0, 7);
  } else {
    if (title == "") title=url;
  }

  //std::cout << url << std::endl;
  webview_run(url, title, init_js);

  return 0;
}

