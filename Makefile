
include header.mk

PREFIX=webview-app
SRCS=$(wildcard *.cpp)

ifeq (${OS},Linux)
SRCS:=$(filter-out wv-winapi.cpp,${SRCS})
else
OBJS=${PREFIX}_res.o
RES_SRC=${PREFIX}_res.rc
endif

OBJS+=$(SRCS:.cpp=.o)
TARGET=${PREFIX}${EXEXT}

.PHONY: FORCE

ifeq ($(DO_MSBUILD),1)
ARCH=x64
CONF=Release
DEFAULT_TARGET=version_check.txt version.h ${PREFIX}.ico ${TARGET} README.docx

${TARGET} : ${ARCH}/${CONF}/${TARGET}
	cp ${ARCH}/${CONF}/${TARGET} .

${ARCH}/${CONF}/${TARGET} : ${PREFIX}.ico ${SRCS} ${RES_SRC}
	${MSBUILD} webview-app.sln -p:Configuration=${CONF}
else
DEFAULT_TARGET=version_check.txt version.h ${TARGET}

${TARGET} : ${OBJS}
	$(LINK.cc) ${OBJS} $(LOADLIBES) $(LDLIBS) -o $@
endif

all : ${DEFAULT_TARGET}
	@echo "All done ${DEFAULT_TARGET}"

gcc : all

${PREFIX}_res.o : ${PREFIX}.ico


strip : $(TARGET)
	$(STRIP) $(TARGET) | true

upx : strip
	$(UPX) $(TARGET) | true

ifeq ($(MAKECMDGOALS),deliv)
DLLDEPS=$(shell ldd ${TARGET} | sed "/WINDOWS/d;s/.*=> //;s/ .0x.*//" | sort -u | tr '\n' ' ')
endif

deliv : upx
ifneq ($(DLLDEPS),)
	@echo "Bringing DLL dependencies."
	@cp ${DLLDEPS} .
endif

clean :
	rm -f *~ *.d ${PREFIX}.ico *.o $(OBJS) $(TARGET)
ifeq ($(DO_MSBUILD),1)
	rm -rf ${ARCH} README.docx
endif

include rules.mk

