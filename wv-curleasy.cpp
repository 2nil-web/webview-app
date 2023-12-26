
#include "wv-curleasy.h"
#include <cstdio>
#include <iostream>

size_t CurlEasy::write_callback(char *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

#define SetOptOrBreak(opt, param)                                                                                      \
  if (!SetOpt(opt, param))                                                                                             \
  {                                                                                                                    \
    _LastErrorMsg = #opt;                                                                                              \
    return;                                                                                                            \
  }
#define SetOptOrDie(opt, param)                                                                                        \
  if (!SetOpt(opt, param))                                                                                             \
  {                                                                                                                    \
    _LastErrorMsg = #opt;                                                                                              \
    return false;                                                                                                      \
  }
CurlEasy::CurlEasy(bool defaultSetup)
{
  if ((ret_code = curl_global_init(CURL_GLOBAL_DEFAULT)) != CURLE_OK)
    return;

  handle = curl_easy_init();

  if (handle)
  {
    SetOptOrBreak(CURLOPT_USERAGENT, "webview-app/1.0");
    SetOptOrBreak(CURLOPT_VERBOSE, 1L);
    SetOptOrBreak(CURLOPT_SSL_VERIFYPEER, 0L);
    SetOptOrBreak(CURLOPT_SSL_VERIFYHOST, 0L);
    SetOptOrBreak(CURLOPT_CA_CACHE_TIMEOUT, 604800L);
    SetOptOrBreak(CURLOPT_WRITEFUNCTION, write_callback);
    SetOptOrBreak(CURLOPT_WRITEDATA, &buffer);
  }
}

CurlEasy::~CurlEasy()
{
  curl_easy_cleanup(handle);
  curl_global_cleanup();
}

// If return empty string then might be useful to call LastErrorMsg()
std::string CurlEasy::Perform(std::string url)
{
  buffer = "";

  if (handle)
  {
    if (!SetOpt(CURLOPT_URL, url.c_str()))
    {
      _LastErrorMsg = "CURLOPT_URL";
      return "";
    }
    /* Perform the request, ret_code will get the return code */
    ret_code = curl_easy_perform(handle);
    /* Check for errors */
    if (ret_code != CURLE_OK)
    {
      _LastErrorMsg = "Perform failed";
      // std::cerr << _LastErrorMsg << " : " << curl_easy_strerror(ret_code) << std::endl;
    }
  }

  return buffer;
}

std::string CurlEasy::LastErrorMsg()
{
  std::string s = "";
  if (_LastErrorMsg != "")
    s = _LastErrorMsg + ':';
  if (_LastErrorMsg != "" && ret_code != CURLE_OK)
    s += ':';
  if (ret_code != CURLE_OK)
    s += curl_easy_strerror(ret_code);
  return s;
}

// If return false then might be useful to call LastErrorMsg()
bool CurlEasy::SetOpt(CURLoption curl_opt, ...)
{
  if (handle)
  {
    va_list ap;
    va_start(ap, curl_opt);
    ret_code = curl_easy_setopt(handle, curl_opt, ap);
    va_end(ap);
  }
  else
    return false;

  if (ret_code != CURLE_OK)
  {
    _LastErrorMsg = "SetOpt failed";
  }
  else
    _LastErrorMsg = "";

  return (ret_code == CURLE_OK);
}

// If return false then might be useful to call LastErrorMsg()
bool CurlEasy::SetCredential(std::string user, std::string password)
{
  if (user == "")
  {
    _LastErrorMsg = "Missing user id or token when setting credential";
    return false;
  }

  // If user and password are provided then use theml for authentication
  if (password != "")
  {
    SetOptOrDie(CURLOPT_USERNAME, user.c_str());
    SetOptOrDie(CURLOPT_PASSWORD, password.c_str());
  }
  else
  {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, password.c_str());
    SetOptOrDie(CURLOPT_HTTPHEADER, headers);
    curl_slist_free_all(headers);
  }

  return true;
}

CurlEasy *cq = nullptr;

std::string httpget_new(std::string url, bool defSetup)
{
  if (cq == nullptr)
    cq = new CurlEasy(defSetup);
  auto s = cq->Perform(url);
  if (s != "")
    return s;
  return cq->LastErrorMsg();
}

std::string httpcred_new(std::string id, std::string pass)
{
  if (cq == nullptr)
    cq = new CurlEasy();
  if (cq->SetCredential(id, pass))
    return "true";
  return cq->LastErrorMsg();
}
