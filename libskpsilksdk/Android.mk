# LOCAL_PATH := $(call my-dir)

# MY_LOCAL_PATH := $(LOCAL_PATH)
MY_LOCAL_PATH := $(LOCAL_PATH)/../libskpsilksdk


include $(CLEAR_VARS)

# $(warning $(call this-makefile))
# $(warning $(MY_LOCAL_PATH))

# LOCAL_LDLIBS += -lz -llog
LOCAL_CFLAGS += -Wall -O3
LOCAL_CPPFLAGS += -std=c++17
LOCAL_C_INCLUDES += $(MY_LOCAL_PATH) $(MY_LOCAL_PATH)/../include

LOCAL_MODULE := libskpsilksdk
libskpsilksdk_SOURCES := $(wildcard $(MY_LOCAL_PATH)/*.c)
LOCAL_SRC_FILES := $(libskpsilksdk_SOURCES)

include $(BUILD_STATIC_LIBRARY)
