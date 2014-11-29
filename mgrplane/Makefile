TOP=.
include $(TOP)/mk/env.mk
export PROJ_DIR=$(PWD)
SRV_DIRS = $(TOP)/src/srv/srvcore
SRV_DIRS += $(TOP)/src/srv/srvnet
CLIENT_DIRS = $(TOP)/src/client

SRC_DIRS = $(SRV_DIRS) $(CLIENT_DIRS)

PROG_SRV_LDFLAGS = $(TOP)/lib/libsrvcore.a
PROG_SRV_LDFLAGS += $(TOP)/lib/libsrvnet.a
PROG_CLIENT_LDFLAGS = $(TOP)/lib/libclient.a


PROGS = $(TOP)/bin/srv  $(TOP)/bin/cli 

export PROG_SRV_LDFLAGS PROG_CLIENT_LDFLAGS


all: $(SRC_DIRS) $(PROGS)


$(SRC_DIRS):
	@mkdir -p $(TOP)/bin
	$(Q) $(MAKE) $(MAKE_DEBUG) --directory=$@

$(TOP)/bin/srv: $(SRV_DIRS)
	$(Q) $(MAKE) $(MAKE_DEBUG) -f $(TOP)/mk/prog.mk srv
	
$(TOP)/bin/cli: $(CLIENT_DIRS)
	$(Q) $(MAKE) $(MAKE_DEBUG) -f $(TOP)/mk/prog.mk cli


.PHONY: all $(SRC_DIRS) clean



clean:
	$(Q) for d in $(SRC_DIRS); \
	do \
	$(MAKE) clean --directory=$$d; \
	done \
 
	$(Q) -rm -f $(PROGS)


