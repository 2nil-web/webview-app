
# Intégrer clang-format

CPPFLAGS+=-IC:/Software/UnixTools/msys64/mingw64/include
CPPFLAGS+=-DCURL_STATICLIB
CXXFLAGS += -std=c++20 -g -Wall 
LDFLAGS += -static
LDFLAGS += -static -mwindows -g
LDFLAGS += -LC:/Software/UnixTools/msys64/mingw64/lib 

LDLIBS += -ladvapi32 -lole32 -lshell32 -lshlwapi -luser32 -lversion

#pacman -S mingw-w64-x86_64-curl-gnutls
LDLIBS += -lcurl -lssh2 -lssh2 -lpsl -lbcrypt -ladvapi32 -lcrypt32 -lbcrypt -lwldap32 -lzstd -lzstd -lbrotlidec -lbrotlidec -lz -lws2_32
LDLIBS += -lbrotlidec -lbrotlicommon -lidn2 -liconv -lunistring

SRCS=$(wildcard *.cc)
EXES=${SRCS:.cc=.exe}

all : ${EXES}

clean :
	rm -f ${EXES}

%.exe: %.cc
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

