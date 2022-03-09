#ndielems

CC=gcc

ifeq ($(PLATFORM),macosx)
  CFLAGS=-c 
else
  CFLAGS=-c `pkg-config --cflags gstreamer-1.0`
endif 

obj_dir ?= $(BUILD_DIR)/obj/ndigst/ndielems

sources := $(wildcard ./*.c)
objects = $(sources:%.c=$(obj_dir)/%.o)
include_dirs := ../capitogst/flatbuffer/fbheaders ../../library/include ../external/flatcc/include /usr/local/include/gstreamer-1.0


 ifeq ($(PLATFORM),macosx)
    include_dirs += /Library/Frameworks/GStreamer.framework/Headers 
 endif 

CPPFLAGS += $(addprefix -I ,$(include_dirs))

all: $(LIB_NDIELEMS)

$(LIB_NDIELEMS): $(objects)
	$(AR) $(ARFLAGS) $@ $^
	@echo "ndielems library build successful!"

$(objects): $(obj_dir)/%.o: %.c
	@echo Compiling $<
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: clean
clean:
	@echo "doing clean in ndigst/ndielems"
	$(RM) -r $(obj_dir)
	$(RM) -f $(LIB_NDIELEMS)
