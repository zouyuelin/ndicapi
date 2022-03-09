OS=$(shell uname -s)
ifndef PLATFORM
  ifeq ($(OS),Darwin)
    PLATFORM=macosx
    ndicapi_lib_nm := libndicapi.dylib
    capitogst_lib_nm := libcapitogst.dylib
    ndielems_lib_nm := libndielems.dylib
  else   
    ifeq ($(OS),Linux)
      PLATFORM=linux
      ndicapi_lib_nm := libndicapi.so
      capitogst_lib_nm := libcapitogst.so
      ndielems_lib_nm := libndielems.so
    else
      PLATFORM=x86-win64
      ndicapi_lib_nm := ndicapi.dll
      capitogst_lib_nm := capitogst.dll
      ndielems_lib_nm := ndielems.dll
    endif
  endif
endif

library_lib := library
sample_exe := sample
capitogst_lib := ndigst/capitogst
ndielems_lib := ndigst/ndielems
ardemo_exe := ndigst/ardemo
gst_libraries := $(capitogst_lib) $(ndielems_lib)

export BUILD_DIR := $(abspath ./build/$(PLATFORM))
export LIB_LIBRARY := $(BUILD_DIR)/$(ndicapi_lib_nm)
export LIB_CAPITOGST := $(BUILD_DIR)/$(capitogst_lib_nm)
export LIB_NDIELEMS := $(BUILD_DIR)/$(ndielems_lib_nm)
export PLATFORM

.PHONY: all $(sample_exe) $(ardemo_exe) $(library_lib) $(gst_libraries)
all: $(sample_exe) $(ardemo_exe)

$(sample_exe) $(library_lib):
	$(MAKE) --directory=$@ $(TARGET)
	
#convience to allow $ make ardemo	
.PHONY: ardemo
ardemo : $(ardemo_exe)

#convience to allow $ make capitogst	
.PHONY: capitogst
capitogst : $(capitogst_lib)

#convience to allow $ make ndielems	
.PHONY: ndielems
ndielems : $(ndielems_lib)

# to clean, use:  $ make TARGET=clean
$(ardemo_exe) $(gst_libraries):
	$(MAKE) --directory=$@ $(TARGET)
	$(if $(TARGET), $(MAKE) $(TARGET))

$(sample_exe): $(library_lib)
$(ardemo_exe): $(library_lib) $(gst_libraries)
$(capitogst_lib): $(library_lib)
$(ndielems_lib): $(capitogst_lib)

.PHONY: clean
clean:
	#$(RM) -r $(BUILD_DIR)

