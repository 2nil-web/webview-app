
# Intégrer clang-format

CXXFLAGS += -std=c++20 -g -Wall 
LDFLAGS += -static
SRCS=$(wildcard *.cc)
EXES=${SRCS:.cc=.exe}

all : ${EXES}

clean :
	rm -f ${EXES}

%.exe: %.cc
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

