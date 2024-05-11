
#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#include <shellapi.h>
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

#ifdef _WIN32
#include "wv-winapi.h"
#endif
#include "wv-util.h"

std::string my_getenv(const std::string var)
{
//  std::cout << "var " << var << std::endl;

  char *pValue;
#ifdef _MSC_VER
  size_t len;
  errno_t err;
  err = _dupenv_s(&pValue, &len, var.c_str());

  if (err) {
    std::cout << "var " << var << " is empty or does not exist" << std::endl;
    return "";
  }
#else
  pValue=getenv(var.c_str());
  if (pValue == NULL) return "";
#endif
  return pValue;
}

bool my_setenv(const std::string var, const std::string val)
{
//  std::cout << "var " << var << ", val " << val << std::endl;

#ifdef _WIN32
  return (_putenv((char *)(var+'='+val).c_str()) == 0);
#else
  return (setenv((char *)var.c_str(), (char *)val.c_str(), 1) == 0);
#endif
}

// return if string == true/ok/yes/1 else false
bool str2bool(std::string s)
{
  if (s == "true" || s == "ok" || s == "1" || s == "yes")
    return true;
  return false;
}

bool any_of_ctype(const std::string s, std::function<int(int)> istype)
{
  return std::any_of(s.begin(), s.end(), [istype](char c) { return istype(c); });
}

bool all_of_ctype(const std::string s, std::function<int(int)> istype)
{
  return std::all_of(s.begin(), s.end(), [istype](char c) { return istype(c); });
}

std::vector<std::string> split(const std::string &str, char delim)
{
  std::vector<std::string> strings;
  size_t start;
  size_t end = 0;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
  {
    end = str.find(delim, start);
    strings.push_back(str.substr(start, end - start));
  }
  return strings;
}

std::string trim(std::string &s)
{
  s.erase(0, s.find_first_not_of(" \n\r\t"));
  s.erase(s.find_last_not_of(" \n\r\t") + 1);
  return s;
}

std::string stream2str(std::ifstream &ifs)
{
  std::stringstream ss;
  ss << ifs.rdbuf();
  // std::cout << "stream2str " << ss.str() << std::endl;
  return ss.str();
}

std::string path2str(std::filesystem::path p)
{
  if (!std::filesystem::exists(p))
    return "File " + p.string() + "does not exists";
  // std::cout << "p " << p << std::endl;

  std::ifstream ifs(p);
  return stream2str(ifs);
}

std::string file2str(std::string filename)
{
  if (filename == "")
    return "File name cannot be empty";
  return path2str(std::filesystem::path(filename));
}

std::string file2str(std::wstring wfilename)
{
  if (wfilename == L"")
    return "File name cannot be empty";
  return path2str(std::filesystem::path(wfilename));
}

std::string fread(std::string filename)
{
  return to_htent(file2str(s2ws(filename)));
}

std::string fread(std::wstring wfilename)
{
  return to_htent(file2str(wfilename));
}

std::wstring wstream2wstr(std::wifstream &wifs)
{
  std::wstringstream wss;
  wss << wifs.rdbuf();
  // std::wcout << L"wstream2wstr " << wss.str() << std::endl;
  return wss.str();
}

std::string wfile2str(std::string filename)
{
  if (filename == "")
    return "File name cannot be empty";
  if (!std::filesystem::exists(filename))
    return "File " + filename + "does not exists";
  // std::cout << "wfile2str_s " << filename << std::endl;

  std::wifstream wifs(filename);
  return ws2s(wstream2wstr(wifs));
}

void replace_all(std::string &s, std::string srch, std::string repl)
{
  size_t pos = 0, retpos;
  for (;;)
  {
    retpos = s.find(srch, pos);
    if (retpos == std::string::npos)
      break;
    s.replace(retpos, srch.length(), repl);
    pos = retpos + repl.length();
  }
}

void replace_all(std::wstring &s, std::wstring srch, std::wstring repl)
{
  size_t pos = 0, retpos;
  for (;;)
  {
    retpos = s.find(srch, pos);
    if (retpos == std::string::npos)
      break;
    s.replace(retpos, srch.length(), repl);
    pos = retpos + repl.length();
  }
}

