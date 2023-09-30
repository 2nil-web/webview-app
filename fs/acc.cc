
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <regex>

#include "../base64.hpp"
 
template <typename TP>
std::time_t to_time_t(TP tp) {
  namespace ch=std::chrono;
  auto sctp=ch::time_point_cast<ch::system_clock::duration>(tp-TP::clock::now()+ch::system_clock::now());
  return ch::system_clock::to_time_t(sctp);
}

#ifdef _WIN32
#include <windows.h>
#define my_gmtime(a,b) gmtime_s(b,a)
#define my_localtime(a,b) localtime_s(b,a)
#else
#define my_gmtime(a,b) std::gmtime_r(a,b)
#define my_localtime(a,b) std::localtime_r(a,b)
#endif

// Convert a file time to a string, default format is ISO8601 and default time zone is local
std::string file_time_to_string(std::filesystem::file_time_type file_time, std::string fmt="%Y-%m-%d %H:%M:%S", bool gm=false) {
  std::time_t tt=to_time_t(file_time);
  std::tm tim;
  if (gm) my_gmtime(&tt, &tim);
  else my_localtime(&tt, &tim);
  std::stringstream buffer;
  buffer << std::put_time(&tim, fmt.c_str());
  std::string fmtime=buffer.str();

  return fmtime;
}

std::string lastwrite(std::filesystem::path p) {
  return file_time_to_string(std::filesystem::last_write_time(p));
}

std::string show_status(std::filesystem::file_status fs) {
    using std::filesystem::perms;
    auto show = [=](char op, perms perm) { return (perms::none == (perm & fs.permissions()) ? '-' : op); };
    std::string ret=(fs.type() == std::filesystem::file_type::directory) ? "d":"-";
    ret+=show('r', perms::owner_read);
    ret+=show('w', perms::owner_write);
    ret+=show('x', perms::owner_exec);
    ret+=show('r', perms::group_read);
    ret+=show('w', perms::group_write);
    ret+=show('x', perms::group_exec);
    ret+=show('r', perms::others_read);
    ret+=show('w', perms::others_write);
    ret+=show('x', perms::others_exec);
    return ret;
}


std::string replace_all(std::string _s, std::string srch, std::string repl) {
  size_t pos=0;
  std::string s=_s;

  while (pos += repl.length()) {
    pos=s.find(srch, pos);
    if (pos == std::string::npos) break;
    s.replace(pos, srch.length(), repl);
  }

  return s;
}

std::wstring replace_all(std::wstring _s, std::wstring srch, std::wstring repl) {
  size_t pos=0;
  std::wstring s=_s;

  while (pos += repl.length()) {
    pos=s.find(srch, pos);
    if (pos == std::wstring::npos) break;
    s.replace(pos, srch.length(), repl);
  }

  return s;
}

struct path_elem {
  std::string perms;
  uintmax_t size;
  std::filesystem::file_time_type last_write;
  std::filesystem::path name;
  std::string b64_path_name;
};

struct path_list {
  std::filesystem::path path;
  std::string b64_path_name;
  uintmax_t biggest_size;
  size_t biggest_name_size;
  std::vector<path_elem> list;
};

path_elem get_path_elem(std::filesystem::path p) {
  path_elem pe;
  auto fs=std::filesystem::status(p);
  pe.perms=show_status(fs);
  if (fs.type() != std::filesystem::file_type::directory) pe.size=std::filesystem::file_size(p);
  else pe.size=0;
  pe.last_write=std::filesystem::last_write_time(p);
  pe.name=p;
  pe.b64_path_name=base64::to_base64(pe.name.string().c_str());
  return pe;
}

void add_path_elem(path_list& pl, std::filesystem::path p) {
  auto pe=get_path_elem(p);
  if (pe.size > pl.biggest_size) pl.biggest_size=pe.size;
  auto bns=pe.name.wstring().size();
  if (bns > pl.biggest_name_size) pl.biggest_name_size=bns;
  pl.list.push_back(pe);
}

path_list get_path_list(std::filesystem::path _path, bool recursive=false) {
  if (_path.empty()) _path=".";
  path_list pl;
  pl.path=std::filesystem::absolute(_path);
  pl.b64_path_name=base64::to_base64(pl.path.string().c_str());
  pl.biggest_size=0;
  pl.biggest_name_size=0;
  path_elem pe;

  auto fs=std::filesystem::status(pl.path);

  if (fs.type() == std::filesystem::file_type::directory) {
    for (const auto& e:std::filesystem::directory_iterator(pl.path)) {
      add_path_elem(pl, e);
    }
  } else {
    add_path_elem(pl, pl.path);
    pl.path=pl.path.parent_path();
  }

  return pl;
}

void display_path_list(path_list pl) {
  std::cout << replace_all(std::filesystem::canonical(pl.path).string(), "\\", "/") << std::endl;

  int parent_path_size=pl.path.string().size();
  int rj=std::to_string(pl.biggest_size).size();

  for (auto& pe:pl.list) {
    std::cout << pe.perms << ' ' << std::setw(rj) << pe.size << ' ' << file_time_to_string(pe.last_write) << ' ';
    std::cout << std::filesystem::canonical(pe.name).generic_string().substr(parent_path_size+1) << ' ';
    std::cout << std::string(pl.biggest_name_size-pe.name.wstring().size(), ' ');
    std::cout << pe.b64_path_name;
    std::cout << std::endl;
  }
}

bool isBase64(const std::string s) {
  if (s.length()%4 == 0) return std::regex_match(s, std::regex("^[A-Za-z0-9+/]*={0,2}$"));
  return false;
}

int main(int argc, char **argv) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif

  if (argc == 2) {
    if (isBase64(argv[1])) {
      display_path_list(get_path_list(base64::from_base64(argv[1])));
    } else display_path_list(get_path_list(argv[1]));
  } else display_path_list(get_path_list("."));
  return 0;
}

