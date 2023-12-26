
#include "../wv-curl.cpp"

bool verbose = false, peer_check = false, host_check = false;

// curl -sku lalannd2:ocvdBum12$*4
// https://wiki.space.thales/rest/api/content/search?cql=contributor+in+(alkadea,arnones,capous,cavallc,chaumia1,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,thurona,tourel,xsii077,xsii076)+and+space+=+orchestra+and+lastmodified+=+2023-11-07&limit=1000
//'https://lalannd2:ocvdBum12$*3@wiki.space.thales/rest/api/content/search?cql=contributor+in+(alkadea,arnones,capous,cavallc,chaumia1,fresnew,guyonnt,kouachb,lalannd2,leleut,moninn,monnete,nottea,thurona,tourel,xsii077,xsii076)+and+space+=+orchestra+and+lastmodified+=+2023-10-02&limit=1000'
void check_opts(std::string opt)
{
  if (opt == "verbose")
  {
    verbose = true;
    std::cout << "verbose " << verbose << std::endl;
  }
  else if (opt == "peer_check")
  {
    peer_check = true;
    std::cout << "peer_check " << peer_check << std::endl;
  }
  else if (opt == "host_check")
  {
    host_check = true;
    std::cout << "host_check " << host_check << std::endl;
  }
}

int main(int argc, char **argv)
{
  if (argc >= 4)
  {
    std::string id, pass, url;

    id = argv[1];
    pass = argv[2];
    url = argv[3];

    for (int i = 4; i < argc; i++)
      check_opts(argv[i]);
    if (verbose)
      std::cout << "RESULT IS:" << std::endl;
    std::cout << httpget_cred(id, pass, url, peer_check, host_check, verbose);
  }
  else
    std::cerr << "Missing mandatory parameters id, password and url" << std::endl;
}
