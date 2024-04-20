
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

#ifndef _WIN32
typedef void* HWND;
#endif


bool devmode = false, runjs_and_exit = false, html_string = false;
std::string url, title = "", init_js = "";

std::string add_bro_args="";
void set_browser_args(char, std::string, std::string val)
{
  std::cout << "Adding browser argument [--" << val << "]" << std::endl;
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

int width=-1, height=-1, hints=-1;
#ifdef _WIN32
int xpos=-1, ypos=-1;
bool minimized=false;
#endif
std::string icon_file="";

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
     [](char, std::string, std::string val) -> void { title = val; }},
    {"js", 'j', opt_only, required_argument, "Inject a javascript command before loading html page.",
     [](char, std::string, std::string val) -> void { init_js = val; }},
    {"runjs", 'r', opt_only, required_argument, "Run the provided javascript command and exit.",
     [](char, std::string, std::string val) -> void {
       init_js = val;
       runjs_and_exit = true;
     }},
    {"debug", 'd', opt_only, no_argument, "Activate the developper mode in the webview.",
     [](char, std::string, std::string) -> void { devmode = true; }},
    {"icon", 'n', opt_only, required_argument, "Set windows icon with the provided .ico file.",
     [](char, std::string, std::string val) -> void { icon_file=val; }},
#ifdef _WIN32
    {"xpos", 'x', opt_only, required_argument, "Set webview windows initial x position.",
     [](char, std::string, std::string val) -> void { xpos = std::stoi(val); }},
    {"ypos", 'y', opt_only, required_argument, "Set webview windows initial y position.",
     [](char, std::string, std::string val) -> void { ypos = std::stoi(val); }},
    {"minimized", 'm', opt_only, no_argument,
     "The webview window will be minimized at startup, a javascript call to the function 'webapp_restore()' will be necessary to restore it to its normal size.",
     [](char, std::string, std::string) -> void { minimized = true; }},
#endif
    {"width", 'w', opt_only, required_argument, "Set webview windows initial witdh.",
     [](char, std::string, std::string val) -> void { width = std::stoi(val); }},
    {"height", 'h', opt_only, required_argument, "Set webview windows initial height.",
     [](char, std::string, std::string val) -> void { height = std::stoi(val); }},
    {"hints", 'i', opt_only, required_argument,
     "Set webview hints => 0: width and height are default size, 1 set them as "
     "minimum bound, 2 set them as maximum bound. 3 they are fixed. Any other "
     "value is ignored.",
     [](char, std::string, std::string val) -> void { hints = std::stoi(val); }},
};


std::vector<std::string> idxs = {
  "webview.min.html",
  "webview.html",
  "index.min.html",
  "index.html",
  "index.min.js",
  "index.js"
};
std::string get_index()
{
  for (auto& sidx : idxs) {
    std::string idx = std::filesystem::absolute(sidx).generic_string();
    if (std::filesystem::is_regular_file(idx))
    {
//      std::cout << "Found index file: " << idx << std::endl;
      return idx;
    }
  }
  return "";
}

webview_wrapper w;

HWND *wnd=nullptr;


bool run_and_exit = false;
void webview_set(bool devmode = false, bool _run_and_exit = false)
{
//  void *wnd = nullptr;
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
  //w.conf.debug = devmode;
  w.conf = { devmode, true, true, true, true, true };
  w.create((void *)wnd);
  create_binds(w);
}

void GetGeom (HWND hw, int& x, int& y, int& w, int& h) {
#ifdef _WIN32
  RECT rc;
  GetWindowRect(hw, &rc);
  x=rc.left;
  y=rc.top;
  w=rc.right-rc.left;
  h=rc.bottom-rc.top;
  //std::cout << "GetWindow x " << x << ", y " << y << ", w " << w << ", h " << h << std::endl;
#endif
}