std::string rep_crlf(std::string s)
{
  replace_all(s, "\r", "\\r");
  replace_all(s, "\n", "\\n");
  return s;
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
  tfn = std::string((tpath + '/' + pfx).c_str());
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
bool must_convert_to_htent(wchar_t wc)
{
  if (wc == 92 || wc == 34 || wc < 32 || wc > 126)
    return true;
  return false;
  //  if (wc > 31 && wc < 127) return false;
  //  if (wc < 32 && wc > 126) return true;
  //  return true;
}

// Convert non ascii characters of a wstring to html entities form in decimal (default) or hexa &#[x]value;
std::string to_htent(const std::wstring ws, bool dec_base)
{
  std::stringstream ss;

  for (auto wc : ws)
  {
    if (!isascii(wc))
    {
      ss << "&#";
      if (dec_base)
        ss << std::dec;
      else
        ss << 'x' << std::hex;
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

std::string s2n(std::string s, size_t &i)
{
  std::string h = "";
  do
  {
    h += s[i++];
  } while (i < s.size() && s[i] != ';');
  return h;
}

unsigned int stoh(std::string s)
{
  unsigned int x;
  std::stringstream ss;
  ss << std::hex << s;
  ss >> x;
  return x;
}

// Convert the html entities in hexa or decimal form contained in the string htent to their wchar_t value, return the
// obtained string in ws
std::wstring from_htent(const std::string htent, std::wstring &ws)
{
  // &#0;
  if (htent.empty() || htent.size() < 4)
    ws = s2ws(htent);
  else
  {
    ws = L"";
    for (size_t i = 0; i < htent.size(); i++)
    {
      if (htent.substr(i, 3) == "&#x")
      {
        i += 2;
        ws += (wchar_t)stoh(s2n(htent, i));
      }
      else if (htent.substr(i, 2) == "&#")
      {
        i += 2;
        ws += (wchar_t)std::stoi(s2n(htent, i));
      }
      else
      {
        ws += (wchar_t)htent[i];
      }
    }
  }

  return ws;
}

// Same as previous for string
std::string from_htent(const std::string htent, std::string &s)
{
  std::wstring ws;
  s = ws2s(from_htent(htent, ws));
  return s;
}

#ifdef _WIN32
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
#endif

#ifdef _WIN32
#define my_popen _popen
#define my_pclose _pclose
std::string wpipe2s(const std::wstring command)
{
  FILE *fp = _wpopen(command.c_str(), L"r");

  if (fp)
  {
    std::wostringstream oss;
    constexpr std::size_t MAX_LINE_SZ = 1024;
    char line[MAX_LINE_SZ];
    while (fgetws((wchar_t *)line, MAX_LINE_SZ, fp))
      oss << (wchar_t *)line;
    my_pclose(fp);
    return to_htent(oss.str());
  }

  return "";
}
#else
#define my_popen popen
#define my_pclose pclose
#endif
std::string pipe2s(const std::string command)
{
  FILE *fp = my_popen(command.c_str(), "r");

  if (fp)
  {
    std::ostringstream oss;
    constexpr std::size_t MAX_LINE_SZ = 1024;
    char line[MAX_LINE_SZ];
    while (fgets(line, MAX_LINE_SZ, fp))
      oss << line;
    my_pclose(fp);
    return to_htent(oss.str());
  }

  return "";
}

std::string shell_cmd(std::string cmd, std::string param, std::string dir, std::string ope)
{
  if (cmd == "") return "First parameter cannot be empty";
#ifdef _WIN32
  LPCSTR lpOperation=NULL, lpFile=cmd.c_str(), lpParameters=NULL, lpDirectory=NULL;
  if (! ope.empty() && ope != "") lpOperation=ope.c_str();
  if (! param.empty() && param != "") lpParameters=param.c_str();
  if (! dir.empty() && dir != "") lpDirectory=dir.c_str();
 if (ShellExecute(NULL, lpOperation, lpFile, lpParameters, lpDirectory, SW_SHOW) <= (HINSTANCE)32)
      return StrError("ShellExecute error with file %s, parameters %s, directory %s\n", lpFile, lpParameters, lpDirectory);
#else
  pid_t pid=fork();
  if (pid == 0) execl(cmd.c_str(), param.c_str(), NULL);
  if (pid == -1) return strerror(errno);
#endif
  return "";
}

std::string shell_cmd_wait(std::string cmd, std::string param, std::string dir, std::string ope)
{
  if (cmd == "") return "First parameter cannot be empty";
#ifdef _WIN32
  SHELLEXECUTEINFO ShExecInfo = {0};
  ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
  ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  ShExecInfo.hwnd = NULL;
  ShExecInfo.lpVerb = NULL;
  ShExecInfo.lpFile = cmd.c_str();        
  ShExecInfo.lpParameters = NULL;   
  ShExecInfo.lpDirectory = NULL;
  ShExecInfo.nShow = SW_SHOW;
  ShExecInfo.hInstApp = NULL; 
  if (! ope.empty() && ope != "") ShExecInfo.lpVerb=ope.c_str();
  if (! param.empty() && param != "") ShExecInfo.lpParameters=param.c_str();
  if (! dir.empty() && dir != "") ShExecInfo.lpDirectory=dir.c_str();
  ShellExecuteEx(&ShExecInfo);
  WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
  CloseHandle(ShExecInfo.hProcess);
// if (!ShellExecuteEx(&ShExecInfo))
//      return StrError("ShellExecute error with file %s, parameters %s, directory %s\n", lpFile, lpParameters, lpDirectory);
#else
  pid_t pid=fork();
  if (pid == 0) execl(cmd.c_str(), param.c_str(), NULL);
  if (pid == -1) return strerror(errno);
#endif
  return "";
}

std::string exec_cmd(std::string cmd)
{
  std::string s;
#ifdef _WIN32
  std::wstring wcmd = s2ws("/C " + cmd);
  // std::wstring wcmd = s2ws(cmd);
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

  s = file2str(tmpFile);
  CloseHandle(hFile);
  DeleteFile(tmpFile.c_str());
#else
  std::string tf = tempfile();
  std::string fullcmd = cmd + " > " + tf;
  std::system(fullcmd.c_str());
  s = file2str(tf);
  std::filesystem::remove(tf);
#endif
  return to_htent(s);
}

std::string rep_bs(std::string &s)
{
  std::string bs;
  bs = (char)92;
  replace_all(s, bs, "##BACKSLASH_CODE##");
  s=rep_crlf(s);
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
