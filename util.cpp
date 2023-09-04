
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
#include <locale>
#include <codecvt>
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
#ifndef UNICODE
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

    len = (ULONG)strlen(CmdLine);
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
    argv[argc] = nullptr;

    (*_argc) = argc;
    return argv;
}
#endif
#endif

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
  if (pfx.empty()) pfx="tmp.XXXXXX";
#ifdef _WIN32
  replace_all(tpath, "/", "\\");
  char tfnw[MAX_PATH];
  if (GetTempFileName(tpath.c_str(), pfx.c_str(), 0, tfnw) != 0) tfn=tfnw;
#else
  tfn=std::string((tpath+pfx).c_str());
  int id=mkstemp(tfn.c_str());

  if (id != -1) {
    close(id);
    unlink(tfnl.c_str());
    tfn=tfnl;
  }
#endif

  return std::string(tfn);
}

#ifdef _WIN32
#ifdef _MSC_FULL_VER
UINT CodePage=CP_ACP;
DWORD dwFlags=0;
std::string ws2s(std::wstring ws) {
  int l=WideCharToMultiByte(CodePage, dwFlags, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);

  if (l > 0) {
    char *sTo=new char[l+1];
    WideCharToMultiByte(CodePage, dwFlags, ws.c_str(), -1, sTo, l, nullptr, nullptr);
    sTo[l]='\0';
    std::string s=sTo;
    delete sTo;
    return s;
  }

  return "";
}

std::wstring s2ws(std::string s) {
  wchar_t *wsTo=new wchar_t[s.size()+1];
  wsTo[s.size()]='\0';
  MultiByteToWideChar(CodePage, dwFlags, s.c_str(), -1, wsTo, (int)s.length());
  std::wstring ws=wsTo;
  delete wsTo;
  return ws;
}
#else

std::string ws2s(std::wstring ws) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(ws);
}

std::wstring s2ws(std::string s) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(s);
}
#endif

std::wstring wfile2wstr(std::string filename) {
    std::wifstream wif(filename);
    wif.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << wif.rdbuf();
    return wss.str();
}

std::string file2str(std::string filename) {
    std::ifstream ifs(filename);
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

const wchar_t *getcmdw() {
  static wchar_t cmd_path[MAX_PATH];
  if (SearchPathW(nullptr, L"cmd", L".exe", MAX_PATH, cmd_path, nullptr)) return cmd_path;
  else return L"C:\\Windows\\System32\\cmd.exe";
}

const char *getcmda() {
  static char cmd_path[MAX_PATH];
  if (SearchPathA(nullptr, "cmd", ".exe", MAX_PATH, cmd_path, nullptr)) return cmd_path;
  else return "C:\\Windows\\System32\\cmd.exe";
}

std::wstring SystemToString(const std::string cmd) {
  std::wstring wcmd=s2ws("/C "+cmd);
  std::string tmpFile=tempfile();
  std::wstring wtmpFile=s2ws(tmpFile);

  AllocConsole();
  ShowWindow(GetConsoleWindow(), SW_HIDE);
  SetConsoleOutputCP(65001);

  STARTUPINFOW si={};
  si.cb=sizeof(STARTUPINFOW);
  PROCESS_INFORMATION pi={};
  SECURITY_ATTRIBUTES se={};
  se.nLength=sizeof(SECURITY_ATTRIBUTES);

  HANDLE hFile=CreateFileW(wtmpFile.c_str(), FILE_WRITE_DATA, FILE_SHARE_DELETE | FILE_SHARE_READ, &se, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY , 0);
  SetStdHandle(STD_OUTPUT_HANDLE, hFile);
  SetStdHandle(STD_ERROR_HANDLE, hFile);
  CreateProcessW(getcmdw(), const_cast<wchar_t*>(wcmd.c_str()), 0, 0, 0, 0, 0, 0, &si, &pi);
  WaitForSingleObject(pi.hProcess,INFINITE);

  //FlushFileBuffers(GetStdHandle(STD_OUTPUT_HANDLE));
  //FlushFileBuffers(GetStdHandle(STD_ERROR_HANDLE));
  //FlushFileBuffers(hFile);
  
  std::wstring s=wfile2wstr(tmpFile);
  CloseHandle(hFile);
  DeleteFileW(wtmpFile.c_str());
  return s;
}
#endif

std::string exec_cmd(std::string cmd) {
#ifdef _WIN32
  return ws2s(SystemToString(cmd));
#else
  std::string tf=tempfile();
  std::string fullcmd=cmd+" > "+tf;
  std::system(fullcmd.c_str());
  std::string s=file2str(tf);
  std::filesystem::remove(tf);
  return s;
#endif
}

std::string rep_bs(std::string &s) {
  std::string bs;
  bs=(char)92;
  replace_all(s, bs, "##BACKSLASH_CODE##");
  rep_crlf(s);
  replace_all(s, "##BACKSLASH_CODE##", bs+bs);
  return s;
}

// Filesystem api exposed to javascript
std::string pwd() {
  return std::filesystem::current_path().string();
}

std::string cwd(std::string new_dir) {
  std::filesystem::current_path(new_dir);
  return pwd();
}

std::string listdir(std::string path) {
  std::string res="";
  if (path.empty()) {
    path=std::filesystem::current_path().string();
  }

  for (const auto & entry : std::filesystem::directory_iterator(path)) {
    res+=entry.path().string()+"\n";
  }

  return res;
}

