
#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <regex>

#include <webview.h>

#include "wv-util.h"
#include "base64.hpp"
#include "wv-winapi.h"
#include "wv-curl.h"
#include "Utf8Conv.hpp"
using Utf8Conv::Utf16ToUtf8;
using Utf8Conv::Utf8ToUtf16;

webview::webview *w=nullptr;


// Convert a wstring to a string containing a suite of hexa numbers, separated by space and representing the utf code of each characters of the wstring
// i.e. the wstring "totö要らない" will result in a string "74 6f 74 f6 8981 3089 306a 3044"
std::string s_w2h(std::wstring ws) {
  std::stringstream cnv;
  for (auto e:ws) {
    //if (cnv.tellp() != 0)
    if (!cnv.str().empty())
      cnv << ' ';
    cnv << std::hex << (unsigned int)e;
  }

  return cnv.str();
}



// Convert a string containing a suite of hexa numbers, separated by space and representing utf code to a wstring
// i.e. the string "74 6f 74 f6 8981 3089 306a 3044"  will give the wstring "totö要らない" 
std::wstring s_h2w(std::string hs) {
  std::string hex_chars(hs);

  std::istringstream hex_chars_stream(hex_chars);
  std::wstring ws=L"";
  unsigned int c;
  while (hex_chars_stream >> std::hex >> c) {
    ws+= (wchar_t)c;
  }

  return ws;
}

void write_cons(std::string s, std::ostream& out=std::cout) {
  if (s.empty()) return;
#ifdef _WIN32
  char title[256];
  std::string tit="";
  int gctres=GetConsoleTitle(title, 255);
  //out << gctres << std::endl;
  if (gctres > 0) {
    tit=title;
    if (tit.find("invisible cygwin console") != std::string::npos) {
      out << s; out.flush();
    } else {
      HANDLE ho;
      if (&out == &std::cerr) ho=GetStdHandle(STD_ERROR_HANDLE);
      else ho=GetStdHandle(STD_OUTPUT_HANDLE);
      WriteConsole(ho, s.c_str(), (DWORD)s.size(), nullptr, nullptr);
    }
  } else out << s; out.flush();
#else
    out << s; out.flush();
#endif

  if (w != nullptr) w->eval("console.log('[["+s+"]]');");
  else MessageBox(NULL, "", "No Win", MB_OK);
}

bool isWideString(const std::string s) {    
  for (auto& c : s) { 
    //if(c & 0x80) return true;
    if(c > 0xff) return true;
  }

  return false;
}

bool isWideString(const std::wstring s) {    
  for (auto& c : s) { 
    //if(c & 0x80) return true;
    if(c > 0xff) return true;
  }

  return false;
}

std::string skipWideChars(std::string ws) {
  std::string s;
  for(auto& c : ws) { 
    //if (c & 0x80) s+='_';
    if (c > 0xff) s+='_';
    else s+=(char)c;
  }

  return s;
}

std::string skipWideChars(std::wstring ws) {
  std::string s;
  for(auto& c : ws) { 
    //if(c & 0x80) s+='*';
    if (c > 0xff) s+='_';
    else s+=(char)c;
  }

  return s;
}

const std::string hexa_pfx="UTF_IN_HEXA_STRING";
// Return a javascript array of strings corresponding to a directory list, recursively or not
std::string lsdir(std::string path, bool recursive=false) {
  std::string res, res_line;
  if (path.empty()) path=".";
  // Pas de récursif, trop dangereux, ça plante le PC ...
  for (const auto& e:std::filesystem::directory_iterator(path)) {
    // Will return also a path_hexa field if path is not only made of ascii characters in order to workaround the utf8 Window$ shitty processing
    std::string eps;
#ifdef _MSC_VER
    eps = ws2s(e.path().wstring());
#else
    eps = e.path().string();
#endif

    replace_all(eps, "\\", "/");
    res_line="\"path\":\""+eps+'"';

    if (isWideString(eps)) {
      std::string hs=hexa_pfx+s_w2h(e.path().wstring());
      res_line+=",\"path_hexa\":\""+hs+'"';
    }

    //std::cout << res_line << std::endl;
    res+="{"+res_line+"},";
  }

  // Remove last comma
  res.pop_back();

  res="{\"result\":["+res+"]}";
  return res;
}

// Return a string that represents the number in hexadecimal form
std::string to_js_hex(unsigned int number) {
   std::ostringstream str;
   str << std::hex << number;
   return "0X"+str.str();
}

// Return a string that represents the number in octal form
std::string to_js_oct(unsigned int number) {
   std::ostringstream str;
   str << std::oct << number;
   return "0"+str.str();
}

