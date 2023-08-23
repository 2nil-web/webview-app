
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <string.h>
#endif

#include <iostream>
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <filesystem>
#endif

std::string ws2s(const std::wstring& wstr) {
  using ctX = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<ctX, wchar_t> cX;
  return cX.to_bytes(wstr);
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

