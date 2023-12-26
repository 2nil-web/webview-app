
#ifdef __unix__
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#elif defined(_WIN32) || defined(WIN32)
#include <io.h>
#include <shlobj.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <winsock2.h>
#endif

#include <algorithm>
#include <cctype>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <initializer_list>
#include <iostream>
#include <locale>
#include <regex>
#include <string>
#include <vector>

#ifdef __GNUC__
#define MIN std::min
#endif

#ifdef _MSC_VER
#define MIN min
#endif

enum verbose_mode
{
  SILENT,
  DEFAULT,
  VERBOSE
};
verbose_mode verbosity = DEFAULT;

std::string app_name = "rest", app_vers = "0.1", proxy = "", user_agent = app_name + "/" + app_vers, server_url = "";

#define my_curl_setopt(curl_opt, opt_param)                                                                            \
  if ((code = curl_easy_setopt(curl, curl_opt, opt_param)) != CURLE_OK)                                                \
    return false;

size_t writer(char *data, size_t size, size_t nmemb, std::string *writerData)
{
  if (writerData == NULL)
    return 0;
  writerData->append(data, size * nmemb);
  return size * nmemb;
}

bool curl_init(CURL *&curl, std::string url, std::string &buffer, char errorBuffer[CURL_ERROR_SIZE])
{
  if ((curl = curl_easy_init()) == NULL)
  {
    std::cerr << "Failed to create CURL connection" << std::endl;
    return false;
  }

  CURLcode code;

  my_curl_setopt(CURLOPT_ERRORBUFFER, errorBuffer);
  my_curl_setopt(CURLOPT_USERAGENT, user_agent.c_str());
  my_curl_setopt(CURLOPT_URL, url.c_str());
  my_curl_setopt(CURLOPT_WRITEFUNCTION, writer);
  my_curl_setopt(CURLOPT_WRITEDATA, &buffer);

  //  my_curl_setopt(CURLOPT_SSL_VERIFYPEER, 0L);
  //  my_curl_setopt(CURLOPT_SSL_VERIFYHOST, 0L);

  if (proxy != "")
    my_curl_setopt(CURLOPT_PROXY, proxy.c_str());
  my_curl_setopt(CURLOPT_FOLLOWLOCATION, 1L);

  switch (verbosity)
  {
  case SILENT:
    my_curl_setopt(CURLOPT_NOPROGRESS, 1L);
    my_curl_setopt(CURLOPT_VERBOSE, 0L);
    break;
  case VERBOSE:
    my_curl_setopt(CURLOPT_NOPROGRESS, 0L);
    my_curl_setopt(CURLOPT_VERBOSE, 1L);
    break;
  default:
    break;
  }

  return true;
}

static int nget = 0;

bool get_url_response(std::string url, std::string &buffer)
{
  CURL *curl = NULL;
  char errorBuffer[CURL_ERROR_SIZE] = "";
  curl_global_init(CURL_GLOBAL_ALL);

  // Initialize CURL connection
  if (!curl_init(curl, url, buffer, errorBuffer))
  {
    std::cerr << "Connection initialisation failed " << errorBuffer << std::endl;
    return false;
  }

  // Retrieve content for the URL
  CURLcode code = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  nget++;

  if (code != CURLE_OK)
  {
    std::cerr << "Failed to get " << url << " [" << errorBuffer << "]" << std::endl;
    return false;
    ;
  }

  return true;
}

FILE *my_fopen(std::string filename, std::string mode)
{
#ifdef _WIN32
  FILE *fp;
  fopen_s(&fp, filename.c_str(), mode.c_str());
  return fp;
#else
  return fopen(filename.c_str(), mode.c_str());
#endif
}

