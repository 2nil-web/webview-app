
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

#ifdef _WIN32
#include "wv-winapi.h"
#ifdef WITH_REGISTRY
#include "wv-reg.h"
#endif
#endif

#ifdef WITH_CURL
#include "wv-curl.h"
#endif

#include "wv-opts.h"
#include "wv-util.h"
#include "wv-wrap.h"
#include "wv-binds.h"

void write_cons(webview_wrapper& w, std::string s, std::ostream &out = std::cout)
{
  if (s.empty())
    return;
#ifdef _WIN32
  char title[256];
  std::string tit = "";
  int gctres = GetConsoleTitle(title, 255);
  // out << gctres << std::endl;
  if (gctres > 0)
  {
    tit = title;
    if (tit.find("invisible cygwin console") != std::string::npos)
    {
      out << s;
      out.flush();
    }
    else
    {
      HANDLE ho;
      if (&out == &std::cerr)
        ho = GetStdHandle(STD_ERROR_HANDLE);
      else
        ho = GetStdHandle(STD_OUTPUT_HANDLE);
      WriteConsole(ho, s.c_str(), (DWORD)s.size(), nullptr, nullptr);
    }
  }
  else
    out << s;
  out.flush();
#else
  out << s;
  out.flush();
#endif

  w.eval("console.log('[[" + s + "]]');");
}

std::string setfile(std::filesystem::path path)
{
  // std::cout << path << std::endl;
  std::string eps;
#ifdef _WIN32
  eps = to_htent(path.wstring());
#else
  eps = to_htent(path.string());
#endif
  replace_all(eps, "\\", "/");
  eps = "\"path\":\"" + eps + '"';
  return "{" + eps + "},";
}

// Return a javascript array of strings corresponding to a directory list,
std::string lsdir(std::string spath, bool recursive = false)
{
  std::string res = "", res_line;
  std::filesystem::path path;

  if (spath.empty())
    path = ".";
  else
    path = spath;

  if (std::filesystem::is_directory(path))
  {
    // Pas de récursif, trop dangereux, ça plante le PC ...
    for (const auto &e : std::filesystem::directory_iterator(path))
    {
      res += setfile(e);
    }
  }
  else
  {
    if (std::filesystem::is_regular_file(path))
    {
      res += setfile(path.filename());
    }
    else
      res += setfile("not a directory or file '" + path.filename().string() + "'");
  }

  // Remove last comma
  res.pop_back();

  res = "{\"result\":[" + res + "]}";
  return res;
}

// Return a string that represents the number in octal form, useful for a command like chmod
std::string to_js_oct(unsigned int number)
{
  std::ostringstream str;
  str << std::oct << number;
  return "0" + str.str();
}

void print_file_types()
{
  static int un = true;
  if (un)
  {
    un = false;
    std::cout << "none      :" << (int)std::filesystem::file_type::none << std::endl;
    std::cout << "not_found :" << (int)std::filesystem::file_type::not_found << std::endl;
    std::cout << "regular   :" << (int)std::filesystem::file_type::regular << std::endl;
    std::cout << "directory :" << (int)std::filesystem::file_type::directory << std::endl;
    std::cout << "symlink   :" << (int)std::filesystem::file_type::symlink << std::endl;
    std::cout << "block     :" << (int)std::filesystem::file_type::block << std::endl;
    std::cout << "character :" << (int)std::filesystem::file_type::character << std::endl;
    std::cout << "fifo      :" << (int)std::filesystem::file_type::fifo << std::endl;
    std::cout << "socket    :" << (int)std::filesystem::file_type::socket << std::endl;
    std::cout << "unknown   :" << (int)std::filesystem::file_type::unknown << std::endl;
    std::cout << std::flush;
  }
}
// As file_type have unspecified values in the C++ standard
// and as I notice a difference between G++ and MSVC
// let force there value to my own choice for javascript
int forced_file_type(std::filesystem::file_type ft)
{
  switch (ft)
  {
  case std::filesystem::file_type::not_found:
    return 0;
  case std::filesystem::file_type::none:
    return 1;
  case std::filesystem::file_type::regular:
    return 2;
  case std::filesystem::file_type::directory:
    return 3;
  case std::filesystem::file_type::symlink:
    return 4;
  case std::filesystem::file_type::block:
    return 5;
  case std::filesystem::file_type::character:
    return 6;
  case std::filesystem::file_type::fifo:
    return 7;
  case std::filesystem::file_type::socket:
    return 8;
  case std::filesystem::file_type::unknown:
  default:
    return 9;
  }
}

