
#include <iostream>
#include <string>

#include <webview.h>

#include "wv-util.h"
#ifdef _WIN32
#include "wv-winapi.h"
#endif
#include "wv-wrap.h"

#define WP ((webview::webview *)w)


/*
webview.h 
line 879 : class gtk_webkit_engine
line 1077 : using browser_engine = detail::gtk_webkit_engine;

line 1170 : class cocoa_wkwebview_engine
line 1565 : using browser_engine = detail::cocoa_wkwebview_engine;

line 2484 : class win32_edge_engine
line 2890 : using browser_engine = detail::win32_edge_engine;
*/
void webview_wrapper::create(bool debug, void *wnd)
{
  if (w != nullptr)
    return;
  w = new webview::webview(debug, wnd);
  bind_doc("webapp_help", "return a help message.", [&](const std::string &req) -> std::string {
    auto arg1 = json_parse(req, "", 0);
    std::string res, s = "";

    if (arg1 == "json")
    {
      for (auto fh : func_help)
      {
        s += "\"" + fh.first + "\":\"" + fh.second + "\",";
      }

      // Remove last comma
      s.pop_back();
      res = "{" + s + "}";
    }
    else
    {
      for (auto fh : func_help)
      {
        s += fh.first + ':' + fh.second + '\n';
      }
      res = '"' + to_htent(s) + '"';
    }

    // std::cout << res << std::endl;
    return res;
  });
}


webview_wrapper::webview_wrapper(bool debug, void *wnd)
{
  create(debug, wnd);
}


webview_wrapper::webview_wrapper()
{
}


webview_wrapper::~webview_wrapper()
{
  delete WP;
}


void webview_wrapper::navigate(const std::string &url)
{
  WP->navigate(url);
}

void webview_wrapper::bind(const std::string &name, sync_binding_t fn)
{
  WP->bind(name, fn);
}

void webview_wrapper::bind(const std::string &name, binding_t fn, void *arg)
{
  WP->bind(name, fn, arg);
}

void webview_wrapper::unbind(const std::string &name)
{
  WP->unbind(name);
}

void webview_wrapper::resolve(const std::string &seq, int status, const std::string &result)
{
  WP->resolve(seq, status, result);
}

void webview_wrapper::run()
{
  WP->run();
}

void *webview_wrapper::window()
{
  return WP->window();
}

void webview_wrapper::terminate()
{
  WP->terminate();
}

void webview_wrapper::dispatch(std::function<void()> f)
{
  WP->dispatch(f);
}

void webview_wrapper::set_title(const std::string &title)
{
  WP->set_title(title);
}

void webview_wrapper::minimize()
{
#ifdef _WIN32
  ShowWindow((HWND)WP->window(), SW_SHOWMINIMIZED);
#endif
}

void webview_wrapper::restore()
{
#ifdef _WIN32
  ShowWindow((HWND)WP->window(), SW_RESTORE);
#endif
}

void webview_wrapper::set_pos(int x, int y)
{
#ifdef _WIN32
  HWND hw=(HWND)WP->window();
  SetWindowPos(hw, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
#endif
}

void webview_wrapper::set_size(int width, int height, int hints)
{
  //std::cout << "set_size w " << width << ", h " << height << ", hints " << hints << std::endl;
  if (hints < 0) {
#ifdef _WIN32
  HWND hw=(HWND)WP->window();
  SetWindowPos(hw, NULL, 0, 0, width, height, SWP_NOREPOSITION | SWP_NOZORDER);
#else
  hints=0;
#endif
  }

  WP->set_size(width, height, hints);
}

void webview_wrapper::set_hints(int hints)
{
  if (hints > -1 && hints < 4) {
#ifdef _WIN32
    RECT rc;
    HWND hw=(HWND)WP->window();
    GetWindowRect(hw, &rc);
    set_size(rc.right-rc.left, rc.bottom-rc.top, hints);
#endif
  }
}

void webview_wrapper::set_html(const std::string &html)
{
  WP->set_html(html);
}

void webview_wrapper::init(const std::string &js)
{
  WP->init(js);
}

void webview_wrapper::eval(const std::string &js)
{
  WP->eval(js);
}

std::string json_parse(const std::string &s, const std::string &key, const int index)
{
  return webview::detail::json_parse(s, key, index);
}


void webview_wrapper::bind_doc(const std::string &name, const std::string desc, sync_binding_t fn, bool indoc)
{
  WP->bind(name, fn);
  if (indoc)
    func_help.push_back(make_pair(name, desc));
}


void webview_wrapper::bind_doc(const std::string &name, const std::string desc, binding_t fn, void *arg, bool indoc)
{
  WP->bind(name, fn, arg);
  if (indoc)
    func_help.push_back(make_pair(name, desc));
}


void webview_wrapper::bind_noprom(const std::string &name, sync_binding_t fn)
{
  WP->bind(name, fn);
}


std::string webview_wrapper::version()
{
  return std::string(webview_version()->version_number) + std::string(webview_version()->pre_release) +
         std::string(webview_version()->build_metadata);
}

