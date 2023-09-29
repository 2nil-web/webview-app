
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <cstdio>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <clocale>
#include <locale>
#endif
#include <filesystem>
#include <fstream>


template <typename TP>
std::time_t to_time_t(TP tp) {
  namespace ch=std::chrono;
  auto sctp=ch::time_point_cast<ch::system_clock::duration>(tp-TP::clock::now()+ch::system_clock::now());
  return ch::system_clock::to_time_t(sctp);
}

// Convert a file time to a string, default format is ISO8601 and default time zone is local
std::string file_time_to_string(std::filesystem::file_time_type file_time, std::string fmt="%Y-%m-%dT%H:%M:%S", bool gm=false) {
  std::time_t tt=to_time_t(file_time);
  std::tm *tim;
  if (gm) tim=std::gmtime(&tt);
  else tim=std::localtime(&tt);
  std::stringstream buffer;
  buffer << std::put_time(tim, fmt.c_str());
  std::string fmtime=buffer.str();

  return fmtime;
}

int main(int argc, char **argv) {
#ifdef _WIN32
    _setmode(_fileno(stderr), _O_WTEXT);
#else
    std::setlocale(LC_ALL, "");
    std::locale::global(std::locale(""));
#endif

  if (argc == 2) {
    std::cout << "argv " << argv[1] << std::endl << std::flush;
    auto abs_path=std::filesystem::absolute(argv[1]).string();
    std::cout << "path " << abs_path << std::endl;
    std::filesystem::file_time_type file_time=std::filesystem::last_write_time(abs_path);
    std::cout << abs_path << " : " << file_time_to_string(file_time) << std::endl;
  }

  return 0;
}
