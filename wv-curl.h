
#ifndef WV_CURL_H
#define WV_CURL_H

#include <curl/curl.h>

std::string httpget(std::string url, bool peer_check = false, bool host_check = false, bool verbose = false);
std::string httpget_cred(std::string id, std::string pass, std::string url, bool peer_check, bool host_check, bool verbose);

#ifdef CURLEASY_CLASS
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
};
#endif // CLASS

#endif /* WV_CURL_H */
