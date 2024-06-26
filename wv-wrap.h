
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


struct webview_conf {
  bool debug, status, zoom, ctx_menu, psw_sav, auto_fill;

};

class webview_wrapper
{
private:
  static webview_wrapper *me;

  std::string on_move_func="", on_exit_func="";
  void *w = nullptr;
  std::vector<pair_of_string> func_help = {};

public:
  static webview_conf conf;

  void out_conf();
  webview_wrapper();
  webview_wrapper(bool debug, void *wnd = nullptr);
  void create(void *wnd = nullptr);
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

  void hide();
  void restore();
  void minimize();
  void set_title(const std::string &);
  void set_icon(const std::string);
  void set_pos(int, int);
  void get_pos(int&, int&);

  void set_hints(int);
  void get_size(int&, int&);
#ifdef _WIN32
  // Global variable.
  HWINEVENTHOOK g_hook;
  static void CALLBACK HandleWinEvent(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD);
  void InitSpy();
  void ExitSpy();
  static LRESULT myWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
  void set_size(int, int, int hints=-1);
#else
  void set_size(int, int, int hints=0);
#endif

  void set_on_exit(const std::string);
  void set_on_move(const std::string);
  void set_html(const std::string &);
  void init(const std::string &);
  void eval(const std::string &);
  std::string version();
};

#endif /* WEBVIEW_WRAPPER_H */
