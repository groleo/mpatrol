LOCAL_PATH:=$(call my-dir)

TOP:=../../
S:=$(TOP)/src
T:=$(TOP)/tools
TF:=$(TOP)/tests/fail

SRC_FILES:=\
	  $(S)/list.c $(S)/tree.c $(S)/slots.c $(S)/utils.c $(S)/memory.c $(S)/heap.c $(S)/alloc.c $(S)/info.c \
	  $(S)/stack.c $(S)/addr.c $(S)/strtab.c $(S)/symbol.c $(S)/signals.c $(S)/diag.c $(S)/option.c \
	  $(S)/leaktab.c $(S)/profile.c $(S)/trace.c $(S)/inter.c $(S)/malloc.c $(S)/cplus.c $(S)/version.c \
	  $(T)/dbmalloc.c $(T)/dmalloc.c $(T)/heapdiff.c $(T)/mgauge.c $(T)/mtrace.c

CFLAGS:=-DHAVE_CONFIG_H

C_INCLUDES:=\
	external/mpatrol/src \
	external/mpatrol/tools \
	external/elfutils/libelf \

include $(CLEAR_VARS)
LOCAL_SRC_FILES:=$(SRC_FILES)
LOCAL_CFLAGS:=$(CFLAGS)
LOCAL_C_INCLUDES:=$(C_INCLUDES)
LOCAL_MODULE_TAGS := eng
LOCAL_STATIC_LIBRARIES:=libelf
LOCAL_MODULE=mpatrol
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:=$(C_INCLUDES)
LOCAL_CFLAGS:=$(CFLAGS)
LOCAL_SRC_FILES:=$(SRC_FILES)
LOCAL_MODULE_TAGS := eng
LOCAL_STATIC_LIBRARIES:=libelf
LOCAL_MODULE=mpatrol
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
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_CFLAGS:=$(CFLAGS) -O0
LOCAL_C_INCLUDES:=$(C_INCLUDES)
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES:=$(TF)/test10.c
LOCAL_MODULE=test10
include $(BUILD_EXECUTABLE)
