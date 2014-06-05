
#####################################################################
.PHONY : all clean help

all: 
	@$(MAKE) -C mt19937ar
	@$(MAKE) -C libconfig
	
clean:
	@echo "	CLEAN"
	@$(MAKE) -C mt19937ar $@
	@$(MAKE) -C libconfig $@

help:
	@echo "possible targets are 'all' 'clean' 'help'"
	@echo "'all'    - builds "
	@echo "'clean'  - deletes "
	@echo "'help'   - outputs this message"

