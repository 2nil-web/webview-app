
#include <cstdio>
#include <iostream>

#include "wv-curl.h"


////////// Voir rest_get dans ghrepo.cpp

size_t write_cb(char *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

std::string _id="", _pswd="";
std::string httpget(std::string url)
{
  CURL *curl;
  CURLcode res;
  std::string buf = "";

  curl_global_init(CURL_GLOBAL_DEFAULT);
  
  curl = curl_easy_init();
  if (curl)
  {
    struct curl_slist *headers=nullptr;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "webview-app/1.0");
    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    // If user and password are provided then use authentication
    if (_id != "") {
      if (_pswd != "") {
        curl_easy_setopt(curl, CURLOPT_USERNAME, _id.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, _pswd.c_str());
      } else {
        headers=curl_slist_append(headers, _id.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_slist_free_all(headers);
      }
    }

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK) {
      std::string err_msg("curl_easy_perform() failed: ");
      err_msg+=curl_easy_strerror(res);
      std::cout << err_msg << std::endl;
      buf += err_msg;
    }

    /* always cleanup */
    if (headers) curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  return buf;
}

void httpcred(std::string id, std::string pass) {
  _id=id;
  _pswd=pass;
}

#ifdef CURLEASY_CLASS
#define SetOptOrBreak(opt, param) if (!SetOpt(opt, param)) { _LastErrorMsg=#opt; return ; }
#define SetOptOrDie(opt, param)   if (!SetOpt(opt, param)) { _LastErrorMsg=#opt; return false; }
CurlEasy::CurlEasy(bool defaultSetup)
{
  if ((ret_code=curl_global_init(CURL_GLOBAL_DEFAULT)) != CURLE_OK) return;

  handle = curl_easy_init();

  if (handle) {
    SetOptOrBreak(CURLOPT_USERAGENT, "webview-app/1.0");
    SetOptOrBreak(CURLOPT_VERBOSE, 1L);
    SetOptOrBreak(CURLOPT_SSL_VERIFYPEER, 0L);
    SetOptOrBreak(CURLOPT_SSL_VERIFYHOST, 0L);
    SetOptOrBreak(CURLOPT_CA_CACHE_TIMEOUT, 604800L);
    SetOptOrBreak(CURLOPT_WRITEFUNCTION, write_cb);
    SetOptOrBreak(CURLOPT_WRITEDATA, &buffer);
  }
}

CurlEasy::~CurlEasy() {
  curl_easy_cleanup(handle);
  curl_global_cleanup();
}

// If return empty string then might be useful to call LastErrorMsg()
std::string CurlEasy::Perform(std::string url) {
  buffer = "";

  if (handle) {
    if (!SetOpt(CURLOPT_URL, url.c_str())) { _LastErrorMsg="CURLOPT_URL"; return ""; }
    /* Perform the request, ret_code will get the return code */
    ret_code = curl_easy_perform(handle);
    /* Check for errors */
    if (ret_code != CURLE_OK) {
      _LastErrorMsg="Perform failed";
      //std::cerr << _LastErrorMsg << " : " << curl_easy_strerror(ret_code) << std::endl;
    }
  }

  return buffer;
}

std::string CurlEasy::LastErrorMsg() {
  std::string s="";
  if (_LastErrorMsg != "") s=_LastErrorMsg+':';
  if (_LastErrorMsg != "" && ret_code != CURLE_OK) s+=':';
  if (ret_code != CURLE_OK) s+=curl_easy_strerror(ret_code);
  return s;
}

// If return false then might be useful to call LastErrorMsg()
bool CurlEasy::SetOpt(CURLoption curl_opt, ...) {
  if (handle) {
    va_list ap;
    va_start(ap, curl_opt);
    ret_code=curl_easy_setopt(handle, curl_opt, ap);
    va_end(ap);
  } else return false;

  if (ret_code != CURLE_OK) {
    _LastErrorMsg="SetOpt failed";
  } else _LastErrorMsg="";

  return (ret_code == CURLE_OK);
}


// If return false then might be useful to call LastErrorMsg()
bool CurlEasy::SetCredential(std::string user, std::string password) {
  if (user == "") {
    _LastErrorMsg="Missing user id or token when setting credential";
    return false;
  }

  // If user and password are provided then use theml for authentication
  if (password != "") {
    SetOptOrDie(CURLOPT_USERNAME, user.c_str());
    SetOptOrDie(CURLOPT_PASSWORD, password.c_str());
  } else {
    struct curl_slist *headers= NULL;
    headers=curl_slist_append(headers, password.c_str());
    SetOptOrDie(CURLOPT_HTTPHEADER, headers);
    curl_slist_free_all(headers);
  }

  return true;
}

CurlEasy *cq=nullptr;

std::string httpget_new(std::string url, bool defSetup)
{
  if (cq == nullptr) cq=new CurlEasy(defSetup);
  auto s=cq->Perform(url);
  if (s != "") return s;
  return cq->LastErrorMsg();
}

std::string httpcred_new(std::string id, std::string pass) {
  if (cq == nullptr) cq=new CurlEasy();
  if (cq->SetCredential(id, pass)) return "true";
  return cq->LastErrorMsg();
}
#endif // CURLEASY_CLASS

