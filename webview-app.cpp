
#include <windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>

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

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
#else
int main() {
#endif
  get_args();
  url=std::string(lpCmdLine);

  if (url.empty()) {
    if (title.empty()) title="Missing parameter";
    url="html://Pass a url, an html file or an html text as parameter to the program.";
  }

  run_webview(devmode, wnd, width, height, hints, url, title, init_js);

  return 0;
}

