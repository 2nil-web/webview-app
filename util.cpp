
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <string.h>
#endif

#include <iostream>
#include <iostream>
#include <sstream>
#include <fstream>
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <filesystem>
#endif

#include "util.h"

std::string temppath() {
  std::string tpath="";
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
  tpath=std::filesystem::temp_directory_path().generic_string();
#else
#ifdef _WIN32
  char tpw[MAX_PATH-14];
  if (GetTempPath(MAX_PATH-14, tpw) != 0) tpath=std::string(tpw);
#else
  tpath="/tmp";
#endif
#endif
  return tpath;
}

std::string tempfile(std::string tpath, std::string pfx) {
  std::string tfn="";

  if (tpath.empty()) tpath=temppath();
  if (pfx.empty()) pfx="temp.XXXXXX";
#ifdef _WIN32
  char tfnw[MAX_PATH];
  if (GetTempFileName(tpath.c_str(), pfx.c_str(), 0, tfnw) != 0) tfn=tfnw;
#else
  char *tfnl=strdup((tpath+'/'+pfx).c_str());
  int id=mkstemp(tfnl);

  if (id != -1) {
    close(id);
    unlink(tfnl);
    tfn=tfnl;
  } else {
    free(tfnl);
  }
#endif

  return std::string(tfn);
}

std::string exec_cmd(std::string cmd) {
  std::string tf=tempfile();
  std::cout << tf << std::endl;
  std::string fullcmd=cmd+" > "+tf;
  std::system(fullcmd.c_str());
  std::ifstream t(tf);
  std::stringstream buffer;
  buffer << t.rdbuf();
  std::string res=buffer.str();
  t.close();
  return res;
}

void replace_all(std::string &s, std::string srch, std::string repl) {
  size_t pos=0;
  while (pos += repl.length()) {
    pos=s.find(srch, pos);
    if (pos == std::string::npos) break;
    s.replace(pos, srch.length(), repl);
  }
}

void rep_crlf(std::string &s) {
  replace_all(s, "\r", "\\r");
  replace_all(s, "\n", "\\n");
}

