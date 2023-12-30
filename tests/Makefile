
root_dir=$(realpath ..)

include ${root_dir}/header.mk

SRCS=$(wildcard *.cpp)
SRCS:=$(filter-out TemplateApp.cpp, $(SRCS))
EXES=${SRCS:.cpp=.exe}


all : ${EXES}

gcc : all

strip :
	strip *.exe | true

upx : strip
	upx *.exe | true

ifeq ($(MAKECMDGOALS),clean)
TORM=$(filter-out TemplateApp.sln,$(wildcard *.sln)) $(filter-out TemplateApp.vcxproj,$(wildcard *.vcxproj)) $(filter-out TemplateApp.vcxproj.user,$(wildcard *.vcxproj.user))
endif
clean :
	rm -f ${EXES} *.mslog $(TORM)
	@rm -rf x64 .vs

include ${root_dir}/rules.mk

