
#include <cstdio>
#include <curl/curl.h>
#include <iostream>

size_t write_cb(char *contents, size_t size, size_t nmemb, void *userp)
{
  //  std::cout << '[' << (char *)contents << ']' << std::endl;
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

int main(int argc, char *argv[])
{
  CURLcode ret;
  CURL *curl;

  std::string id = "", psw, url = "", buf;
  psw = "";

  if (argc == 2)
  {
    url = argv[1];
  }
  else if (argc == 4)
  {
    id = argv[1];
    psw = argv[2];
    url = argv[3];
  }
  else
  {
    // url="https://wiki.space.thales/rest/api/content?limit=2";
    url = "https://wiki.space.thales/rest/api/content/"
          "search\?cql=contributor+in+(alkadea,arnones,capous,cavallc,chaumia1,fresnew,guyonnt,kouachb,lalannd2,leleut,"
          "moninn,monnete,nottea,thurona,tourel,xsii077,xsii076)+and+space+=+orchestra+and+lastmodified+=+2023-11-07&"
          "limit=1000";
  }

  std::cout << "id: " << id << ", url: " << url << std::endl;

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  if (id != "" && psw != "")
  {
    //  curl_easy_setopt(curl, CURLOPT_USERPWD, (id+':'+psw).c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, id.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, psw.c_str());
  }
  curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

  //  curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.5.0");
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
  ret = curl_easy_perform(curl);
  if (ret != CURLE_OK)
    buf += std::string("curl_easy_perform() failed: ") + curl_easy_strerror(ret);

  curl_easy_cleanup(curl);
  curl = NULL;

  std::cout << buf << std::endl;
  return (int)ret;
}
/**** End of sample code ****/
