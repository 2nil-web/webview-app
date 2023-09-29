
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <filesystem>
 

std::string ws2s(std::wstring ws) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(ws);
}

std::wstring s2ws(std::string s) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(s);
}


template <typename TP>
std::time_t to_time_t(TP tp) {
  namespace ch=std::chrono;
  auto sctp=ch::time_point_cast<ch::system_clock::duration>(tp-TP::clock::now()+ch::system_clock::now());
  return ch::system_clock::to_time_t(sctp);
}

#ifdef _WIN32
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
    std::string ret=(fs.type() == std::filesystem::file_type::directory) ? "d":" ";
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
};

struct path_list {
  std::filesystem::path path;
  uintmax_t biggest_size;
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
  return pe;
}

path_list get_path_list(std::filesystem::path _path, bool recursive=false) {
  if (_path.empty()) _path=".";
  path_list pl;
  pl.path=std::filesystem::absolute(_path);
  pl.biggest_size=0;
  path_elem pe;

  for (const auto& e:std::filesystem::directory_iterator(pl.path)) {
    pe=get_path_elem(e);
    if (pe.size > pl.biggest_size) pl.biggest_size=pe.size;
    pl.list.push_back(pe);
  }

  return pl;
}

void display_path_list(path_list pl) {
  //std::cout << pl.path.string() << std::endl;
  std::cout << std::filesystem::canonical(pl.path) << std::endl;

  int rj=std::to_string(pl.biggest_size).size();
  for (auto& pe:pl.list) {
    std::cout << pe.perms << ' ' << std::setw(rj) << pe.size << ' ' << file_time_to_string(pe.last_write) << ' ' << std::filesystem::canonical(pe.name).generic_string().substr(pl.path.string().size()+1) << std::endl;
  }

}

void fstat(std::filesystem::path p, size_t path_l) {
  auto fs=std::filesystem::status(p);
  auto ft=fs.type();
  std::uintmax_t sz;
  if (ft == std::filesystem::file_type::regular) sz=std::filesystem::file_size(p);
  else sz=static_cast<std::uintmax_t>(-1);
  std::string lastwr="****-**-**T**:**:**";
  if (ft != std::filesystem::file_type::not_found) lastwr=lastwrite(p);

  // Perms, size, last write, name
  std::cout << show_status(fs) << ' ' << std::setw(12);
  if (ft != std::filesystem::file_type::directory) std::cout << sz;
  else std::cout << ' ';
  std::cout << ' ' << lastwr << ' ';
  std::wcout << replace_all(p.wstring().substr(path_l+1), L"\\", L"/");
  std::cout << std::endl;
}

void lsdir(std::string _path, bool recursive=false) {
  if (_path.empty()) _path=".";
  auto path=std::filesystem::absolute(_path);

/* Pas de récursif, trop dangereux, ça plante le PC ... Voir éventuellement faire un try/catch
  if (recursive) 
    for (const auto& e:std::filesystem::recursive_directory_iterator(path))
      std::cout << +e.path().string() << std::endl;;
  else*/
  std::cout << replace_all(path.string(), "\\", "/") << ':' << std::endl;
    for (const auto& e:std::filesystem::directory_iterator(path)) {
      fstat(e, path.string().size());
    }

}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "C");

  if (argc == 2) display_path_list(get_path_list(argv[1]));
  else display_path_list(get_path_list("."));
  return 0;
}
