
root_dir=..

include ${root_dir}/header.mk

PREFIX=monitorsinfo
SRCS=$(wildcard *.cpp)

OBJS+=$(SRCS:.cpp=.o)
TARGET=${PREFIX}${EXEXT}

.PHONY: FORCE

ifeq ($(DO_MSBUILD),1)
ARCH=x64
CONF=Release
MSVC_SLN=monitorsinfo.sln

all : ${TARGET}

${TARGET} : ${ARCH}/${CONF}/${TARGET}
	cp ${ARCH}/${CONF}/${TARGET} .

${ARCH}/${CONF}/${TARGET} : ${PREFIX}.ico ${SRCS} ${RES_SRC}
	${MSBUILD} ${MSVC_SLN} -p:Configuration=${CONF}

else
all : ${TARGET}

${TARGET} : ${OBJS}
	$(LINK.cc) ${OBJS} $(LOADLIBES) $(LDLIBS) -o $@
endif

gcc : all

strip :
	strip *.exe | true

upx : strip
	upx *.exe | true

ifeq ($(MAKECMDGOALS),clean)
TORM=$(fwildcard *.vcxproj.user)
endif
clean :
	rm -f *.mslog $(TORM)
	@rm -rf x64 .vs

include ${root_dir}/rules.mk

