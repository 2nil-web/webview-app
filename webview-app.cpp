
#ifdef _WIN32
#include "wv-winapi.h"
#endif

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "wv-opts.h"
#include "wv-util.h"
#include "wv-wrap.h"
#include "wv-binds.h"

bool devmode = false;
bool runjs_and_exit = false;
bool html_string = false;
int xpos=-1, ypos=-1, width = -1, height = -1, hints = 0; /*
      0 Width and height are default size
      1 Width and height are minimum bounds
      2 Width and height are maximum bounds
      3 Window size is fixed
      */

std::string url, title = "", init_js = "";

// short_opt, long_opt, value of opt or empty string

std::string add_bro_args="";
void set_browser_args(char, std::string, std::string val)
{
  add_bro_args+=" --"+val;
}

void set_url(char, std::string, std::string val)
{
  if (val.starts_with("http:") || val.starts_with("https:"))
  {
    url = val;
  }
  else
  {
    url = std::filesystem::absolute(val).generic_string();
  }

  // std::cout << "set_url " << url << std::endl;
  if (title.empty())
    title = url;
}

void set_html(char, std::string, std::string val)
{
  url = "html://";
  url += val;
  html_string = true;
  if (title.empty())
    title = "HTML string";
}

void set_title(char, std::string, std::string val)
{
  title = val;
}

void set_hints(char, std::string, std::string val)
{
  hints = std::stoi(val);

  if (hints < 0 || hints > 3)
  {
    std::cerr << "hints value must be an integer value from 0 to 3, defaulting "
                 "to zero."
              << std::endl;
    hints = 0;
  }
}

std::vector<run_opt> r_opts = {
#ifdef _WIN32
    {"browser-args", 'b', opt_only, required_argument,
     "Provide additional browser arguments to the webview2 component.",
     set_browser_args},
#endif
    {"url", 'f', opt_only, required_argument,
     "Provide the url. Default is to search for index.html or index.js in the "
     "current directory, if there is no '-u' option this information might "
     "also be provided as the last argument of the command.",
     set_url},
    {"html", 'c', opt_only, required_argument, "Provide an html string.", set_html},
    {"", '\0', 0, 0, "-f and -c are mutually exclusive.", nullptr},
    {"title", 't', opt_only, required_argument,
     "Set the title of the webview windows, default is to display the url as "
     "title if it is provided or nothing if just an html string is provided.",
     set_title},
    {"js", 'j', opt_only, required_argument, "Inject a javascript command before loading html page.",
     [](char, std::string, std::string val) -> void { init_js = val; }},
    {"runjs", 'r', opt_only, required_argument, "Run the provided javascript command and exit.",
     [](char, std::string, std::string val) -> void {
       init_js = val;
       runjs_and_exit = true;
     }},
    {"debug", 'd', opt_only, no_argument, "Activate the developper mode in the webview.",
     [](char, std::string, std::string) -> void { devmode = true; }},

    {"xpos", 'x', opt_only, required_argument, "Set webview windows initial x position (no default).",
     [](char, std::string, std::string val) -> void { xpos = std::stoi(val); }},
    {"ypos", 'y', opt_only, required_argument, "Set webview windows initial y position (no default).",
     [](char, std::string, std::string val) -> void { ypos = std::stoi(val); }},

    {"width", 'w', opt_only, required_argument, "Set webview windows initial witdh (Default is 640).",
     [](char, std::string, std::string val) -> void { width = std::stoi(val); }},
    {"height", 'h', opt_only, required_argument, "Set webview windows initial height (Default is 480).",
     [](char, std::string, std::string val) -> void { height = std::stoi(val); }},
    {"hints", 'i', opt_only, required_argument,
     "Set webview hints => 0: width and height are default size, 1 set them as "
     "minimum bound, 2 set them as maximum bound. 3 they are fixed. Any other "
     "value is defaulted to 0 with a warning.",
     set_hints}

    /*
      { "", '\0', 0, 0, "\nAdditionnal help message.", nullptr },
      { "", '\0', 0, 0, "", nullptr },
      { "", '\0', 0, 0, "\n2nd Additional message.", nullptr }*/
};

std::string get_index()
{
  // std::cout << "get_index" << std::endl;

  std::string idx = std::filesystem::absolute("index.html").generic_string();
  if (std::filesystem::is_regular_file(idx))
  {
    // std::cout << "html " << idx << std::endl;
    return idx;
  }

  idx = std::filesystem::absolute("index.js").generic_string();
  if (std::filesystem::is_regular_file(idx))
  {
    // std::cout << "js " << idx << std::endl;
    return idx;
  }

  return "";
}

