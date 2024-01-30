
# intégrer js check + minify

# Bug d'affichage avec uname sous clang64 ...
ifeq (${MSYSTEM},CLANG64)
	UNAME=/usr/bin/uname
	CC=clang
	CXX=clang++
else
	UNAME=uname
	CC=gcc
	CXX=g++
endif

ifneq (${WITH_CURL},)
ifeq (${MSYSTEM},UCRTW64)
	STATIC_CURL=0
else
	STATIC_CURL=1
endif

STATIC_CURL=0
endif

OS=$(shell ${UNAME} -s)

ifneq (${OS},Linux)
ECHO=echo -e
else
ECHO=echo
endif

PGF=$(subst \,/,$(subst C:\,/c/,$(PROGRAMFILES)))
PGF86=${PGF} (x86)
PATH:=${PATH}:${PGF86}/Inno Setup 6
PATH:=${PATH}:${PGF}/Inkscape/bin
PATH:=${PATH}:${PGF86}/Pandoc

ifneq (${OS},Linux)
 MAGICK=magick
endif
RC=windres
STRIP=strip
UPX=upx

VERSION=$(shell git describe --abbrev=0 --tags 2>/dev/null || echo 'Unknown_version')
COMMIT=$(shell git rev-parse --short HEAD 2>/dev/null || echo 'Unknown_commit')
DECORATION=Nawak-Bidon
ISO8601 := $(shell date +%Y-%m-%dT%H:%M:%SZ)

ifeq (${root_dir},)
	root_dir=.
endif

WVDIR=${root_dir}/webview
WV2SUBDIR=Microsoft.Web.WebView2.1.0.1150.38
CPPFLAGS += -DWIN32_LEAN_AND_MEAN


ifneq (${WITH_CURL},)
CPPFLAGS += -DWITH_CURL
ifeq ($(STATIC_CURL),1)
CPPFLAGS += -DCURL_STATICLIB
endif
endif
CPPFLAGS += -I${WVDIR} -I${WVDIR}/build/external/libs/${WV2SUBDIR}/build/native/include

CXXFLAGS += -std=c++20 -g
CXXFLAGS += -Wall # -pedantic -Wextra # Utiliser ces 2 dernières options de temps en temps peut-être utile ...
CXXFLAGS += -Wno-unknown-pragmas
LDFLAGS += -g

ifeq (${OS},Linux)
CXXFLAGS += $(shell pkg-config --cflags gtk+-3.0 webkit2gtk-4.1)
LDFLAGS +=-L/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu/webkit2gtk-4.1 -L/usr/lib/x86_64-linux-gnu/cmake/harfbuzz -L/usr/lib/python3/dist-packages/cairo -L/usr/lib/x86_64-linux-gnu/glib-2.0 -L/usr/lib/x86_64-linux-gnu/glib-2.0
LDLIBS += $(shell pkg-config --libs gtk+-3.0 webkit2gtk-4.1 webkit2gtk-web-extension-4.1)
ifneq (${WITH_CURL},)
LDLIBS += $(shell pkg-config --libs libcurl)
endif
#LDFLAGS += -static
PANDOC=pandoc
else
EXEXT=.exe
CPPFLAGS += --include=webview_mingw_support.h
#LDFLAGS += -mwindows
LDFLAGS += -static

ifneq (${WITH_CURL},)
#pacman -S mingw-w64-x86_64-curl-gnutls
ifeq ($(STATIC_CURL),1)
LDLIBS += -lcurl
else
LDLIBS += -lcurl.dll
endif
LDLIBS += -lidn2 -lssh2 -lpsl -lbcrypt -lcrypt32 -lzstd -lz -liconv -lunistring -lbrotlidec -lbrotlicommon
endif

LDLIBS += -lwldap32 -lws2_32 -ladvapi32 -lole32 -lshell32 -lshlwapi -loleaut32 -lcomctl32 -lactiveds -luser32 -lnetapi32 -lversion	
endif

#MSBUILD='C:\Program\ Files\Microsoft\ Visual\ Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe'
MSBUILD=/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/MSBuild/Current/Bin/amd64/MSBuild.exe
DO_MSBUILD=$(shell test -f $(MSBUILD) && echo 1 || echo 0)
ifeq ($(MAKECMDGOALS),gcc)
DO_MSBUILD=0
endif

ifeq ($(DO_MSBUILD),1)
# These 2 environment variables cause troubles to the Visual Studio build
unexport TMP TEMP
endif

