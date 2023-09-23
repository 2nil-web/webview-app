

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

#include <webview.h>

#include "util.h"
#include "winapi.h"

webview::webview *w=nullptr;


void write_cons(std::string s, std::ostream& out=std::cout) {
  if (s.empty()) return;
#ifdef _WIN32
  char title[256];
  std::string tit="";
  int gctres=GetConsoleTitle(title, 255);
  //out << gctres << std::endl;
  if (gctres > 0) {
    tit=title;
    //MessageBox(NULL, "", title, MB_OK);
    //out << gctres << ", [[" << title << "]]" << std::endl; //out.flush();

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

// Return a javascript array of strings corresponding to a directory list, recursively or not
std::string lsdir(std::string path, bool recursive=false) {
  std::string res="[";
  if (path.empty()) path=".";

  if (recursive) 
    for (const auto& e:std::filesystem::recursive_directory_iterator(path))
      res+='"'+e.path().string()+"\",";
  else
    for (const auto& e:std::filesystem::directory_iterator(path))
      res+='"'+e.path().string()+"\",";

  res[res.size()-1]=']';
  replace_all(res, "\\", "/");
  //std::cout << res << std::endl;
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
    std::cout << "none      :" << (unsigned int)std::filesystem::file_type::none << std::endl;
    std::cout << "not_found :" << (unsigned int)std::filesystem::file_type::not_found << std::endl;
    std::cout << "regular   :" << (unsigned int)std::filesystem::file_type::regular << std::endl;
    std::cout << "directory :" << (unsigned int)std::filesystem::file_type::directory << std::endl;
    std::cout << "symlink   :" << (unsigned int)std::filesystem::file_type::symlink << std::endl;
    std::cout << "block     :" << (unsigned int)std::filesystem::file_type::block << std::endl;
    std::cout << "character :" << (unsigned int)std::filesystem::file_type::character << std::endl;
    std::cout << "fifo      :" << (unsigned int)std::filesystem::file_type::fifo << std::endl;
    std::cout << "socket    :" << (unsigned int)std::filesystem::file_type::socket << std::endl;
    std::cout << "unknown   :" << (unsigned int)std::filesystem::file_type::unknown << std::endl;
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
    default : 
                                                 return 9;
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

void create_binds(webview::webview &w) {
  w.bind("wait_nothread", [&](const std::string & req) -> std::string {
    return sec_wait(webview::detail::json_parse(req, "", 0));
  });

  w.bind(
    "wait_thread",
    [&](const std::string &seq, const std::string &req, void * /*arg*/) {
    std::thread([&, seq, req] {
      auto result = sec_wait(webview::detail::json_parse(req, "", 0));
      w.resolve(seq, 0, result);
    }).detach();
  },
  nullptr);

   w.bind(
      "fstat",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          std::filesystem::path p=webview::detail::json_parse(req, "", 0);
          auto fs=std::filesystem::status(p);

          auto ft=fs.type();
          std::uintmax_t sz;
          if (ft != std::filesystem::file_type::directory &&
              ft != std::filesystem::file_type::not_found &&
              ft != std::filesystem::file_type::unknown   &&
              ft != std::filesystem::file_type::none)
            sz=std::filesystem::file_size(p);
          else sz=static_cast<std::uintmax_t>(-1);

          auto res="{\"type\":" + std::to_string(forced_file_type(ft))+",\"perms\":"+to_js_oct((unsigned)fs.permissions())+",\"size\":"+std::to_string(sz)+"}";
          //unsigned int p=(unsigned int)fs.permissions();
          //std::cout << p << " <=> " << to_js_oct(p) <<  " <=> " << to_js_hex(p) << std::endl;
          w.resolve(seq, 0, res);
        }).detach();
      },
      nullptr);

  // ls().then(r=>{r.forEach((d)=>console.log(d))});
  // ls().then(r=>{r.forEach((d)=>output_text.value+=d+"\n")});
  /*
ls().then(r=>{
  r.forEach((d)=>output_text.value+=d.replace(/^\.\//, "")+"\n");
  output_text.value+=r.length+" files(s)\n";
});
  */
  // ls().then(r=>{r.forEach((d)=>writeln(d))});
  w.bind(
      "absolute",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          auto pth=webview::detail::json_parse(req, "", 0);
          auto res=std::filesystem::absolute(pth).string();
//          std::cout << "absolute, canonical        " << std::filesystem::canonical(pth).string() << std::endl;
//          std::cout << "absolute, weakly_canonical " << std::filesystem::weakly_canonical(pth).string() << std::endl;
          //std::cout << std::filesystem::canonical(pth).string() << std::endl;
          replace_all(res, "\\", "/");
//          std::cout << res << std::endl;
          w.resolve(seq, 0, '"'+res+'"');
        }).detach();
      },
      nullptr);

  w.bind("chdir", [&](const std::string &req) -> std::string {
    auto pth=webview::detail::json_parse(req, "", 0);
    std::filesystem::current_path(pth);
//    std::cout << "chdir, canonical        " << std::filesystem::canonical(pth).string() << std::endl;
//    std::cout << "chdir, weakly_canonical " << std::filesystem::weakly_canonical(pth).string() << std::endl;
    return "";
  });

  w.bind(
      "ls",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          w.resolve(seq, 0, lsdir(webview::detail::json_parse(req, "", 0)));
        }).detach();
      },
      nullptr);

  w.bind(
      "lsr",
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          w.resolve(seq, 0, lsdir(webview::detail::json_parse(req, "", 0), true));
        }).detach();
      },
      nullptr);

