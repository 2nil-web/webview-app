
#ifndef UTIL_H
#define UTIL_H

#include <bitset>
#include <functional>
#include <string>
#include <vector>

bool my_setenv(const std::string var, std::string val);
std::string my_getenv(const std::string var);


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

// Split string into vector of string using delim to split
std::vector<std::string> split(const std::string &str, char delim);

bool str2bool(std::string s);

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
std::string shell_cmd(std::string cmd, std::string param="", std::string dir="", std::string ope="");
std::string shell_cmd_wait(std::string cmd, std::string param="", std::string dir="", std::string ope="");
void replace_all(std::string &s, std::string srch, std::string repl);
void replace_all(std::wstring &s, std::wstring srch, std::wstring repl);
std::string rep_crlf(std::string s);
std::string rep_bs(std::string &s);

// Filesystem api exposed to javascript
std::string pwd();
std::string cwd(std::string new_dir = "");

#endif /* UTIL_H */
