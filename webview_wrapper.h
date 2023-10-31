
#ifndef WEBVIEW_WRAPPER_H
#define WEBVIEW_WRAPPER_H

#include <string>
#include <functional>

using binding_t = std::function<void(std::string, std::string, void *)>;
using sync_binding_t = std::function<std::string(std::string)>;
std::string json_parse(const std::string &s, const std::string &key, const int index);

class webview_wrapper {
public:
  webview_wrapper(bool debug = false, void *wnd = nullptr);
  void navigate(const std::string &url);
  void bind(const std::string &name, sync_binding_t fn);
  void bind(const std::string &name, binding_t fn, void *arg);
  void run();
  void unbind(const std::string &name);
  void resolve(const std::string &seq, int status, const std::string &result);
  void *window();
  void terminate();
  void dispatch(std::function<void()>);
  void set_title(const std::string &);
  void set_size(int, int, int);
  void set_html(const std::string &);
  void init(const std::string &);
  void eval(const std::string &);
private :
  void *w;
};

#endif /* WEBVIEW_WRAPPER_H */

