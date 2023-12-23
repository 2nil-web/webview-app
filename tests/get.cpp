
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

void usage(char *progpath, std::ostream &out = std::cout)
{
  out << "Usage: " << std::filesystem::path(progpath).stem().string() << " [OPTIONS] ... url" << std::endl;
  out <<
      R"EOF(List all the available repositories for the given personal access token.
Options :
  -h, --help       : print this message and exit.
  -v, --verbose    : verbose mode. Useful for debugging and seeing what's going on "under the hood".
  -q, --silent     : silent mode.

  -k, --insecure   : apply only to the -r option, allow insecure connection by not checking the ssl certificate of the remote server.
)EOF";
}

enum verbose_mode
{
  SILENT,
  DEFAULT,
  VERBOSE
};

size_t writer(char *data, size_t size, size_t nmemb, std::string *writerData)
{
  if (writerData == NULL)
    return 0;
  writerData->append(data, size * nmemb);
  return size * nmemb;
}

#define trc std::cout << __LINE__ << std::endl << std::flush

bool do_curl(std::string url, verbose_mode verbosity, bool insecure, std::vector<std::string> &hds)
{
  std::string buffer;
  CURLcode code;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();

  if (curl)
  {
    struct curl_slist *headers = NULL;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    if (insecure)
    {
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    for (auto hd : hds)
    {
      std::cout << hd << std::endl << std::flush;
      headers = curl_slist_append(headers, hd.c_str());
    }

    trc;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    trc;
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

    trc;
    code = curl_easy_perform(curl);
    trc;
    curl_easy_cleanup(curl);
    trc;

    if (headers != NULL)
      curl_slist_free_all(headers);
    trc;
  }

  curl_global_cleanup();

  if (code == CURLE_OK)
  {
    std::cout << buffer << std::endl;
    return true;
  }
  else
  {
    std::cout << "Problem with download of " << url << '.' << std::endl;
    return false;
  }
}

int main(int argc, char *argv[])
{
  static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                         {"verbose", no_argument, 0, 'v'},
                                         {"silent", no_argument, 0, 'q'},
                                         {"header", required_argument, 0, 'H'},
                                         {0, 0, 0, 0}};

  int option_index = 0, c;
  bool insecure = false;
  verbose_mode verbosity = DEFAULT;
  std::vector<std::string> heads, urls;

  while ((c = getopt_long(argc, argv, "hvqkH:", long_options, &option_index)) != -1)
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

    case 'k':
      insecure = true;
      break;

    case 'H':
      heads.push_back(optarg);
      break;

    case '?':
      usage(argv[0], std::cerr);
      return EINVAL;
    default:
      std::cout << "?? getopt returned character code " << c << " ??" << std::endl;
    }
  }

  if (optind < argc)
  {
    while (optind < argc)
    {
      std::cout << argv[optind] << std::endl;
      if (!do_curl(argv[optind], verbosity, insecure, heads))
        return EXIT_FAILURE;
      optind++;
    }
  }
  else
  {
    std::cerr << "Missing at least one urls." << std::endl;
    return EINVAL;
  }

  return EXIT_SUCCESS;
}
