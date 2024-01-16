
#ifndef WEBVIEW_WRAPPER_H
#define WEBVIEW_WRAPPER_H

#include <functional>
#include <string>

std::wstring my_widen_string(const std::string &input);
std::string my_narrow_string(const std::wstring &input);

using binding_t = std::function<void(std::string, std::string, void *)>;
using sync_binding_t = std::function<std::string(std::string)>;
using pair_of_string = std::pair<std::string, std::string>;

std::string json_parse(const std::string &s, const std::string &key, const int index);
std::string json_escape(const std::string &s, bool add_quotes = true);
int json_unescape(const char *s, size_t n, char *out);
constexpr bool is_json_special_char(unsigned int c);
constexpr bool is_control_char(unsigned int c);

class webview_wrapper
{
public:
  webview_wrapper();
  webview_wrapper(bool debug, void *wnd = nullptr);
  void create(bool debug = false, void *wnd = nullptr);
  ~webview_wrapper();
  void bind(const std::string &name, sync_binding_t fn);
  void bind(const std::string &name, binding_t fn, void *arg);

  void bind_doc(const std::string &name, const std::string desc, sync_binding_t fn, bool indoc = true);
  void bind_noprom(const std::string &name, sync_binding_t fn);

  void bind_doc(const std::string &name, const std::string desc, binding_t fn, void *arg = nullptr, bool indoc = true);

  void resolve(const std::string &seq, int status, const std::string &result);

  void navigate(const std::string &url);
  void run();
  void unbind(const std::string &name);
  void *window();
  void terminate();
  void dispatch(std::function<void()>);

  void restore();
  void minimize();
  void set_title(const std::string &);
  void set_pos(int, int);

  void set_hints(int);
#ifdef _WIN32
  void set_size(int, int, int hints=-1);
#else
  void set_size(int, int, int hints=0);
#endif

  void set_html(const std::string &);
  void init(const std::string &);
  void eval(const std::string &);
  std::string version();

private:
  void *w = nullptr;
  std::vector<pair_of_string> func_help = {};
};

#endif /* WEBVIEW_WRAPPER_H */
