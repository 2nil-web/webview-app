
#ifndef UTIL_H
#define UTIL_H

#include <functional>

#ifdef _WIN32
#include <Windows.h>
void WinError(const char *fmt, ...);
#ifdef UNICODE
#define CommandLineToArgv CommandLineToArgvW
#else
PCHAR *CommandLineToArgvA(PCHAR CmdLine, int *_argc);
#define CommandLineToArgv CommandLineToArgvA
#endif
#endif

// Convert wstring to string
std::string ws2s(std::wstring ws);
// Convert string to wstring
std::wstring s2ws(std::string s);

// Convert an utf8 string into an url encoded hexadecimal one
std::string h2s(const std::string s);
// Convert an url hexadecimal encoded string into an utf8 string
std::string s2h(const std::string s);

std::string file2str(std::string filename);

std::string trim(std::string &s);
bool any_of_ctype(const std::string, std::function<int(int)>);
bool all_of_ctype(const std::string, std::function<int(int)>);

std::string temppath();
std::string tempfile(std::string tpath = "", std::string pfx = "");
std::string exec_cmd(std::string cmd);
void replace_all(std::string &s, std::string srch, std::string repl);
void rep_crlf(std::string &s);
std::string rep_bs(std::string &s);

// Filesystem api exposed to javascript
std::string pwd();
std::string cwd(std::string new_dir = "");

#endif /* UTIL_H */
