# LOCAL_PATH := $(call my-dir)

MY_LOCAL_PATH := $(LOCAL_PATH)/../libmp3lame


include $(CLEAR_VARS)

# $(warning $(call this-makefile))
# $(warning $(MY_LOCAL_PATH))

# LOCAL_LDLIBS += -lz -llog
LOCAL_CFLAGS += -fPIE -Wall -O3 -DHAVE_MEMCPY=1 -DSTDC_HEADERS=1 -Dieee754_float32_t=float
LOCAL_CPPFLAGS += -std=c++17
LOCAL_C_INCLUDES += $(MY_LOCAL_PATH) $(MY_LOCAL_PATH)/../include

LOCAL_MODULE := libmp3lame
libmp3lame_SOURCES := $(wildcard $(MY_LOCAL_PATH)/*.c)
# $(warning $(libmp3lame_SOURCES))
LOCAL_SRC_FILES := $(libmp3lame_SOURCES)

include $(BUILD_STATIC_LIBRARY)
