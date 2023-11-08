
#ifndef WEBVIEW_RUN_H
#define WEBVIEW_RUN_H

void webview_set(bool devmode = false, int width = 640, int height = 480, int hints = 0, bool runjs_and_exit = false);
void webview_run(std::string url, std::string title = "", std::string init_js = "");
#endif // WEBVIEW_RUN_H
