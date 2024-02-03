@echo off

echo Prerequisite are git and the Visual Studio compiler suite for C++.
echo To generate the application icon you may need magick convert or provide an icon file named webview-app.ico.

where git >nul || ( echo You need git to download the webview library.; exit /b )

where cmake >nul || ( echo You need cmake to build the webview library.; exit /b )

echo Considering the use of webview with Visual Studio.

if exist webview\ (
  echo No need to clone the webview directory as it already exists.
) else (
  echo Cloning webview ...
  git clone https://github.com/webview/webview.git webview
)

cmd /C webview\script\build.bat

::copy webview\webview.h .
copy webview\build\external\libs\Microsoft.Web.WebView2*\build\native\include\* .
echo You can now compile this example by running Visual Studio.

