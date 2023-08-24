
#include <windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include <thread>


#include "util.h"
#include "runopt.h"

extern void run_webview(bool devmode, void *wnd, int width, int height, int hints, std::string url, std::string title="", std::string init_js="");

bool devmode=false;
void *wnd=nullptr;
int width=-1, height=-1, hints=0; /*
0 Width and height are default size
1 Width and height are minimum bounds
2 Width and height are maximum bounds
3 Window size is fixed
*/

std::string url, title="", init_js="";

void get_args() {
  devmode=true;
}

std::vector<run_opt> r_opts = {
  /*
  { "i_nfo",      'n', opt_only,  no_argument,       "display various informations on the current file.", info },
  { "_line",      'l', opt_only,  optional_argument, "display lines of the current file. Without parameters it will display all the lines, an interactive warning might appear if the file has more than a 1000 lines. You can also pass a range in the form 'r1-r2' or a list of line in the form 'r1 r2 r3 ...'. Rows indexes start to 1 and end to maximum number of lines.", row },
  { "_cell",      'c', opt_only,  optional_argument, "behave like the 'line' command but for cells.", cell },
  { "lincol",    '\0', opt_only,  required_argument, "display a cell by its line and column coordinate. By example lincol 0,0 <=> cell 0 and lincol 'lastline','lastcol' <=> cell 'lastcellindex'.", linecolumn },*/
  { "", '\0', 0, 0, "\nAdditionnal help message.", NULL },
  { "", '\0', 0, 0, "", NULL },
  { "", '\0', 0, 0, "\n2nd Additional message.", NULL }
};


#ifdef _WIN32

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
  LPSTR *argv;
  int argc;
  argv=CommandLineToArgvA(GetCommandLine(), &argc);
#else
int main(int argc, char **argv, char **) {
#endif
  getopt_init(argc, argv, r_opts, "WebView app.", "", "(c) Denis LALANNE. Provided as is. NO WARRANTY of any kind.");
  url=std::string(lpCmdLine);

  if (url.empty()) {
    if (title.empty()) title="Missing parameter";
    url="html://Pass a url, an html file or an html text as parameter to the program.";
  }

  run_webview(devmode, wnd, width, height, hints, url, title, init_js);

  return 0;
}

