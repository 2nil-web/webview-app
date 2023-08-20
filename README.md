# webview-test

Test of the : [webview library](https://github.com/webview/webview.git).

Provided with a Makefile that build the test under msys/mingw64.

The test file originate from the bind.cc example of the webview library.

The content of the 2 folders "webview" and "WebView2" comes from building the webview library with the "script/build.sh" file.

The icon is an svg built with inkscape and converted into a .ico file with magick convert.

The resulting binary size  can be reduced with upx (make upx).