bool curl_download(std::string url, std::string dir, std::string id, std::string passwd)
{
  CURL *curl = curl_easy_init();

  std::string url_pop = url;
  if (url_pop.back() == '/')
    url_pop.pop_back();
  std::string filename = std::filesystem::path(url_pop).filename().string();

  if (dir != "")
    filename = std::filesystem::path(dir + '/' + filename).make_preferred().string();

  if (curl)
  {
    struct curl_slist *headers = NULL;

    FILE *fp = my_fopen(filename, "wb");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    // curl_easy_setopt(curl, CURLOPT_PASSWORD, std::string(id+':'+passwd).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
#ifdef __GNUC__
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
#endif

    if (id != "")
      curl_easy_setopt(curl, CURLOPT_USERNAME, id.c_str());
    if (passwd != "")
    {
      if (id != "")
        curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd.c_str());
      else
      {
        headers = curl_slist_append(headers, passwd.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      }
    }

    switch (verbosity)
    {
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

    CURLcode code = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);

    if (code == CURLE_OK)
    {
      std::cout << "Download " << url << " to " << filename << " OK." << std::endl;
      return true;
    }

    if (headers)
      curl_slist_free_all(headers);
  }

  std::cout << "Problem with download of " << url << '.' << std::endl;
  return false;
}

bool curl_alive(std::string url)
{
  CURL *curl;
  CURLcode res;

  if (verbosity == VERBOSE)
    std::cout << "Checking access to url " << url << std::endl;

  curl = curl_easy_init();
  if (curl)
  {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (verbosity == SILENT)
    {
      std::string dummy;
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dummy);
    }
    /* Perform the query, res will get the return code */
    res = curl_easy_perform(curl);
    /* always cleanup */
    curl_easy_cleanup(curl);

    /* Check for errors */
    if (res != CURLE_OK)
    {
      if (verbosity == VERBOSE)
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
      return false;
    }
  }
  else
    return false;

  return true;
}

std::string ltrim(const std::string &s, const std::string c = " \n\r\t\f\v\"")
{
  size_t start = s.find_first_not_of(c);
  return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s, const std::string c = " \n\r\t\f\v")
{
  size_t end = s.find_last_not_of(c);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s, const std::string c = " \n\r\t\f\v")
{
  return rtrim(ltrim(s));
}

std::vector<std::string> avail_http_mets = {"GET",    "HEAD",    "POST",    "PUT",  "PATCH",
                                            "DELETE", "CONNECT", "OPTIONS", "TRACE"};

// If s is within avail_http_mets, return it else return GET
std::string check_http_met(std::string s)
{
  // Convert s to upper case
  for (auto &c : s)
    c = toupper(c);
  // Check and/or correct.
  for (auto avail_http_met : avail_http_mets)
  {
    if (s == avail_http_met)
      return avail_http_met;
  }

  return "GET";
}

std::vector<std::string> split(std::string s, char delim = ' ')
{
  std::string l;
  std::vector<std::string> v;
  std::stringstream ss(s);
  while (std::getline(ss, l, delim))
    v.push_back(l);
  return v;
}

/*
Test et génération du code source avec curl
curl --libcurl curl_post.c   -k -X POST -d "login=userBidon&consent=True"
https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users curl --libcurl curl_get.c    -k -X GET
https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon curl --libcurl curl_put.c    -k -X PUT  -d
"consent=false"               https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon curl --libcurl
curl_delete.c -k -X DELETE https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon

Dans l'ordre ajoute, list, modifie, liste, efface et liste :
./rest -r 'POST login=userBidon&consent=True' https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users
./rest -r GET                                 https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon
./rest -r 'PUT consent=False'                 https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon
./rest -r GET                                 https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon
./rest -r DELETE                              https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon
./rest -r GET                                 https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon

On peut tout faire en une seule commande :
./rest -r 'POST login=userBidon&consent=True' https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users \
          -r GET https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon \
          -r 'PUT consent=False' https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon \
          -r GET https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon \
          -r DELETE https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon \ -r GET
https://iyc-tools-rgpdserver-n265-d.epaas.eu.airbus.corp/users/userBidon

RGPD Akka :
http://and-csmo-01.akka.eu:5000
curl -k -X POST -d "login=userBidon&consent=True" http://and-csmo-01.akka.eu:5000/users
curl -k -X GET                                    http://and-csmo-01.akka.eu:5000/users/userBidon
curl -k -X PUT  -d  "consent=false"               http://and-csmo-01.akka.eu:5000/users/userBidon
curl -k -X GET                                    http://and-csmo-01.akka.eu:5000/users/userBidon
curl -k -X DELETE                                 http://and-csmo-01.akka.eu:5000/users/userBidon
curl -k -X GET                                    http://and-csmo-01.akka.eu:5000/users/userBidon
*/

