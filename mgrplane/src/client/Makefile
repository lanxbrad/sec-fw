TOP=../..
include $(TOP)/mk/env.mk


DIR=client
TARGET=$(TOP)/lib/lib$(DIR).a
BUILD_DIR=$(TOP)/build/$(DIR)
SOURCES:=$(wildcard *.c)
OBJS:=$(addprefix $(BUILD_DIR)/,$(subst .c,.o,$(SOURCES)))
DEPS:=$(addprefix $(BUILD_DIR)/,$(subst .c,.d,$(SOURCES)))

PROJ_CFLAGS += -I.  

.PHONY: all clean

all:$(TARGET)

include $(TOP)/mk/rule.mk