//  w.bind("simple_ls", [&](const std::string &req) -> std::string { return lsdir(webview::detail::json_parse(req, "", 0)); });

  // Change window title
  w.bind("webapp_get_title", [&](const std::string &seq, const std::string &req, void *) {
     std::thread([&, seq, req] {
        std::string prev_title="";
#ifdef _WIN32
        prev_title=GetWindowText((HWND)w.window());
#endif
        auto result="{\"value\": \""+prev_title+"\"}";
        //auto result=res_cmd;
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );

  w.bind("webapp_title", [&](const std::string &seq, const std::string &req, void *) {
      std::thread([&, seq, req] {
        std::string prev_title="";
#ifdef _WIN32
        prev_title=GetWindowText((HWND)w.window());
#endif
        auto title=webview::detail::json_parse(req, "", 0);
        w.set_title(title);
        auto result="{\"value\": \""+prev_title+"\"}";
        //auto result=res_cmd;
        w.resolve(seq, 0, result);
      }).detach();
    },
    nullptr
  );

  // Change window dimension and sizing behaviour
  w.bind("webapp_size", [&](const std::string & req) -> std::string {
      auto params=webview::detail::json_parse(req, "", 0);
      auto l_width = std::stoi(webview::detail::json_parse(req, "", 0));
      auto l_height = std::stoi(webview::detail::json_parse(req, "", 1));
      auto l_hints = std::stoi(webview::detail::json_parse(req, "", 2));
      w.set_size(l_width, l_height, l_hints);
      return "";
    });

  // Exit from the web application
  w.bind("webapp_exit", [&](const std::string &) -> std::string { w.terminate(); return ""; });


  // Run a local command and return an eventual result at a later time.
  w.bind("webapp_exec", [&](const std::string &seq, const std::string &req, void *) {
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


  w.bind("write", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s);
    return "";
  });

  w.bind("writeln", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s);
    write_cons("\n");
    return "";
  });

  w.bind("ewrite", [&](const std::string &req) -> std::string {
    auto s=webview::detail::json_parse(req, "", 0);
    write_cons(s, std::cerr);
    return "";
  });

  w.bind("ewriteln", [&](const std::string &req) -> std::string {
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