CURL *rest_init(std::string url)
{
  CURL *hnd;
  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
  curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
  return hnd;
}

bool my_insec = false;

void rest_agent(CURL *hnd)
{
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, user_agent.c_str());
  curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);

  if (my_insec)
  {
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
  }
}

int rest_perform(CURL **hnd)
{
  curl_easy_setopt(*hnd, CURLOPT_TCP_KEEPALIVE, 1L);
  CURLcode ret = curl_easy_perform(*hnd);
  curl_easy_cleanup(*hnd);
  *hnd = NULL;
  return (int)ret;
}

//// GET
int rest_get(std::string url)
{
  CURL *hnd = rest_init(url);
  rest_agent(hnd);
  curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
  return rest_perform(&hnd);
}

//// POST - ADD
int rest_post(std::string url, std::string fields)
{
  CURL *hnd = rest_init(url);
  curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, fields.c_str());
  curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)fields.length());
  rest_agent(hnd);
  curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
  return rest_perform(&hnd);
}

//// PUT - MODIFY
int rest_put(std::string url, std::string fields)
{
  CURL *hnd = rest_init(url);
  curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, fields.c_str());
  curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)fields.length());
  rest_agent(hnd);
  curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "PUT");
  return rest_perform(&hnd);
}

//// DELETE
int rest_delete(std::string url)
{
  CURL *hnd = rest_init(url);
  rest_agent(hnd);
  curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "DELETE");
  return rest_perform(&hnd);
}

int rest_send(std::string url, std::string request = "", bool insecure = false, verbose_mode verbosity = DEFAULT)
{
  if (request.length() > 0)
  {
    my_insec = insecure;
    auto reqs = split(request);
    std::string met = check_http_met(reqs[0]);
    if (verbosity == VERBOSE)
      std::cout << met << " ";

    if (reqs.size() > 1)
    {
      if (verbosity == VERBOSE)
        std::cout << reqs[1] << " " << url << std::endl << std::flush;
      if (met == "POST")
        return rest_post(url, reqs[1]);
      if (met == "PUT")
        return rest_put(url, reqs[1]);
    }
    else
    {
      if (verbosity == VERBOSE)
        std::cout << url << std::endl << std::flush;
      if (met == "GET")
        return rest_get(url);
      if (met == "DELETE")
        return rest_delete(url);
    }

    if (verbosity == VERBOSE)
      std::cout << std::flush;

    return 1;
  }
  else
    return rest_get(url);
}

void usage(char *progpath, std::ostream &out = std::cout)
{
  out << "Usage: " << std::filesystem::path(progpath).stem().string() << " [OPTIONS] ..." << std::endl;
  out <<
      R"EOF(List all the available repositories for the given personal access token.
Options :
  -h, --help       : print this message and exit.
  -v, --verbose    : verbose mode. Useful for debugging and seeing what's going on "under the hood".
  -q, --silent     : silent mode.

  -p, --proxy      : a string of the form 'http(s)://server:port' to provide the url of an eventual proxy, for example '-p http://toto.fr:9090'.

  -o, --output     : filename to store the result of the query, default is to display on stdout, if possible.

  -d, --download   : download the file pointed by the provided URL. This option may be used independantly of the previous ones and multiple times.
  -i, --ident      : the eventual login id.
  -c, --pwd-hdr    : if the -i option is used then this is used as a password, else it is taken as an header.
  -w, --where      : the directory where to download the file of the -d option, default is to download in the current directory.

  -r, --rest-api   : send rest api via the specified url and exit after one or many of this option, the url can be preceded by one of the http methods (default is get), the result is displayed on the standard output or to the file provided by the -o option.
  -k, --insecure   : apply only to the -r option, allow insecure connection by not checking the ssl certificate of the remote server.

  -f, --free-query : query to add to the url, the result is displayed as is.
  rest -f user -j name -j login <=> curl -H "Accept: application/vnd.github.v3+json" https://toto.fr | jq '.name,.login'

  -a, --alive      : just return if the url(s) passed in parameter point to a living site. Add the -v option if you want to display more message else -q if you only want to test the returned status. This option can be used multiple times.
)EOF";
}
// "hvqp:i:c:w:r:ka:f:j:"