void webview_run(std::string url, std::string title = "", std::string init_js = "")
{
  // std::cout << "URL " << url << ", TITLE " << title << ", JS " << init_js <<
  // std::endl;
  w.set_title(title);
  if (icon_file != "") w.set_icon(icon_file);

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


#ifdef _WIN32
  HWND hw=(HWND)(w.window());
  bool pre_size=(xpos > 0 && ypos > 0 && width > 0 && height > 0);

  if  (pre_size) {
    int xg, yg, wg, hg;
    // ==> must patch webview.h 0.11.0 at line 2642 with : 'ShowWindow(m_window, SW_SHOWMINIMIZED);'
    GetGeom(hw, xg, yg, wg, hg);
    //std::cout << "xpos " << xpos << ", ypos " << ypos << ", wi " << width << ", he " << height << std::endl;
    if (xpos < 0) xpos=xg;
    if (xpos < 0) xpos=0;
    if (ypos < 0) ypos=yg;
    if (ypos < 0) ypos=0;
    if (width < 0) width=wg;
    if (width < 0) width=640;
    if (height < 0) height=hg;
    if (height < 0) height=400;

    //std::cout << "xpos " << xpos << ", ypos " << ypos << ", wi " << width << ", he " << height << std::endl;
    if (!minimized) {
      MoveWindow(hw, xpos, ypos, width, height, FALSE);
      ShowWindow(hw, SW_RESTORE);
      MoveWindow(hw, xpos, ypos, width, height, TRUE);
    } else {
      ShowWindow(hw, SW_RESTORE);
    }
  }

  if (!minimized) {
  // IsWindowVisible
  // LONG lStyles = GetWindowLong(GWL_STYLE); if( lStyles & WS_MINIMIZE )
    ShowWindow(hw, SW_RESTORE);
  }

  if (hints > -1 && hints < 4) {
    std::cout << "set_hints" << std::endl;
    w.set_hints(hints);
  }
#else
  if ( width > 0 || height > 0 || (hints > -1 && hints < 4)) {
    if (width < 0) width=640;
    if (height < 0) height=480;
    if (hints < 0) hints=0;
    w.set_size(width, height, hints);
  }
#endif

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


/* Because of CORS we have to disable web security with WebView2 in order to alow to send http queries from the app to remote servers
 From https://github.com/MicrosoftEdge/WebView2Feedback/issues/4166, env var "WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS" with following values :
--disable-web-security # Tested fetch works with wiki.space.thales
--allow-insecure-localhost
--unsafely-treat-insecure-origin-as-secure=http:*
--disable-site-isolation-trials
--disable-block-insecure-private-network-requests
--enable-insecure-private-network-requests-allowed
--disable-private-network-access-respect-preflight-results
--disable-features=AutoupgradeMixedContent,PrivateNetworkAccessSendPreflights,PrivateNetworkAccessRespectPreflightResults,BlockInsecurePrivateNetworkRequests
--enable-features=PrivateNetworkAccessNonSecureContextsAllowed,InsecurePrivateNetworkRequestsAllowed
--enable-blink-features=PrivateNetworkAccessNonSecureContextsAllowed,InsecurePrivateNetworkRequestsAllowed

 See also : https://github.com/MicrosoftEdge/WebView2Feedback/issues/4166
 See there : https://observablehq.com/@mbostock/fetch-with-basic-auth
*/

#ifndef _WIN32
int main(int argc, char **argv, char **)
{
#else
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
  LPSTR *argv;
  int argc;
  argv = CommandLineToArgv(GetCommandLine(), &argc);

  // Set default value of WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS to --disable-web-security
  if (add_bro_args == "") my_setenv("WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", "--disable-web-security --unsafely-treat-insecure-origin-as-secure=http:* --unsafely-treat-insecure-origin-as-secure=https:*");
  // Or to the ones provided through repetitive call to the -b options
  else my_setenv("WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", add_bro_args);
  // Eventuellement voir ICoreWebView2EnvironmentOptions, put_AdditionalBrowserArguments
  // Dans https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2environmentoptions?view=webview2-1.0.2210.55
  // extern std::string do_fstat(std::string sp);
  // std::cout << "fstat1" << std::endl;
  // do_fstat(url);
#endif

  getopt_init(argc, argv, r_opts, "WebView app.", "", "(c) Denis LALANNE. Provided as is. NO WARRANTY of any kind.");

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

  webview_set(devmode, runjs_and_exit);

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