template <typename TP> std::time_t to_time_t(TP tp)
{
  namespace ch = std::chrono;
  auto sctp = ch::time_point_cast<ch::system_clock::duration>(tp - TP::clock::now() + ch::system_clock::now());
  return ch::system_clock::to_time_t(sctp);
}

#ifdef _WIN32
#define my_gmtime(a, b) gmtime_s(b, a)
#define my_localtime(a, b) localtime_s(b, a)
#else
#define my_gmtime(a, b) gmtime_r(a, b)
#define my_localtime(a, b) localtime_r(a, b)
#endif

// Convert a file time to a string, default format is ISO8601 and default time
// zone is local
std::string file_time_to_string(std::filesystem::file_time_type file_time, std::string fmt = "%Y-%m-%d %H:%M:%S",
                                bool gm = false)
{
  std::time_t tt = to_time_t(file_time);
  std::tm tim;
  if (gm)
    my_gmtime(&tt, &tim);
  else
    my_localtime(&tt, &tim);
  std::stringstream buffer;
  buffer << std::put_time(&tim, fmt.c_str());
  std::string fmtime = buffer.str();

  return fmtime;
}

std::string lastwrite(std::filesystem::path p)
{
  return file_time_to_string(std::filesystem::last_write_time(p));
}

bool isBase64(const std::string s)
{
  if (s.length() % 4 == 0)
    return std::regex_match(s, std::regex("^[A-Za-z0-9+/]*={0,2}$"));
  return false;
}

void fwrite(std::string fname, std::string s, std::ios_base::openmode omod = std::ios::app)
{
  std::ofstream f(fname, omod);
  f << s;
  f.close();
}

std::string do_fstat(std::string sp)
{
  std::string s;
  from_htent(sp, s);
  replace_all(s, "\\", "/");
#ifdef _WIN32 // ou _MSVC ?
  std::wstring ws;
  from_htent(sp, ws);
  replace_all(ws, L"\\", L"/");
#else
  std::string ws = s;
#endif
  std::filesystem::path p(ws);
  sp = to_htent(ws);
  auto fs = std::filesystem::status(p);
  auto ft = fs.type();
  std::uintmax_t sz;
  if (ft == std::filesystem::file_type::regular)
    sz = std::filesystem::file_size(p);
  else
    sz = static_cast<std::uintmax_t>(-1);
  std::string lastwr = "****-**-**T**:**:**";
  if (ft != std::filesystem::file_type::not_found)
    lastwr = lastwrite(p);
  else
    std::cout << "NOT FOUND " << sp << '(' << p << ')' << std::endl;

  unsigned int perms = (unsigned)fs.permissions();
  // std::cout << "PERM " << std::oct << perms << ", OCT PERM " << to_js_oct(perms) << std::endl;
  std::string res = "{\"file\":\"" + sp + "\"," + "\"type\":\"" + std::to_string(forced_file_type(ft)) + "\"," +
                    "\"perms\":\"" + to_js_oct(perms) + "\"," + "\"size\":\"" + std::to_string(sz) + "\"," +
                    "\"last_write\":\"" + lastwr + "\"}";
  // std::cout << res << std::endl;
  return res;
}

// console.log(webapp_help())
static unsigned int nfs = 0;
void create_binds(webview_wrapper& w)
{
#ifdef WITH_CURL
  w.bind_doc("libcurl_ver", "Return a string indicating the libcurl version.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 std::string libcurlver = LIBCURL_VERSION;
                 std::cout << libcurlver << std::endl;
                 w.resolve(seq, 0, '"' + libcurlver + '"');
               }).detach();
             });

  // Set http credential
  w.bind_doc("wiki", "run an http get against the wiki.", [&](const std::string &seq, const std::string &req, void *) {
    std::thread([&, seq, req] {
      wiki_curl();
      w.resolve(seq, 0, "");
    }).detach();
  });


  w.bind_doc("httpget", "run an http get with the provided url.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 std::string url = json_parse(req, "", 0);
                 auto res = httpget(url);
                 // std::cout << "httpget " << url << ":\n" << res << std::endl;
                 replace_all(res, "\\", "");
                 res = to_htent(res);
                 w.resolve(seq, 0, res);
                 std::cout << res << std::endl << std::flush;
               }).detach();
             });

  // Set http credential
  w.bind_doc("httpcred", "run an http get with the provided credential (id/password) and url.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 auto id = json_parse(req, "", 0);
                 auto pass = json_parse(req, "", 1);
                 auto url = json_parse(req, "", 2);
                 auto ret = httpget_cred(id, pass, url);
                 std::cout << ret << std::endl;
                 w.resolve(seq, 0, ret);
               }).detach();
             });