webview_wrapper w;

bool run_and_exit = false;
void webview_set(bool devmode = false, int x=-1, int y=-1, int width = 640, int height = 480, int hints = 0, bool _run_and_exit = false)
{
  void *wnd = nullptr;
  run_and_exit = _run_and_exit;

#ifdef _WIN32
  if (run_and_exit)
  {
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
      // Console mode, webview window will not be rendered.
      // HWND hwnd; extern HWND CreateWin(); hwnd=CreateWin(); wnd=&hwnd;
    }
    else
    {
      // GUI mode is not compatible with run_and_exit option
      run_and_exit = false;
      devmode = true;
    }
  }
#endif

  //w.ini_pos(x, y);
  w.create(devmode, (void *)wnd);
  //w.hide();
  std::cout << "x " << x << ", y " << y << ", w " << width << ", h " << height << std::endl;
  w.set_size(width, height, hints);
  //if (x > -1 && y > -1) w.set_pos(x, y);
  create_binds(w);
 // w.show();
}

void webview_run(std::string url, std::string title = "", std::string init_js = "")
{
  // std::cout << "URL " << url << ", TITLE " << title << ", JS " << init_js <<
  // std::endl;
  w.set_title(title);

  if (run_and_exit)
  {
    if (init_js.back() != ';')
      init_js += ';';
    init_js += " webapp_exit();";
    w.init(init_js);

    w.set_html("html://<div></div>");
  }
  else
  {
    if (!init_js.empty())
    {
      w.init(init_js);
    }

    if (url.starts_with("html://"))
    {
      w.set_html(url);
    }
    else
    {
      if (!url.starts_with("http://") && !url.starts_with("https://"))
      {
        url = "file://" + url;
      }
      w.navigate(url);
    }
  }

  // std::cout << init_js << std::endl;
  w.run();
}


// Live html test :
// ./webview-app.exe -c "<input type='button' value='Exit web app'
// onclick='exit_webapp()'>"
// ./webview-app.exe -f index.html
// ./webview-app.exe index.html
// ./webview-app.exe
// ./webview-app.exe -j "window.confirm('Hello')"
// ./webview-app.exe -r "for(i=0; i < 10; i++) writeln(i);" # under a
// cygwin/mingw mintty console start /wait webview-app.exe -r "for(i=0; i < 10;
// i++) writeln(i);" # under a windows cmd
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
  LPSTR *argv;
  int argc;
  argv = CommandLineToArgv(GetCommandLine(), &argc);
  getopt_init(argc, argv, r_opts, "WebView app.", "", "(c) Denis LALANNE. Provided as is. NO WARRANTY of any kind.");

  // Set default value of WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS to --disable-web-security
  if (add_bro_args == "") my_setenv("WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", "--disable-web-security");
  // Or to the ones provided through repetitive call to the -b options
  else my_setenv("WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", add_bro_args);
  // Eventuellement voir ICoreWebView2EnvironmentOptions, put_AdditionalBrowserArguments
  // Dans https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2environmentoptions?view=webview2-1.0.2210.55
#else
int main(int argc, char **argv, char **)
{
  getopt_init(argc, argv, r_opts, "WebView app.", "", "(c) Denis LALANNE. Provided as is. NO WARRANTY of any kind.");
#endif
  // extern std::string do_fstat(std::string sp);
  // std::cout << "fstat1" << std::endl;
  // do_fstat(url);

  if (url.empty())
  {
    if (optind < argc)
    {
      url = argv[optind];

      if (url.starts_with("file://"))
        url = std::filesystem::absolute(url.substr(7)).generic_string();
      else if (!url.starts_with("html://") && !url.starts_with("http://") && !url.starts_with("https://"))
        url = std::filesystem::absolute(url).generic_string();
    }
    else
    {
      // Search for index.html or index.js in current directory
      url = get_index();

      if (url.empty())
      {
        if (title.empty())
          title = "Missing parameter";
        url = "html://";
        std::string hm = usage();
        replace_all(hm, "\r", "<br>");
        replace_all(hm, "\n", "<br>");
        if (!hm.empty())
          url += "<pre>" + hm + "</pre>";
      }
    }
  }

  if (width < 0)
    width = 640;
  if (height < 0)
    height = 480;
  webview_set(devmode, xpos, ypos, width, height, hints, runjs_and_exit);



  if (url.starts_with("html://"))
  {
    if (title == "")
      title = "HTML string";
    url.erase(0, 7);
  }
  else
  {
    if (title == "")
      title = url;
  }

  // std::cout << url << std::endl;
  webview_run(url, title, init_js);

  return 0;
}

