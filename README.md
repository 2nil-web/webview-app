# WebView Application template

Application using the : [webview library](https://github.com/webview/webview.git).

1) Prerequisite to build the app are:
   
   - git to download the webview library.
   
   - make and g++ to build the webview lib and the app under linux, Windows and eventually MacOS.
   
   - Or Visual Studio under Windows.

2) A Makefile is provided that allow the application build under the gcc/g++ compiler suite.

3) An .sln file is provided that allow the build under the MS Visual Studio compiler suite.

4) The application is written in C++.
   
   - It originates from the bind.cc example of the webview library.
   - It has been tested under Windows and Linux.
   - It remain to be tested under MacOS although it should works, as stated by the webview library documentation ...
   - An svg icon is provided that can be used to create a .ico file with magick convert. Or provide your own .ico file.
   - The resulting binary size  can be reduced with upx (make upx).

5) Built
   
   - To set the webview library run the "initlib.sh" script under bash or the "initlib.bat" script under Windows cmd.
   
   - Then run make under gcc.
   
   - Or build the webview-app.sln under MS Visual Studio.
   
   - And from time to time update the webview header with :

```bash
cd webview
curl -sO https://raw.githubusercontent.com/webview/webview/master/webview.h
curl -sO https://raw.githubusercontent.com/webview/webview/master/webview.cc
```

Patch to webview.h 0.11.0 for linux

Patch file

```diff
--- webview.h    2023-10-29 10:21:37.270757522 +0100
+++ my_webview.h    2023-10-29 10:20:14.734136652 +0100
@@ -686,7 +686,7 @@
   }

   void eval(const std::string &js) {

- webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(m_webview), js.c_str(),
+ webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(m_webview), js.c_str(), js.size(), nullptr, nullptr,

                                 nullptr, nullptr, nullptr);

  }
```

Then patch command to apply

```bash
cd webview
patch webview.h <../patch_webview_linux
```

6. libcurl
* Download
  
  * git clone https://github.com/curl/curl.git

* Static build under visual studio "x64 Native Tools COmmand Prompt for VS 2022"
  
  * rename src/tool_hugehelp.c.cvs to src/tool_hugehelp.c
  
  * cd curl\winbuild
  
  * nmake /f Makefile.vc mode=static
  
  * copy ..\builds\libcurl-vc-x64-release-static-ipv6-sspi-schannel where you need it.



ToDo :

Faire des tests avec Dom Examples : [GitHub - mdn/dom-examples: Code examples that accompany various MDN DOM and Web API documentation pages](https://github.com/mdn/dom-examples.git)
