
MSBUILD=/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/MSBuild/Current/Bin/amd64/MSBuild.exe
DO_MSBUILD=$(shell test -f $(MSBUILD) && echo 1 || echo 0)
DO_MSBUILD=0

ifeq ($(DO_MSBUILD),1)
ARCH=x64
CONF=Release
define genmsexe
	@sed 's/\<TemplateApp\>/$(1)/g' TemplateApp.sln >$1.sln
	@sed 's/\<TemplateApp\.cpp\>/$(1).cc/g;s/\<TemplateApp\>/$(1)/g' TemplateApp.vcxproj >$1.vcxproj
	@${MSBUILD} $1.sln -p:Configuration=${CONF} >$1.mslog && mv ${ARCH}/${CONF}/$1.exe . && rm -f $1.mslog
	@rm -f $1.sln $1.vcxproj*
	@rm -rf ${ARCH}
endef
else
	# Intégrer clang-format
	WVDIR=../webview
	WV2DIR=Microsoft.Web.WebView2.1.0.1150.38
	CPPFLAGS += -I${WVDIR} -I${WVDIR}/build/external/libs/${WV2DIR}/build/native/include
	CPPFLAGS+=-DCURL_STATICLIB
	CXXFLAGS += -std=c++20 -g -Wall 
	LDFLAGS += -static
	LDFLAGS += -g #-mwindows
	#LDFLAGS += -LC:/Software/UnixTools/msys64/mingw64/lib 
	#LDLIBS += -ladvapi32 -lole32 -lshell32 -lshlwapi -luser32 -lversion
	#pacman -S mingw-w64-x86_64-curl-gnutls
	#LDLIBS += -lcurl -lssh2 -lssh2 -lpsl -lbcrypt -ladvapi32 -lcrypt32 -lbcrypt -lwldap32 -lzstd -lzstd -lbrotlidec -lbrotlidec -lz -lws2_32
	#LDLIBS += -lbrotlidec -lbrotlicommon -lidn2 -liconv -lunistring
endif

SRCS=$(wildcard *.cc)
EXES=${SRCS:.cc=.exe}


all : ${EXES}

strip :
	strip *.exe | true

upx : strip
	upx *.exe | true

clean :
	rm -f ${EXES} *.mslog

%.exe: %.cc
ifeq ($(DO_MSBUILD),1)
	@echo "Building $@ from $^ with msvc"
	$(call genmsexe,$*,${ARCH},${CONF})
else
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@
endif