void print_file_types() {
  static int un=true;
  if (un) {
    un=false;
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
int forced_file_type(std::filesystem::file_type ft) {
  switch (ft) {
    case std::filesystem::file_type::not_found : return 0;
    case std::filesystem::file_type::none      : return 1;
    case std::filesystem::file_type::regular   : return 2;
    case std::filesystem::file_type::directory : return 3;
    case std::filesystem::file_type::symlink   : return 4;
    case std::filesystem::file_type::block     : return 5;
    case std::filesystem::file_type::character : return 6;
    case std::filesystem::file_type::fifo      : return 7;
    case std::filesystem::file_type::socket    : return 8;
    case std::filesystem::file_type::unknown   :
    default :                                    return 9;
  }
}

std::string sec_wait(std::string ssec) {
  auto msg=ssec+" second wait";
  int sec = std::stoi(ssec);
//  std::cout << "Starting " << msg << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(sec));
//  std::cout << "Ending   " << msg << std::endl;
  return '"'+msg+" is over.\"";
}

template <typename TP>
std::time_t to_time_t(TP tp) {
  namespace ch=std::chrono;
  auto sctp=ch::time_point_cast<ch::system_clock::duration>(tp-TP::clock::now()+ch::system_clock::now());
  return ch::system_clock::to_time_t(sctp);
}

#ifdef _WIN32
#define my_gmtime(a,b) gmtime_s(b,a)
#define my_localtime(a,b) localtime_s(b,a)
#else
#define my_gmtime(a,b) std::gmtime_r(a,b)
#define my_localtime(a,b) std::localtime_r(a,b)
#endif

// Convert a file time to a string, default format is ISO8601 and default time zone is local
std::string file_time_to_string(std::filesystem::file_time_type file_time, std::string fmt="%Y-%m-%d %H:%M:%S", bool gm=false) {
  std::time_t tt=to_time_t(file_time);
  std::tm tim;
  if (gm) my_gmtime(&tt, &tim);
  else my_localtime(&tt, &tim);
  std::stringstream buffer;
  buffer << std::put_time(&tim, fmt.c_str());
  std::string fmtime=buffer.str();

  return fmtime;
}

std::string lastwrite(std::filesystem::path p) {
  return file_time_to_string(std::filesystem::last_write_time(p));
}


bool isBase64(const std::string s) {
  if (s.length()%4 == 0) return std::regex_match(s, std::regex("^[A-Za-z0-9+/]*={0,2}$"));
  return false;
}


void fwrite(std::string fname, std::string s, std::ios_base::openmode omod=std::ios::app) {
  std::ofstream f(fname, omod);
  f << s;
  f.close();
}

std::string do_fstat(std::string sp) {
  std::filesystem::path p;
//  std::cout << "deb fstat " << sp << std::endl;
  if (sp.starts_with(hexa_pfx)) {
    auto sp2=sp.substr(hexa_pfx.size());
    replace_all(sp2, "\\", "/");
    p=s_h2w(sp2);
#ifdef _MSC_VER
    sp=Utf16ToUtf8(p.wstring());
    //if (sp != p.string()) std::cout << "HEX " << sp << "\n  # " << p << std::endl;
#else
    sp=skipWideChars(p.wstring());
    //sp=p.string();
#endif
  } else {
    p=sp;
  }
  replace_all(sp, "\\", "/");
//  std::cout << "mid fstat " << p << std::endl;

  auto fs=std::filesystem::status(p);
  auto ft=fs.type();
  std::uintmax_t sz;
  if (ft == std::filesystem::file_type::regular) sz=std::filesystem::file_size(p);
  else sz=static_cast<std::uintmax_t>(-1);
  std::string lastwr="****-**-**T**:**:**";
  if (ft != std::filesystem::file_type::not_found) lastwr=lastwrite(p);
  else {
    std::cout << "NOT FOUND " << sp << std::endl;
  }
  std::string res ="{\"file\":\""       + sp+"\","+
                    "\"type\":\""       + std::to_string(forced_file_type(ft))+"\","+
                    "\"perms\":\""      + to_js_oct((unsigned)fs.permissions())+"\","+
                    "\"size\":\""       + std::to_string(sz)+"\","+
                    "\"last_write\":\"" + lastwr+"\"}";
  std::cout << "end fstat:" << res << std::endl << std::flush;
  return res;
}

using pair_of_string = std::pair<std::string, std::string>;
std::vector <pair_of_string> func_help;

void add_to_doc(const std::string name, const std::string desc) {
  func_help.push_back(make_pair(name, desc));
}


// Synchronous bind
void bind_doc(webview::webview &w, const std::string &name, const std::string desc, webview::webview::sync_binding_t fn, bool indoc=true) {
  //std::cout << "sync bind " << name << std::endl;
  w.bind(name, fn);
  if (indoc) add_to_doc(name, desc);
}

// Asynchronous bind
void bind_doc(webview::webview &w, const std::string &name, const std::string desc,  webview::webview::binding_t fn, void *arg=nullptr, bool indoc=true) {
  //std::cout << "async bind " << name << std::endl;
  w.bind(name, fn, arg);
  if (indoc) add_to_doc(name, desc);
}

// console.log(webapp_help())
static unsigned int nfs=0;
void create_binds(webview::webview &w) {
  bind_doc(w, "webapp_help", "return a help message.", [&](const std::string & req) -> std::string {
    auto arg1=webview::detail::json_parse(req, "", 0);
    std::string res, s="";

    if (arg1 == "tab") {
      for (auto fh:func_help) {
        s+="\""+fh.first+"\":\""+fh.second+"\",";
      }

      // Remove last comma
      s.pop_back();
      res="{"+s+"}";
    } else {
      for (auto fh:func_help) {
        s+=fh.first+':'+fh.second;
      }
      res='"'+s+'"';
    }

    //std::cout << res << std::endl;
    return res;
  });

  bind_doc(w, 
    "fstat", "gives information details on a file.",
    [&](const std::string &seq, const std::string &req, void *) {
      std::thread([&, seq, req] {
        auto sp=webview::detail::json_parse(req, "", 0);
        w.resolve(seq, 0, do_fstat(sp));
      }).detach();
    },
    nullptr);

  bind_doc(w,
    "fwrite", "truncate and write to file with provided file name.", [&](const std::string &req) -> std::string {
    std::string fn, s;
    int n=0;
    fn=webview::detail::json_parse(req, "", n++);
    for(;;) {
      s=webview::detail::json_parse(req, "", n++);
      if (s.empty()) break;
      if (n == 2) {
        fwrite(fn, s, std::ios::out);
      } else {
        fwrite(fn, "\n");
        fwrite(fn, s);
      }
    }
    return "";
  });

  bind_doc(w, "fappend", "append to file with provided file name.", [&](const std::string &req) -> std::string {
    std::string fn, s;
    int n=0;
    fn=webview::detail::json_parse(req, "", n++);
    for(;;) {
      s=webview::detail::json_parse(req, "", n++);
      if (s.empty()) break;
      if (n == 2) {
        fwrite(fn, s);
      } else {
        fwrite(fn, "\n");
        fwrite(fn, s);
      }
    }
    return "";
  });

  bind_doc(w, "wait_nothread", "wait in foreground", [&](const std::string & req) -> std::string {
    return sec_wait(webview::detail::json_parse(req, "", 0));
  });

  bind_doc(w, 
    "wait_thread", "wait in background",
    [&](const std::string &seq, const std::string &req, void * /*arg*/) {
    std::thread([&, seq, req] {
      auto result = sec_wait(webview::detail::json_parse(req, "", 0));
      w.resolve(seq, 0, result);
    }).detach();
  },
  nullptr);

  bind_doc(w, 
      "absolute", "gives absolute path of a file path.",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          auto pth=webview::detail::json_parse(req, "", 0);
          auto res=std::filesystem::absolute(pth).string();
          replace_all(res, "\\", "/");
          w.resolve(seq, 0, '"'+res+'"');
        }).detach();
      },
      nullptr);

  bind_doc(w, "chdir", "change current directory.", [&](const std::string &req) -> std::string {
    nfs=0;
    auto pth=webview::detail::json_parse(req, "", 0);
    std::filesystem::current_path(pth);
    return "";
  });

  bind_doc(w, 
    "httpget", "run an http get with the provided url.",
    [&](const std::string &seq, const std::string &req, void *) {
      std::thread([&, seq, req] {
        auto sp=webview::detail::json_parse(req, "", 0);
        auto res=httpget(sp);
        std::cout << res << std::endl;
        w.resolve(seq, 0, res);
      }).detach();
    },
    nullptr);

  bind_doc(w, 
      "ls_attach", "list provided directory in foreground.",
      [&](const std::string &req) {
          auto param=webview::detail::json_parse(req, "", 0);
          auto res=lsdir(param);
          std::cout << res << std::endl;
          return res;
      });

  bind_doc(w, 
      "ls", "list provided directory in background",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          auto param=webview::detail::json_parse(req, "", 0);
          auto res=lsdir(param);
          //std::cout << res << std::endl;
          w.resolve(seq, 0, res);
        }).detach();
      },
      nullptr);

  bind_doc(w, 
      "lsr", "list provided directory recursively in background ==> very dangerous may hangs the machine !",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          w.resolve(seq, 0, lsdir(webview::detail::json_parse(req, "", 0), true));
        }).detach();
      },
      nullptr);

  bind_doc(w, "webapp_get_title", "return webapp window title in foreground.", [&](const std::string &req) -> std::string {
        std::string prev_title="";
#ifdef _WIN32
        prev_title=GetWindowText((HWND)w.window());
#endif
        return "{\"value\": \""+prev_title+"\"}";
  });

  bind_doc(w, "webapp_get_title_bg", "return webapp window title in background.", [&](const std::string &seq, const std::string &req, void *) {
     std::thread([&, seq, req] {
        std::string prev_title="";
#ifdef _WIN32
        prev_title=GetWindowText((HWND)w.window());
#endif
        auto result="{\"value\": \""+prev_title+"\"}";
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );

  // Change window title
  bind_doc(w, "webapp_title", "change webapp window title.", [&](const std::string &seq, const std::string &req, void *) {
      std::thread([&, seq, req] {
        std::string prev_title="";
#ifdef _WIN32
        prev_title=GetWindowText((HWND)w.window());
#endif
        auto title=webview::detail::json_parse(req, "", 0);
        w.set_title(title);
        auto result="{\"value\": \""+prev_title+"\"}";
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );

  // Change window dimension and sizing behaviour
  bind_doc(w, "webapp_size", "set dimension and hint of webapp window.", [&](const std::string & req) -> std::string {
      auto params=webview::detail::json_parse(req, "", 0);
      auto l_width = std::stoi(webview::detail::json_parse(req, "", 0));
      auto l_height = std::stoi(webview::detail::json_parse(req, "", 1));
      auto l_hints = std::stoi(webview::detail::json_parse(req, "", 2));
      w.set_size(l_width, l_height, l_hints);
      return "";
    });

  // Exit from the web application
  bind_doc(w, "webapp_exit", "exit from webapp.", [&](const std::string &) -> std::string { w.terminate(); return ""; });


  // Run a local command and return an eventual result at a later time.
  bind_doc(w, "webapp_exec", "run an external command.", [&](const std::string &seq, const std::string &req, void *) {
      std::thread([&, seq, req] {
        auto cmd=webview::detail::json_parse(req, "", 0);
        std::string res_cmd=exec_cmd(cmd);
        //std::cout << res_cmd << std::endl;
        std::string bs;
        bs=(char)92;
        replace_all(res_cmd, bs, "##BACKSLASH_CODE##");
        rep_crlf(res_cmd);
        replace_all(res_cmd, "##BACKSLASH_CODE##", bs+bs);
        auto result="{\"value\": \""+res_cmd+"\"}";
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );


  bind_doc(w, "write", "write a string to stdout.", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s);
    return "";
  });

  bind_doc(w, "writeln", "write a string to stdout and add a carriage return.", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s);
    write_cons("\n");
    return "";
  });

  bind_doc(w, "ewrite", "write a string to stderr.", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s, std::cerr);
    return "";
  });

  bind_doc(w, "ewriteln", "write a string to stderr and add a carriage return.", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s, std::cerr);
    write_cons("\n", std::cerr);
    return "";
  });
}

bool run_and_exit=false;
void webview_set(bool devmode, int width, int height, int hints, bool _run_and_exit) {
  if (w == nullptr) {
    void *wnd=nullptr;
    run_and_exit=_run_and_exit;

#ifdef _WIN32
    if (run_and_exit) {
      if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        // Console mode, webview window will not be rendered.
        //HWND hwnd; extern HWND CreateWin(); hwnd=CreateWin(); wnd=&hwnd;
      } else {
        // GUI mode is not compatible with run_and_exit option
        run_and_exit=false;
        devmode=true;
      }
    }
#endif

    w=new webview::webview(devmode, (void *)wnd);
    w->set_size(width, height, hints);
    create_binds(*w);
  }
}

void webview_run(std::string url, std::string title, std::string init_js) {
  w->set_title(title);

  if (run_and_exit) {
    if (init_js.back() != ';') init_js+=';';
    init_js+=" webapp_exit();";
    w->init(init_js);

    w->set_html("html://<div></div>");
  } else {
    if (!init_js.empty()) { w->init(init_js); }

    if (url.starts_with("html://")) {
      w->set_html(url);
    } else {
      w->navigate(url);
    }
  }

  //std::cout << init_js << std::endl;

  w->run();
  delete w;
}

