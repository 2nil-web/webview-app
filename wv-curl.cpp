
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
