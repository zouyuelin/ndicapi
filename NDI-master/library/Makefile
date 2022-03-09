# library

CC=g++
CFLAGS=-c

obj_dir ?= $(BUILD_DIR)/obj/library

sources := $(wildcard ./src/*.cpp)
objects = $(sources:%.cpp=$(obj_dir)/%.o)
include_dirs := include src/include
CPPFLAGS += $(addprefix -I ,$(include_dirs))

all: $(LIB_LIBRARY)

$(LIB_LIBRARY): $(objects)
	mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^
	@echo "library build successful!"

$(objects): $(obj_dir)/%.o: %.cpp
	@echo Compiling $<
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: clean
clean:
	@echo "doing clean in library"
	$(RM) -r $(obj_dir)	
	$(RM) -f $(LIB_LIBRARY)	
