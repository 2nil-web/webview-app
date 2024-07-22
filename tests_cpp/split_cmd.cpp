
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <fstream>

const std::string ws = " \t\n\r\f\v";
std::string trim(std::string &s, std::string delims=ws)
{
  s.erase(0, s.find_first_not_of(delims));
  s.erase(s.find_last_not_of(delims) + 1);
  return s;
}

void split_cmd(const std::string& s, std::string* prog, std::string* params)
{
  std::string cmd(s);
  trim(cmd);
  size_t end_exe=0;

  if(cmd[0] == '\"') {
    end_exe=cmd.find_first_of('\"', 1);
    if(std::string::npos != end_exe) {
      *prog=cmd.substr(1, end_exe - 1);
      *params=cmd.substr(end_exe + 1);
    } else {
      *prog=cmd.substr(1, end_exe);
      *params="";
    }
  } else {
    do {
      end_exe=cmd.find_first_of(' ', end_exe);
      if (end_exe > 0 && cmd[end_exe-1] == '\\') end_exe++;
      else break;
    } while (end_exe != std::string::npos);

    *prog=cmd.substr(0, end_exe);
    if(std::string::npos != end_exe) {
      *params=cmd.substr(end_exe + 1);
    } else {
      *params="";
    }
  }
}

int main(int argc, char **argv) {
  std::string x, p;
  if (argc == 2) {
    split_cmd(argv[1], &x, &p);
    std::cout << "prog: " << x << ", params: " << p << std::endl;
  }
}

