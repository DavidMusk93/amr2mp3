LOCAL_PATH := $(call my-dir)

include $(LOCAL_PATH)/../libmp3lame/Android.mk
include $(LOCAL_PATH)/../libskpsilksdk/Android.mk

include $(CLEAR_VARS)

# $(info "user-defined:$(TEST)")

ifeq (1,$(TEST))
FILTER_SRC := JniHelper.cc SafeString.cc
else
HIDE_FLAG := -fvisibility=hidden
FILTER_SRC := test.cc
endif
FILTER_SRC += StringConverter.cc

LOCAL_LDLIBS += -lz -llog -lm
LOCAL_CFLAGS += -fPIE -DHAVE_MEMCPY=1 -DSTDC_HEADERS=1 -Dieee754_float32_t=float $(HIDE_FLAG)
LOCAL_CPPFLAGS += -std=c++17
LOCAL_C_INCLUDES += $(LOCAL_PATH) $(LOCAL_PATH)/../include $(LOCAL_PATH)/../libmp3lame $(LOCAL_PATH)/../libskpsilksdk
LOCAL_STATIC_LIBRARIES += libmp3lame libskpsilksdk
ifeq (1,$(TEST))
LOCAL_MODULE := amr2mp3
else
LOCAL_MODULE := silk
endif
LOCAL_SRC_FILES := $(wildcard *.c) $(filter-out $(FILTER_SRC),$(wildcard *.cc))
$(info $(LOCAL_SRC_FILES))

ifeq (1,$(TEST))
include $(BUILD_EXECUTABLE)
else
include $(BUILD_SHARED_LIBRARY)
endif
