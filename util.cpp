
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <string.h>
#endif

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <functional>
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <filesystem>
#endif

#include "util.h"

bool any_of_ctype(const std::string s, std::function<int(int)> istype) {
  return std::any_of(s.begin(), s.end(), [istype](char c) { return istype(c); } );
}

bool all_of_ctype(const std::string s, std::function<int(int)> istype) {
  return std::all_of(s.begin(), s.end(), [istype](char c) { return istype(c); } );
}

std::string trim(std::string& s) {
  s.erase(0, s.find_first_not_of(" \n\r\t"));
  s.erase(s.find_last_not_of(" \n\r\t")+1);
  return s;
}

#ifdef _WIN32
PCHAR* CommandLineToArgvA( PCHAR CmdLine, int* _argc) {
    PCHAR* argv;
    PCHAR  _argv;
    ULONG   len;
    ULONG   argc;
    CHAR   a;
    ULONG   i, j;

    BOOLEAN  in_QM;
    BOOLEAN  in_TEXT;
    BOOLEAN  in_SPACE;

    len = strlen(CmdLine);
    i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);
    argv = (PCHAR*)GlobalAlloc(GMEM_FIXED, i + (len+2)*sizeof(CHAR));
    _argv = (PCHAR)(((PUCHAR)argv)+i);
    argc = 0;
    argv[argc] = _argv;
    in_QM = FALSE;
    in_TEXT = FALSE;
    in_SPACE = TRUE;
    i = 0;
    j = 0;

    while((a = CmdLine[i])) {
        if(in_QM) {
            if(a == '\"') {
                in_QM = FALSE;
            } else {
                _argv[j] = a;
                j++;
            }
        } else {
            switch(a) {
            case '\"':
                in_QM = TRUE;
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                in_SPACE = FALSE;
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                if(in_TEXT) {
                    _argv[j] = '\0';
                    j++;
                }
                in_TEXT = FALSE;
                in_SPACE = TRUE;
                break;
            default:
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                _argv[j] = a;
                j++;
                in_SPACE = FALSE;
                break;
            }
        }
        i++;
    }
    _argv[j] = '\0';
    argv[argc] = NULL;

    (*_argc) = argc;
    return argv;
}
#endif

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

