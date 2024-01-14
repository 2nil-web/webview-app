
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

  w.create(devmode, (void *)wnd);
  create_binds(w);
}

void DisplayGeom (HWND hw) {
  RECT rc;
  GetWindowRect(hw, &rc);
  std::cout << "x " << rc.left << ", y " << rc.top << ", w " << rc.right-rc.left << ", h " << rc.bottom-rc.top << std::endl;
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

  HWND hw=(HWND)(w.window());

  // ShowWindow(m_window, SW_SHOWMINIMIZED); at line 2642 of webview.h
  DisplayGeom(hw);
// IsWindowVisible
// LONG lStyles = GetWindowLong(GWL_STYLE); if( lStyles & WS_MINIMIZE )

//  if (ini_x > -1 && ini_y > -1) {
    MoveWindow(hw, 20, 20, 400, 300, FALSE);
    ShowWindow(hw, SW_SHOWNORMAL);
    MoveWindow(hw, 20, 20, 400, 300, TRUE);
//  } else {
//    ShowWindow(hw, SW_SHOWNORMAL);
//  }
  DisplayGeom(hw);

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

/* From https://github.com/MicrosoftEdge/WebView2Feedback/issues/4166, env var "WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS" with following values :
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