#endif
#ifdef _WIN32
  w.bind_doc("msgbox", "Display a dialog in Windows MessageBox style.", [&](const std::string &req) -> std::string {
     std::string msg = json_parse(req, "", 0);
     std::string tit = json_parse(req, "", 1);
     int but = std::stoi(json_parse(req, "", 2));
     int ret=MessageBox((HWND)(w.window()), msg.c_str(), tit.c_str(), but);
     return std::to_string(ret);
 });

  w.bind_doc("webapp_show", "Hide the webapp window.", [&](const std::string &req) -> std::string {
    w.show();
    return "";
 });

  w.bind_doc("webapp_hide", "Hide the webapp window.", [&](const std::string &req) -> std::string {
    w.hide();
    return "";
 });

  // Run a local command and return an eventual result at a later time.
  w.bind_doc("webapp_shell", "open an external windows command (See ShellExecute).", [&](const std::string &seq, const std::string &req, void *) {
    std::thread([&, seq, req] {
      std::string cmd, param, dir, ope;
      cmd = json_parse(req, "", 0);

      if ((param=json_parse(req, "", 1)) != "") {
        if ((dir=json_parse(req, "", 2)) != "") {
          ope=json_parse(req, "", 3);
        }
      }

      std::cout << "cmd: " << cmd << std::endl;
      std::string res_cmd = shell_cmd(cmd, param, dir, ope);
      std::cout << "res_cmd: " << res_cmd << std::endl;
      rep_bs(res_cmd);
      auto result = "{\"value\": \"" + res_cmd + "\"}";
      std::cout << "result: " << result << std::endl;
      w.resolve(seq, 0, result);
    }).detach();
  });


#ifdef WITH_REGISTRY
// Windows registry storage
  w.bind_doc("StoReg", "Store a string to the Windows registry.", [&](const std::string &req) -> std::string {
    std::string key = json_parse(req, "", 0);
    std::string var = json_parse(req, "", 1);
    std::string val = json_parse(req, "", 2);
    PutRegString(key, var, val);
    return "";
 });

  w.bind_doc("GetReg", "Retrieve a string from the Windows registry.",
    [&](const std::string &seq, const std::string &req, void *) {
      std::thread([&, seq, req] {
        std::string key = json_parse(req, "", 0);
        std::string var = json_parse(req, "", 1);
        std::string defval = json_parse(req, "", 2);
        std::cout << "GetReg key [" << key << "], var [" << var << "], defval [" << defval << ']' << std::endl;
        auto val=GetRegString(key, var, defval);
        std::cout << "GetReg key [" << key << "], var [" << var << "], val [" << val << ']' << std::endl;
        w.resolve(seq, 0, '"' + val + '"');
   }).detach();
 });
