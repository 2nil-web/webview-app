
#ifndef UTIL_H
#define UTIL_H

#include <bitset>
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

// Convert non ascii characters of a wstring to html entities in the following decimal form &#[dec_value];
std::string to_htent(const std::wstring ws, bool dec_base = true);
// Same as previous for string
std::string to_htent(const std::string s, bool dec_base = true);

// Convert the html entities in hexa or decimal form contained in a string to their wchar_t value, return the obtained
// wstring
std::wstring from_htent(const std::string htent, std::wstring &ws);
// Same as previous but return a string
std::string from_htent(const std::string htent, std::string &s);

std::string file2str(std::string filename);
std::string file2str(std::wstring wfilename);
std::string wfile2str(std::wstring wfilename);
std::string wfile2str(std::string filename);

std::string fread(std::string filename);
std::string fread(std::wstring wfilename);

std::string trim(std::string &s);
bool any_of_ctype(const std::string, std::function<int(int)>);
bool all_of_ctype(const std::string, std::function<int(int)>);

std::string temppath();
std::string tempfile(std::string tpath = "", std::string pfx = "");
std::string exec_cmd(std::string cmd);
void replace_all(std::string &s, std::string srch, std::string repl);
void replace_all(std::wstring &s, std::wstring srch, std::wstring repl);
void rep_crlf(std::string &s);
std::string rep_bs(std::string &s);

// Filesystem api exposed to javascript
std::string pwd();
std::string cwd(std::string new_dir = "");

#endif /* UTIL_H */
