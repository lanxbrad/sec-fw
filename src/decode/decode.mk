#  standard component Makefile header
sp              :=  $(sp).x
dirstack_$(sp)  :=  $(d)
d               :=  $(dir)



#  component specification

LIBRARY_DECODE := $(OBJ_DIR)/libdecode.a


LIBDECODE_OBJ_$(d)  :=  \
	$(OBJ_DIR)/decode.o \
	$(OBJ_DIR)/decode-ethernet.o \
	$(OBJ_DIR)/decode-vlan.o \
	$(OBJ_DIR)/decode-ipv4.o \
	$(OBJ_DIR)/decode-tcp.o \
	$(OBJ_DIR)/decode-udp.o
	

INCLUDE_DIR := \
	-I$(d) \
	-I$(OCTEON_ROOT)/sec-fw/src/include


$(LIBDECODE_OBJ_$(d)):  CFLAGS_LOCAL := -O2 -g -W -Wall -Wno-unused-parameter -Wundef -G0 $(INCLUDE_DIR)
$(LIBDECODE_OBJ_$(d)):  CFLAGS_GLOBAL := $(filter-out -fprofile-%,$(CFLAGS_GLOBAL))	


#  standard component Makefile rules

LIBDECODE_DEPS_$(d)   :=  $(LIBDECODE_OBJ_$(d):.o=.d)

LIBS_LIST   :=  $(LIBS_LIST) $(LIBRARY_DECODE)

LIBDECODE_CLEAN_LIST  :=  $(LIBDECODE_CLEAN_LIST) $(LIBDECODE_OBJ_$(d)) $(LIBDECODE_DEPS_$(d)) $(LIBRARY_DECODE)

-include $(LIBDECODE_DEPS_$(d))

$(LIBRARY_DECODE): $(LIBDECODE_OBJ_$(d))
	$(AR) -cr $@ $^

$(OBJ_DIR)/%.o:	$(d)/%.c
	$(COMPILE)


$(OBJ_DIR)/%.o:	$(d)/%.S
	$(ASSEMBLE)

#  standard component Makefile footer

d   :=  $(dirstack_$(sp))
sp  :=  $(basename $(sp))