#endif
#endif
  
  w.bind_doc("webview_ver", "Return a string indicating the webview version.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 std::string webviewver = w.version();
                 std::cout << webviewver << std::endl;
                 w.resolve(seq, 0, '"' + webviewver + '"');
               }).detach();
             });

  w.bind_doc("app_info", "Return a string indicating this application information.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 std::string appinfo = appInfo();
                 std::cout << appinfo << std::endl;
                 w.resolve(seq, 0, '"' + appinfo + '"');
               }).detach();
             });

  w.bind_doc("echo", "echo the parameter.", [&](const std::string &seq, const std::string &req, void *) {
    std::thread([&, seq, req] {
      auto s = json_parse(req, "", 0);
      std::cout << s << std::endl;
      w.resolve(seq, 0, '"' + s + '"');
    }).detach();
  });

  w.bind_doc("hecho", "echo the parameter decoding from and encoding to html entities.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 auto sp = json_parse(req, "", 0);
                 std::string htent = to_htent(sp);
#ifdef _WIN32
                 SetConsoleOutputCP(CP_UTF8);
#endif
                 //          std::cout << "param " << sp << std::endl;
                 //          std::cout << "to_htent " << htent << std::endl;
                 std::string ht_to;
                 from_htent(htent, ht_to);
                 std::cout << "from_htent " << ht_to << std::endl;
                 w.resolve(seq, 0, '"' + htent + '"');
               }).detach();
             });

  w.bind_doc("fstat", "gives information details on a file.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 auto sp = json_parse(req, "", 0);
                 auto s_stat = do_fstat(sp);
                 std::cout << s_stat << std::endl;
                 w.resolve(seq, 0, s_stat);
               }).detach();
             });

  w.bind_doc("fread", "read file with provided file name and return its content.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 auto filename = json_parse(req, "", 0);
                 w.resolve(seq, 0, '"' + fread(filename) + '"');
               }).detach();
             });

  w.bind_doc("fwrite", "truncate and write to file with provided file name.", [&](const std::string &req) -> std::string {
    std::string fn, s;
    int n = 0;
    fn = json_parse(req, "", n++);
    for (;;)
    {
      s = json_parse(req, "", n++);
      if (s.empty())
        break;
      if (n == 2)
      {
        fwrite(fn, s, std::ios::out);
      }
      else
      {
        fwrite(fn, "\n");
        fwrite(fn, s);
      }
    }
    return "";
  });

  w.bind_doc("fappend", "append to file with provided file name.", [&](const std::string &req) -> std::string {
    std::string fn, s;
    int n = 0;
    fn = json_parse(req, "", n++);
    for (;;)
    {
      s = json_parse(req, "", n++);
      if (s.empty())
        break;
      if (n == 2)
      {
        fwrite(fn, s);
      }
      else
      {
        fwrite(fn, "\n");
        fwrite(fn, s);
      }
    }
    return "";
  });

  w.bind_doc("absolute", "gives absolute path of a file path.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 auto pth = json_parse(req, "", 0);
                 auto res = std::filesystem::absolute(pth).string();
                 replace_all(res, "\\", "/");
                 w.resolve(seq, 0, '"' + res + '"');
               }).detach();
             });

  w.bind_doc("getenv", "Return value of environment variable.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 auto var = json_parse(req, "", 0);
                 std::string res = my_getenv(var);
                 replace_all(res, "\\", "/");
                 w.resolve(seq, 0, '"' + res + '"');
               }).detach();
             });

  w.bind_doc("setenv", "Set environment variable to given value.", [&](const std::string &req) -> std::string {
     std::string var = json_parse(req, "", 0);
     std::string val = json_parse(req, "", 1);
     std::cout << var << '=' << val << std::endl;

     if (var != "") {
       std::string expr=var+"="+val;
       if (my_setenv(var, val)) return "true";
     }

     return "false";
 });

