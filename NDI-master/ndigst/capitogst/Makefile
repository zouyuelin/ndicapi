# capitogst

CC=g++
CFLAGS=-c

obj_dir ?= $(BUILD_DIR)/obj/ndigst/capitogst

sources := $(wildcard ./*.cpp)
objects = $(sources:%.cpp=$(obj_dir)/%.o)
include_dirs := ./flatbuffer/fbheaders ../../library/include ../external/flatcc/include
# from https://github.com/dvidelabs/flatcc#portability-layer
# Older/non-standard versions of C++ compilers cause problems because 
# static_assert and alignas behave in strange ways where they are neither 
# absent nor fully working as expected. There are often workarounds, 
# but it is more reliable to use -std=c++11 or -std=c++14.
CPPFLAGS += $(addprefix -I ,$(include_dirs)) -std=c++11

all: $(LIB_CAPITOGST)

$(LIB_CAPITOGST): $(objects)
	$(AR) $(ARFLAGS) $@ $^
	@echo "library build successful!"

$(objects): $(obj_dir)/%.o: %.cpp
	@echo Compiling $<
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: clean
clean:
	@echo "doing clean in ndigst/capitogst"
	$(RM) -r $(obj_dir)
	$(RM) -f $(LIB_CAPITOGST)
