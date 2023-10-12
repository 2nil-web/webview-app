

#ifdef __unix__
#include <unistd.h>
#include <sys/types.h>
#include <linux/limits.h>
#elif defined(_WIN32) || defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include <tchar.h>
#include <stdio.h>
#endif

#include <getopt.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <regex>
#include <initializer_list>
#include <vector>
#include <locale>
#include <json/json.h>
#include <curl/curl.h>

enum verbose_mode { SILENT, DEFAULT, VERBOSE };
verbose_mode verbosity=DEFAULT;

FILE *my_fopen(std::string filename, std::string mode) {
#ifdef _WIN32
  FILE *fp;
  fopen_s(&fp, filename.c_str(), mode.c_str());
  return fp;
#else
  return fopen(filename.c_str(), mode.c_str());
#endif
}

bool curl_download(std::string url, std::string dir, std::string id, std::string passwd) {
  CURL *curl=curl_easy_init();

  std::string url_pop=url;
  if (url_pop.back() == '/') url_pop.pop_back();
  std::string filename=std::filesystem::path(url_pop).filename().string();

  if (dir != "") filename=std::filesystem::path(dir+'/'+filename).make_preferred().string();


  if (curl) {
    struct curl_slist *headers= NULL;

    FILE *fp=my_fopen(filename, "wb");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    //curl_easy_setopt(curl, CURLOPT_PASSWORD, std::string(id+':'+passwd).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
#ifdef __GNUC__
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
#endif

    if (id != "") curl_easy_setopt(curl, CURLOPT_USERNAME, id.c_str());
    if (passwd != "") {
      if (id != "") curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd.c_str());
      else {
        headers=curl_slist_append(headers, passwd.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      }
    }

    switch (verbosity) {
      case SILENT:
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        break;
      case VERBOSE:
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        break;
      default:
        break;
    }

    CURLcode code=curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);

    if (code == CURLE_OK) {
      std::cout << "Download " << url << " to " << filename << " OK." << std::endl;
      return true;
    }

    if (headers) curl_slist_free_all(headers);
  }

  std::cout << "Problem with download of " << url << '.' << std::endl;
  return false;
}

