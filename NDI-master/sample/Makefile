# sample 
CC=g++
CFLAGS=-c

obj_dir ?= $(BUILD_DIR)/obj/sample
lib_obj_dir ?= $(BUILD_DIR)/obj/library


exe_sample := $(BUILD_DIR)/capisample
sources := $(wildcard ./src/*.cpp)
objects = $(sources:%.cpp=$(obj_dir)/%.o)
lib_objects = $(wildcard $(lib_obj_dir)/src/*.o)
include_dirs := ../library/include
CPPFLAGS += $(addprefix -I ,$(include_dirs))

all: $(exe_sample)

# statically linked sample app
$(exe_sample): $(objects) $(exe_sample)-dyn
	$(CXX) $(lib_objects) $(objects) -o $@ $(LDFLAGS)
	@echo "sample exe successful!"

# dynamically linked sample app
$(exe_sample)-dyn: $(objects) $(LIB_LIBRARY)
	$(CXX) $(objects) -L$(BUILD_DIR) -lndicapi -o $@ $(LDFLAGS)
	@echo "dyn sample exe successful!"

$(objects): $(obj_dir)/%.o: %.cpp
	@echo Compiling $<
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: clean
clean:
	@echo "doing clean in sample library"
	$(RM) -r $(obj_dir)
	$(RM) -f $(exe_sample) $(exe_sample)-dyn
