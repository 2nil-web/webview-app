
#ifndef CURLEASY_H
#define CURLEASY_H

#include <curl/curl.h>

class CurlEasy
{
public:
  CurlEasy(bool defaultSetup = true);
  ~CurlEasy();
  bool SetOpt(CURLoption, ...);
  std::string Perform(std::string url);
  bool SetCredential(std::string id_or_token, std::string password = "");
  std::string LastErrorMsg();

private:
  CURL *handle;
  CURLcode ret_code;
  std::string buffer, _LastErrorMsg;
  static size_t write_callback(char *contents, size_t size, size_t nmemb, void *userp);
};

#endif // CURLEASY_H
