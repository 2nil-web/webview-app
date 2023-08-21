
PGF=$(subst \,/,$(subst C:\,/c/,$(PROGRAMFILES)))
PGF86=${PGF} (x86)
PATH:=${PATH}:${PGF86}/Inno Setup 6
PATH:=${PATH}:${PGF}/Inkscape/bin

MAGICK=magick
RC=windres
STRIP=strip
UPX=upx

WVDIR=webview-gcc
WV2DIR=Microsoft.Web.WebView2.1.0.1150.38
CPPFLAGS += -I${WVDIR} -I${WVDIR}/build/external/libs/${WV2DIR}/build/native/include -IWebView2/include --include=webview_mingw_support.h
#CXXFLAGS += -Wall -Wextra -pedantic  -std=c++14
LDFLAGS += -static -mwindows
LDLIBS += -ladvapi32 -lole32 -lshell32 -lshlwapi -luser32 -lversion

EXEXT=.exe
PREFIX=webview-test
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)
OBJS += ${PREFIX}_res.o

TARGET=${PREFIX}${EXEXT}

all : ${TARGET}

${TARGET} : ${OBJS}

${PREFIX}_res.o : ${PREFIX}.ico


strip : $(TARGET)
	$(STRIP) $(TARGET) | true

upx : strip
	$(UPX) $(TARGET) | true

clean :
	rm -f *~ ${PREFIX}.ico *.o $(OBJS)

rclean :
	rm -f *~ *.d ${PREFIX}.ico *.o $(OBJS) $(TARGET) WebView2Loader.dll

cfg :
	@echo "PGF ${PGF}"
	@echo "PGF86 ${PGF86}"
	@echo "PATH"
	@echo "${PATH}" | sed 's/:/\n/g'
	@echo "END PATH"
	which inkscape.exe

# Ces régles implicites ne sont pas utiles quand on fait 'make rclean' (voir même make clean ...)
ifneq ($(MAKECMDGOALS),rclean)
%.exe: %.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.exe: %.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.exe: %.cpp
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.ico : %.png
	${MAGICK} convert -background none $< $@

%.ico : %.svg
	${MAGICK} convert -background none $< $@

# Régles pour construire les fichier objet d'après les .rc
%.o : %.rc
	$(RC) $(CPPFLAGS) $< --include-dir . $(OUTPUT_OPTION)

%.d: %.c
	@echo Checking header dependencies from $<
	@$(COMPILE.c) -isystem /usr/include -MM $< >> $@

#	@echo "Building "$@" from "$<
%.d: %.cpp
	@echo Checking header dependencies from $<
	@$(COMPILE.cpp) -isystem /usr/include -MM $< >> $@

# Inclusion des fichiers de dépendance .d
ifdef OBJS
-include $(OBJS:.o=.d)
endif
endif