#ifdef _WIN32
#define PATHSEP ';'
#else
#define PATHSEP ':'
#endif

  w.bind_doc("addpth", "Add program path to the PATH env variable if is not yet added.",
             [&](const std::string &req) -> std::string {
               std::string pth = my_getenv("PATH");
               std::string currpth = std::filesystem::current_path().string();
               auto vpth = split(pth, PATHSEP);
               bool do_add = true;
               for (std::string p : vpth)
               {
                 if (p == currpth)
                   do_add = false;
               }

               if (do_add)
               {
                 // pth=(std::string("PATH=")+pth+':'+std::filesystem::current_path().string());
                 //pth = "PATH=" + pth + PATHSEP + std::filesystem::current_path().string();
                 my_setenv("PATH", pth + PATHSEP + std::filesystem::current_path().string());
               }
               return "";
             });

  w.bind_doc("chdir", "change current directory.", [&](const std::string &req) -> std::string {
    nfs = 0;
    auto pth = json_parse(req, "", 0);
    std::filesystem::current_path(pth);
    return "";
  });

  w.bind_doc("ls_attach", "list provided directory in foreground.", [&](const std::string &req) {
    auto param = json_parse(req, "", 0);
    auto res = lsdir(param);
    // std::cout << res << std::endl;
    return res;
  });

  w.bind_doc("ls", "list provided directory in background",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 auto param = json_parse(req, "", 0);
                 auto res = lsdir(param);
                 // std::wcout << res << std::endl;
                 w.resolve(seq, 0, res);
               }).detach();
             });

  w.bind_doc("lsr",
             "list provided directory recursively in background ==> very dangerous "
             "may hangs the machine !",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] { w.resolve(seq, 0, lsdir(json_parse(req, "", 0), true)); }).detach();
             });

  w.bind_doc("webapp_get_title", "return webapp window title in foreground.",
             [&](const std::string &req) -> std::string {
               std::string prev_title = "";
#ifdef _WIN32
               prev_title = GetWindowText((HWND)w.window());
#endif
               // return "{\"value\": \"" + prev_title + "\"}";
               return '"' + prev_title + '"';
             });

  w.bind_doc("webapp_get_title_bg", "return webapp window title in background.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 std::string prev_title = "";
#ifdef _WIN32
                 prev_title = GetWindowText((HWND)w.window());
#endif
                 auto result = "{\"value\": \"" + prev_title + "\"}";
                 w.resolve(seq, 0, result);
               }).detach();
             });

  // Change window title
  w.bind_doc("webapp_title", "change webapp window title.",
             [&](const std::string &seq, const std::string &req, void *) {
               std::thread([&, seq, req] {
                 std::string prev_title = "";
#ifdef _WIN32
                 prev_title = GetWindowText((HWND)w.window());
#endif
                 auto title = json_parse(req, "", 0);
                 w.set_title(title);
                 auto result = "{\"value\": \"" + prev_title + "\"}";
                 w.resolve(seq, 0, result);
               }).detach();
             });

  // Change window position
  w.bind_doc("webapp_pos", "set window position.", [&](const std::string &req) -> std::string {
    auto l_x = std::stoi(json_parse(req, "", 0));
    auto l_y = std::stoi(json_parse(req, "", 1));
    std::cout << "x " << l_x << ", y " << l_y << std::endl;
    w.set_pos(l_x, l_y);
    return "";
  });

  // Change window dimension and sizing behaviour
  w.bind_doc("webapp_size", "set dimension and hint of webapp window.", [&](const std::string &req) -> std::string {
//    auto params = json_parse(req, "", 0);
    auto l_width = std::stoi(json_parse(req, "", 0));
    auto l_height = std::stoi(json_parse(req, "", 1));
    auto l_hints = std::stoi(json_parse(req, "", 2));
    //std::cout << "width " << l_width << ", height " << l_height << std::endl;
    w.set_size(l_width, l_height, l_hints);
    /*
      0 Width and height are default size
      1 Width and height are minimum bounds
      2 Width and height are maximum bounds
      3 Window size is fixed
    */
    return "";
  });

  // Exit from the web application
  w.bind_doc("webapp_exit", "exit from webapp.", [&](const std::string &) -> std::string {
    w.terminate();
    return "";
  });

  // Run a local command and return an eventual result at a later time.
  w.bind_doc("webapp_exec", "run an external command.", [&](const std::string &seq, const std::string &req, void *) {
    std::thread([&, seq, req] {
      auto cmd = json_parse(req, "", 0);
      std::cout << "cmd: " << cmd << std::endl;
      std::string res_cmd = exec_cmd(cmd);
      std::cout << "res_cmd: " << res_cmd << std::endl;
      rep_bs(res_cmd);
      auto result = "{\"value\": \"" + res_cmd + "\"}";
      std::cout << "result: " << result << std::endl;
      w.resolve(seq, 0, result);
    }).detach();
  });

  w.bind_doc("write", "write a string to stdout.", [&](const std::string &req) -> std::string {
    auto s = json_parse(req, "", 0);
    write_cons(w, s);
    return "";
  });

  w.bind_doc("writeln", "write a string to stdout and add a carriage return.",
             [&](const std::string &req) -> std::string {
               auto s = json_parse(req, "", 0);
               write_cons(w, s);
               write_cons(w, "\n");
               return "";
             });

  w.bind_doc("ewrite", "write a string to stderr.", [&](const std::string &req) -> std::string {
    auto s = json_parse(req, "", 0);
    write_cons(w, s, std::cerr);
    return "";
  });

  w.bind_doc("ewriteln", "write a string to stderr and add a carriage return.",
             [&](const std::string &req) -> std::string {
               auto s = json_parse(req, "", 0);
               write_cons(w, s, std::cerr);
               write_cons(w, "\n", std::cerr);
               return "";
             });
}

