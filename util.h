
#ifndef UTIL_H
#define UTIL_H

std::string temppath();
std::string tempfile(std::string tpath="", std::string pfx="");
std::string exec_cmd(std::string cmd);
void replace_all(std::string &s, std::string srch, std::string repl);
void rep_crlf(std::string &s);

#endif /* UTIL_H */