int main(int argc, char *argv[])
{
  static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                         {"verbose", no_argument, 0, 'v'},
                                         {"silent", no_argument, 0, 'q'},

                                         {"proxy:", required_argument, 0, 'p'},

                                         {"download:", required_argument, 0, 'd'},
                                         {"ident:", required_argument, 0, 'i'},
                                         {"pwd-hdr:", required_argument, 0, 'c'},
                                         {"where:", required_argument, 0, 'w'},

                                         {"rest-api:", required_argument, 0, 'r'},
                                         {"insecure", no_argument, 0, 'k'},

                                         {"alive:", required_argument, 0, 'a'},
                                         {"free-query", required_argument, 0, 'f'},
                                         {0, 0, 0, 0}};

  int option_index = 0, c;
  bool rest_only = false, insec = false;
  std::vector<std::string> dl_urls, al_urls, rest_apis, http_mets;
  std::string ident = "", passwd = "", dl_dir = "", free_query = "";

  while ((c = getopt_long(argc, argv, "hvqp:d:i:c:w:r:ka:f:", long_options, &option_index)) != -1)
  {
    switch (c)
    {
    case 'h':
      usage(argv[0]);
      return EXIT_SUCCESS;
    case 'v':
      verbosity = VERBOSE;
      break;
    case 'q':
      verbosity = SILENT;
      break;

    case 'p':
      proxy = optarg;
      break;

    case 'd':
      dl_urls.push_back(optarg);
      break;

    case 'i':
      ident = optarg;
      break;

    case 'c':
      passwd = optarg;
      break;

    case 'w':
      dl_dir = optarg;
      break;

    case 'r':
      rest_only = true;

      if (optind < argc && argv[optind][0] != '-')
      {
        http_mets.push_back(optarg);
        rest_apis.push_back(argv[optind]);
      }
      else
      {
        http_mets.push_back("GET");
        rest_apis.push_back(optarg);
      }
      break;

    case 'k':
      insec = true;
      break;

    case 'a':
      al_urls.push_back(optarg);
      break;

    case 'f':
      free_query = optarg;
      break;

    case '?':
      usage(argv[0], std::cerr);
      return EINVAL;
    default:
      std::cout << "?? getopt returned character code " << c << " ??" << std::endl;
    }
  }

  if (rest_only)
  {
    if (insec && verbosity == VERBOSE)
      std::cout << "insecure rest api call\n";

    for (size_t n = 0; n < MIN(http_mets.size(), rest_apis.size()); n++)
    {
      auto h = http_mets[n];
      auto r = rest_apis[n];
      std::cout << "R " << r << ", H " << h << std::endl;
      if (verbosity == VERBOSE)
        std::cout << h << " " << r << std::endl;
      if (rest_send(r, h, insec, verbosity))
        return EXIT_FAILURE;
      if (verbosity != SILENT)
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
  }

  if (dl_urls.size() == 0 && al_urls.size() == 0 && free_query == "")
  {
    std::cerr << "Missing arguments" << std::endl;
    usage(argv[0]);
    return EINVAL;
  }

  // Si options -d et éventuellement -w, -i, -c
  for (auto dl_url : dl_urls)
  {
    if (!curl_download(dl_url, dl_dir, ident, passwd))
      return EXIT_FAILURE;
  }

  // Si options -a
  for (auto al_url : al_urls)
  {
    if (!curl_alive(al_url))
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
