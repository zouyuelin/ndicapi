$(warning ardemo exe directory)

CC=gcc
ifeq ($(PLATFORM),macosx)
  LDFLAGS = -L/Library/Frameworks/GStreamer.framework/Versions/1.0/lib
else
  CFLAGS=`pkg-config --cflags gstreamer-1.0`
endif 


obj_dir ?= $(BUILD_DIR)/obj/ndigst/ardemo

exe_ardemo := $(BUILD_DIR)/ardemo
sources := $(wildcard ./ardemobin.c)
objects = $(sources:%.c=$(obj_dir)/%.o)
exes = $(sources:%.c=$(BUILD_DIR)/%)
flatcc_dir := ../external/flatcc/lib/$(PLATFORM)

include_dirs := ../capitogst/flatbuffer/fbheaders ../../library/include \
 ../external/flatcc/include \
 ../ndielems \
 /usr/local/include/gstreamer-1.0
 
ifeq ($(PLATFORM),macosx)
  include_dirs += /Library/Frameworks/GStreamer.framework/Headers 
endif 

CPPFLAGS += $(addprefix -I ,$(include_dirs))

all: $(exe_ardemo)
 
# dynamically linked sample app
$(exe_ardemo): $(objects) $(LIB_LIBRARY) $(LIB_CAPITOGST) $(LIB_NDIELEMS)
	$(CXX) $(CFLAGS) $(objects) -L$(BUILD_DIR) -L$(flatcc_dir) \
    -lgobject-2.0 -lglib-2.0 -lndielems -lcapitogst -lgstreamer-1.0 \
    -lgstbase-1.0 -lgstvideo-1.0 -lflatcc -lflatccrt -lndicapi -o $@ $(LDFLAGS)
	@echo "dyn ardemo exe successful!"

$(objects): $(obj_dir)/%.o: %.c
	@echo Compiling $<
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: clean
clean:
	@echo "doing clean in sample library"
	$(RM) -r $(obj_dir)
	$(RM) -f $(exe_ardemo)
