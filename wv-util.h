
#ifndef UTIL_H
#define UTIL_H

#include <functional>

#ifdef _WIN32
void WinError(const char *fmt, ...);
#ifdef UNICODE
#define CommandLineToArgv CommandLineToArgvW
#else
PCHAR* CommandLineToArgvA( PCHAR CmdLine, int* _argc);
#define CommandLineToArgv CommandLineToArgvA
#endif
#endif

std::string trim(std::string& s);
bool any_of_ctype(const std::string, std::function<int(int)>);
bool all_of_ctype(const std::string, std::function<int(int)>);

std::string temppath();
std::string tempfile(std::string tpath="", std::string pfx="");
std::string exec_cmd(std::string cmd);
void replace_all(std::string &s, std::string srch, std::string repl);
void rep_crlf(std::string &s);
std::string rep_bs(std::string &s);

// Filesystem api exposed to javascript
std::string pwd();
std::string cwd(std::string new_dir="");

#endif /* UTIL_H */

