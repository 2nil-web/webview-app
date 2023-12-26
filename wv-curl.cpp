
#include <cstdio>
#include <curl/curl.h>
#include <iostream>

size_t write_cb(char *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

std::string httpget(std::string url, bool peer_check, bool host_check, bool verbose)
{
  CURL *curl;
  CURLcode res;
  std::string buf = "";

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if (curl)
  {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    if (verbose)
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "webview-app/1.0");

    // Does or does not check remote host certificate
    if (!peer_check)
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    // Does or does not check remote hostname accordance to its certificate
    if (!host_check)
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    /* cache the CA cert bundle in memory for a week */
    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);

    // Send result of command into a string buffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    // if (res != CURLE_OK) std::cerr << "curl_easy_perform() failed: " <<
    // curl_easy_strerror(res) << std::endl;
    if (res != CURLE_OK)
      buf += std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res);

    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return buf;
}


std::string httpget_cred(std::string id, std::string pass, std::string url, bool peer_check, bool host_check, bool verbose)
{
  CURL *curl;
  CURLcode res;
  std::string buf = "";

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

  if (verbose)
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

  curl_easy_setopt(curl, CURLOPT_USERAGENT, "webview-app/1.0");
  curl_easy_setopt(curl, CURLOPT_USERNAME, id.c_str());
  curl_easy_setopt(curl, CURLOPT_PASSWORD, pass.c_str());
  // Does or does not check remote host certificate
  if (!peer_check)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

  // Does or does not check remote hostname accordance to its certificate
  if (!host_check)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  /* cache the CA cert bundle in memory for a week */
  //curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);

  // Send result of command into a string buffer
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
  /* Perform the request, res will get the return code */
  res = curl_easy_perform(curl);
  /* Check for errors */
  // if (res != CURLE_OK) std::cerr << "curl_easy_perform() failed: " <<
  // curl_easy_strerror(res) << std::endl;
  if (res != CURLE_OK) {
    std::cout << "id : " << id << std::endl;
    std::cout << "pass : " << pass << std::endl;
    std::cout << "url : " << url << std::endl;
    buf += std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res);
  }

  /* always cleanup */
  curl_easy_cleanup(curl);
  curl=NULL;

  curl_global_cleanup();

  return buf;
}

int wiki_curl()
{
  CURLcode ret;
  CURL *curl;

  std::string id="lalannd2", psw="ocvdBum12$*4", url="https://wiki.space.thales/rest/api/content/search\?cql=contributor+in+(alkadea,arnones,capous,cavallc,chaumia1,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,thurona,tourel,xsii077,xsii076)+and+space+=+orchestra+and+lastmodified+=+2023-11-07&limit=1000", buf;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//    curl_easy_setopt(curl, CURLOPT_USERNAME, id.c_str());
//    curl_easy_setopt(curl, CURLOPT_PASSWORD, psw.c_str());
  curl_easy_setopt(curl, CURLOPT_USERPWD, (id+':'+psw).c_str());
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.5.0");
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);


  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
  ret = curl_easy_perform(curl);
  if (ret != CURLE_OK)
    buf += std::string("curl_easy_perform() failed: ") + curl_easy_strerror(ret);

  curl_easy_cleanup(curl);
  curl = NULL;
  curl_global_cleanup();

  std::cout << buf << std::endl;
  return (int)ret;
}
