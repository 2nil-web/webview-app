
#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#else
#include <string.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <chrono>
#include <codecvt>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <filesystem>
#endif

#include "wv-util.h"

bool any_of_ctype(const std::string s, std::function<int(int)> istype)
{
  return std::any_of(s.begin(), s.end(), [istype](char c) { return istype(c); });
}

bool all_of_ctype(const std::string s, std::function<int(int)> istype)
{
  return std::all_of(s.begin(), s.end(), [istype](char c) { return istype(c); });
}

std::string trim(std::string &s)
{
  s.erase(0, s.find_first_not_of(" \n\r\t"));
  s.erase(s.find_last_not_of(" \n\r\t") + 1);
  return s;
}

std::string file2str(std::string filename)
{
  std::ifstream ifs(filename);
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

#ifdef _WIN32
void WinError(const char *fmt, ...)
{
  CHAR *lpMsgBuf;
  char title[1024];
  DWORD len;
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(title, 1024, fmt, ap);
  va_end(ap);

  len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
  lpMsgBuf[len - 2] = '\0';
  MessageBox(NULL, lpMsgBuf, title, MB_OK | MB_ICONERROR);
  LocalFree(lpMsgBuf);
}

#ifndef UNICODE
PCHAR *CommandLineToArgvA(PCHAR CmdLine, int *_argc)
{
  PCHAR *argv;
  PCHAR _argv;
  ULONG len;
  ULONG argc;
  CHAR a;
  ULONG i, j;

  BOOLEAN in_QM;
  BOOLEAN in_TEXT;
  BOOLEAN in_SPACE;

  len = (ULONG)strlen(CmdLine);
  i = ((len + 2) / 2) * sizeof(PVOID) + sizeof(PVOID);
  argv = (PCHAR *)GlobalAlloc(GMEM_FIXED, i + (len + 2) * sizeof(CHAR));
  _argv = (PCHAR)(((PUCHAR)argv) + i);
  argc = 0;
  argv[argc] = _argv;
  in_QM = FALSE;
  in_TEXT = FALSE;
  in_SPACE = TRUE;
  i = 0;
  j = 0;

  while ((a = CmdLine[i]))
  {
    if (in_QM)
    {
      if (a == '\"')
      {
        in_QM = FALSE;
      }
      else
      {
        _argv[j] = a;
        j++;
      }
    }
    else
    {
      switch (a)
      {
      case '\"':
        in_QM = TRUE;
        in_TEXT = TRUE;
        if (in_SPACE)
        {
          argv[argc] = _argv + j;
          argc++;
        }
        in_SPACE = FALSE;
        break;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        if (in_TEXT)
        {
          _argv[j] = '\0';
          j++;
        }
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        break;
      default:
        in_TEXT = TRUE;
        if (in_SPACE)
        {
          argv[argc] = _argv + j;
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

void replace_all(std::string &s, std::string srch, std::string repl)
{
  size_t pos = 0;
  while (pos += repl.length())
  {
    pos = s.find(srch, pos);
    if (pos == std::string::npos)
      break;
    s.replace(pos, srch.length(), repl);
  }
}

void replace_all(std::wstring &s, std::wstring srch, std::wstring repl)
{
  size_t pos = 0;
  while (pos += repl.length())
  {
    pos = s.find(srch, pos);
    if (pos == std::string::npos)
      break;
    s.replace(pos, srch.length(), repl);
  }
}

void rep_crlf(std::string &s)
{
  replace_all(s, "\r", "\\r");
  replace_all(s, "\n", "\\n");
}

std::string temppath()
{
  std::string tpath = "";
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
  tpath = std::filesystem::temp_directory_path().generic_string();
#else
#ifdef _WIN32
  char tpw[MAX_PATH - 14];
  if (GetTempPath(MAX_PATH - 14, tpw) != 0)
    tpath = std::string(tpw);
#else
  tpath = "/tmp";
#endif
#endif
  return tpath;
}

std::string tempfile(std::string tpath, std::string pfx)
{
  std::string tfn = "";

  if (tpath.empty())
    tpath = temppath();
  if (pfx.empty())
    pfx = "tmp.XXXXXX";
#ifdef _WIN32
  replace_all(tpath, "/", "\\");
  char tfnw[MAX_PATH];
  if (GetTempFileName(tpath.c_str(), pfx.c_str(), 0, tfnw) != 0)
    tfn = tfnw;
#else
  tfn = std::string((tpath + pfx).c_str());
  char *stfn = strdup(tfn.c_str());
  int id = mkstemp(stfn);

  if (id != -1)
  {
    close(id);
    unlink(stfn);
    tfn = stfn;
  }
#endif

  return std::string(tfn);
}

#ifdef _MSC_VER
// Converts UTF-16/wstring to UTF-8/string
std::string ws2s(const std::wstring ws)
{
  if (ws.empty())
    return std::string();

  UINT cp = CP_UTF8;
  DWORD flags = WC_ERR_INVALID_CHARS;
  auto wc = ws.c_str();
  auto wl = static_cast<int>(ws.size());
  auto l = WideCharToMultiByte(cp, flags, wc, wl, nullptr, 0, nullptr, nullptr);

  if (l > 0)
  {
    std::string s(static_cast<std::size_t>(l), '\0');
    if (WideCharToMultiByte(cp, flags, wc, wl, &s[0], l, nullptr, nullptr) > 0)
      return s;
  }

  return std::string();
}

// Converts UTF-8/string to UTF-16/wstring
std::wstring s2ws(const std::string s)
{
  if (s.empty())
    return std::wstring();

  UINT cp = CP_UTF8;
  DWORD flags = MB_ERR_INVALID_CHARS;
  auto c = s.c_str();
  auto l = static_cast<int>(s.size());
  auto wl = MultiByteToWideChar(cp, flags, c, l, nullptr, 0);
  if (wl > 0)
  {
    std::wstring ws(static_cast<std::size_t>(wl), L'\0');
    if (MultiByteToWideChar(cp, flags, c, l, &ws[0], wl) > 0)
      return ws;
  }

  return std::wstring();
}
#else
std::string ws2s(std::wstring ws)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(ws);
}

std::wstring s2ws(std::string s)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(s);
}
#endif

// Return false if wide char is a printable ascii else true
bool not_printable_ascii(wchar_t wc)
{
  if (wc > 31 && wc < 127)
    return false;
  return true;
}

// Convert non ascii characters of a wstring to html entities form in decimal (default) or hexa &#[x]value;
std::string to_htent(const std::wstring ws, bool dec_base)
{
  std::stringstream ss;

  for (auto wc : ws)
  {
    if (not_printable_ascii(wc))
    {
      ss << "&#";
      if (dec_base) ss << std::dec;
      else ss << 'x' << std::hex;
      ss << (unsigned int)wc << ';';
    }
    else
      ss << (char)wc;
  }

  return ss.str();
}

// Same as previous for string
std::string to_htent(const std::string s, bool dec_base)
{
  return to_htent(s2ws(s), dec_base);
}

std::string s2n(std::string s, size_t& i) {
  std::string h="";
  do { h+=s[i++]; } while (i < s.size() && s[i] != ';');
  return h;
}

unsigned int stoh(std::string s) {
  unsigned int x;   
  std::stringstream ss;
  ss << std::hex << s;
  ss >> x;
  return x;
}

// Convert the html entities in hexa or decimal form contained in a string to their wchar_t value, return the obtained string
std::wstring htent_to_ws(const std::string s)
{
  // &#0;
  if (s.empty() || s.size () < 4) return s2ws(s);

  std::wstring ws=L"";
  std::string h;
  for (size_t i=0; i < s.size(); i++) {
    if (s.substr(i, 3) == "&#x") {
      i+=2;
      ws+=(wchar_t)stoh(s2n(s, i));
    } else if (s.substr(i, 2) == "&#") {
      i+=2;
      ws+=(wchar_t)std::stoi(s2n(s, i));
    } else {
      ws+=(wchar_t)s[i];
    }
  }

  return ws;
}

// Same as previous for string
std::string htent_to_s(const std::string s)
{
  return ws2s(htent_to_ws(s));
}

#ifdef _WIN32
// Convert an utf8 string into an url encoded hexadecimal one
std::string s2h(const std::string s)
{
  std::string ret;
  char hs[5];
  for (const char *p = s.c_str(); *p; p++)
  {
    _snprintf_s(hs, sizeof(hs), sizeof(hs), "%%%2.2X", (unsigned char)*p);
    ret.append(hs);
  }

  return ret;
}

// Convert an url hexadecimal encoded string into an utf8 string
std::string h2s(const std::string s)
{
  std::string ret;
  int ch;

  for (const char *p = s.c_str(); *p;)
  {
    if (p[0] == '%' && isxdigit(p[1]) && isxdigit(p[2]))
    {
      ch = (isdigit(p[1]) ? p[1] - '0' : toupper(p[1]) - 'A' + 10) * 16 +
           (isdigit(p[2]) ? p[2] - '0' : toupper(p[2]) - 'A' + 10);
      ret.push_back((char)ch);
      p += 3;
    }
    else if (p[0] == '%' && p[1] == '#' && isdigit(p[2]))
    {
      ch = atoi(p + 2);
      ret.push_back((char)ch);
      p += 2;
      while (*p && isdigit(*p))
        p++;
      if (*p == ';')
        p++;
    }
    else
      ret.push_back(*p++);
  }

  return ret;
}

// Convert a wstring to into an url encoded hexadecimal string <=> to javascript
std::string ws2u8h(const std::wstring s)
{
  return s2h(ws2s(s));
}

// Convert an url encoded hexadecimal string into a wstring
std::wstring u8h2ws(const std::string s)
{
  return s2ws(h2s(s));
}

std::wstring wfile2wstr(std::string filename)
{
  std::wifstream wif(filename);
#ifndef _MSC_VER
  wif.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
/*#else
    wif.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t,
   0x10FFFF, std::consume_header>));*/
#endif
  std::wstringstream wss;
  wss << wif.rdbuf();
  return wss.str();
}

const wchar_t *getcmdw()
{
  static wchar_t cmd_path[MAX_PATH];
  if (SearchPathW(nullptr, L"cmd", L".exe", MAX_PATH, cmd_path, nullptr))
    return cmd_path;
  else
    return L"C:\\Windows\\System32\\cmd.exe";
}

const char *getcmda()
{
  static char cmd_path[MAX_PATH];
  if (SearchPathA(nullptr, "cmd", ".exe", MAX_PATH, cmd_path, nullptr))
    return cmd_path;
  else
    return "C:\\Windows\\System32\\cmd.exe";
}

std::wstring SystemToString(const std::string cmd)
{
  std::wstring wcmd = s2ws("/C " + cmd);
  std::string tmpFile = tempfile();
  std::wstring wtmpFile = s2ws(tmpFile);

  AllocConsole();
  ShowWindow(GetConsoleWindow(), SW_HIDE);
  SetConsoleOutputCP(65001);

  STARTUPINFOW si = {};
  si.cb = sizeof(STARTUPINFOW);
  PROCESS_INFORMATION pi = {};
  SECURITY_ATTRIBUTES se = {};
  se.nLength = sizeof(SECURITY_ATTRIBUTES);

  HANDLE hFile = CreateFileW(wtmpFile.c_str(), FILE_WRITE_DATA, FILE_SHARE_DELETE | FILE_SHARE_READ, &se, OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY, 0);
  SetStdHandle(STD_OUTPUT_HANDLE, hFile);
  SetStdHandle(STD_ERROR_HANDLE, hFile);
  CreateProcessW(getcmdw(), const_cast<wchar_t *>(wcmd.c_str()), 0, 0, 0, 0, 0, 0, &si, &pi);
  WaitForSingleObject(pi.hProcess, INFINITE);

  // FlushFileBuffers(GetStdHandle(STD_OUTPUT_HANDLE));
  // FlushFileBuffers(GetStdHandle(STD_ERROR_HANDLE));
  // FlushFileBuffers(hFile);

  std::wstring s = wfile2wstr(tmpFile);
  CloseHandle(hFile);
  DeleteFileW(wtmpFile.c_str());
  return s;
}
#endif

std::string exec_cmd(std::string cmd)
{
#ifdef _WIN32
  return ws2s(SystemToString(cmd));
#else
  std::string tf = tempfile();
  std::string fullcmd = cmd + " > " + tf;
  std::system(fullcmd.c_str());
  std::string s = file2str(tf);
  std::filesystem::remove(tf);
  return s;
#endif
}

std::string rep_bs(std::string &s)
{
  std::string bs;
  bs = (char)92;
  replace_all(s, bs, "##BACKSLASH_CODE##");
  rep_crlf(s);
  replace_all(s, "##BACKSLASH_CODE##", bs + bs);
  return s;
}

// Filesystem api exposed to javascript
std::string pwd()
{
  return std::filesystem::current_path().string();
}

std::string cwd(std::string new_dir)
{
  std::filesystem::current_path(new_dir);
  return pwd();
}
