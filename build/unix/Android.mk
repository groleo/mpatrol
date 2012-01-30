LOCAL_PATH:=$(call my-dir)

MP_TOP:=../..
S:=$(MP_TOP)/src
T:=$(MP_TOP)/tools
TF:=$(MP_TOP)/tests/fail

SRC_FILES:=\
	  $(S)/list.c $(S)/tree.c $(S)/slots.c $(S)/utils.c $(S)/memory.c $(S)/heap.c $(S)/alloc.c $(S)/info.c \
	  $(S)/stack.c $(S)/addr.c $(S)/strtab.c $(S)/symbol.c $(S)/signals.c $(S)/diag.c $(S)/option.c \
	  $(S)/leaktab.c $(S)/profile.c $(S)/trace.c $(S)/inter.c $(S)/malloc.c $(S)/cplus.c $(S)/version.c \
	  $(T)/dbmalloc.c $(T)/dmalloc.c $(T)/heapdiff.c $(T)/mgauge.c $(T)/mtrace.c \
	  $(S)/mutex.c

CFLAGS:=-DHAVE_CONFIG_H

C_INCLUDES:=\
	external/mpatrol/src \
	external/mpatrol/tools \
	external/elfutils/libelf \
	external/libunwind/include \

include $(CLEAR_VARS)
LOCAL_SRC_FILES:=$(SRC_FILES)
LOCAL_CFLAGS:=$(CFLAGS)
LOCAL_C_INCLUDES:=$(C_INCLUDES)
LOCAL_MODULE_TAGS := eng
LOCAL_STATIC_LIBRARIES:=libelf libunwind
LOCAL_MODULE=libmpatrol
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:=$(C_INCLUDES)
LOCAL_CFLAGS:=$(CFLAGS)
LOCAL_SRC_FILES:=$(SRC_FILES)
LOCAL_MODULE_TAGS := eng
LOCAL_STATIC_LIBRARIES:=libelf
LOCAL_MODULE=libmpatrol
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES:=$(S)/mpatrol.c $(S)/getopt.c $(S)/version.c
LOCAL_MODULE=mpatrol
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES:=$(S)/mprof.c $(S)/getopt.c $(S)/version.c $(S)/tree.c $(S)/list.c $(S)/graph.c
LOCAL_MODULE=mprof
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES:=$(S)/mptrace.c $(S)/getopt.c $(S)/version.c $(S)/tree.c $(S)/slots.c $(S)/utils.c
LOCAL_MODULE=mptrace
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES:=$(S)/mleak.c $(S)/getopt.c $(S)/version.c $(S)/tree.c
LOCAL_MODULE=mleak
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_CFLAGS:=$(CFLAGS)
LOCAL_C_INCLUDES:=$(C_INCLUDES)
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES:=$(TF)/test1.c
LOCAL_MODULE=test1
LOCAL_SHARED_LIBRARIES:=libmpatrol
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_CFLAGS:=$(CFLAGS) -O2
LOCAL_C_INCLUDES:=$(C_INCLUDES)
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES:=$(TF)/test10.c
LOCAL_MODULE=test10
#LOCAL_SHARED_LIBRARIES:=libmpatrol
LOCAL_STRIP_MODULE:=false
include $(BUILD_EXECUTABLE)
