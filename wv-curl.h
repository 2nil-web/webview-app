
#ifndef WV_CURL_H
#define WV_CURL_H

#include <curl/curl.h>

std::string httpget(std::string url, bool peer_check = false, bool host_check = false, bool verbose = false);
std::string httpget_cred(std::string id, std::string pass, std::string url, bool peer_check = false,
                         bool host_check = false, bool verbose = false);

int wiki_curl();

#endif /* WV_CURL_H */
