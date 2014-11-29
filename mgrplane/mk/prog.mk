include $(PROJ_DIR)/mk/env.mk
TOP = ..


PROG_SRV_LDFLAGS += 

srv:FORCE
	@echo [LD] $@
	$(Q) $(CC) -o $(PROJ_DIR)/bin/$@ $(PROG_SRV_LDFLAGS) 


PROG_CLIENT_LDFLAGS += -L$(PROJ_DIR)/third

client_start:FORCE
	@echo [LD] $@
	$(Q) $(CC) -o $(PROJ_DIR)/bin/$@ $(PROG_CLIENT_LDFLAGS)

client_end:FORCE
	@echo [LD] $@
	$(Q) $(CC) -o $(PROJ_DIR)/bin/$@ $(PROG_CLIENT_LDFLAGS)
	


.PHONY:srv FORCE