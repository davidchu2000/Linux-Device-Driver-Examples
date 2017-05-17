LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= expdevice

LOCAL_SRC_FILES := \
    expdevice.cpp 

LOCAL_SHARED_LIBRARIES :=


LOCAL_CFLAGS := -Werror $(event_flag)

include $(BUILD_EXECUTABLE)

include $(call first-makefiles-under,$(LOCAL_PATH))
