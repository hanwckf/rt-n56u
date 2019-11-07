# Makefile for the scripts/ sub-directory

# Here, we can update the config.* scripts.
# If we're in CT_LIB_DIR, then CT_LIB_DIR == CT_TOP_DIR, and we can update those
# scripts for later inclusion mainline. If CT_LIB_DIR != CT_TOP_DIR, then those
# scripts are downloaded only for use in CT_TOP_DIR.

# ----------------------------------------------------------
# The tools help entry

help-distrib::
	@echo  '  updatetools        - Update the config tools'

# ----------------------------------------------------------
# Where to get tools from, and where to store them into
# The tools are: config.guess and config.sub

CONFIG_SUB_SRC="http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD"
CONFIG_SUB_DEST=scripts/config.sub
CONFIG_GUESS_SRC="http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD"
CONFIG_GUESS_DEST=scripts/config.guess

PHONY += updatetools
updatetools: $(CONFIG_SUB_DEST) $(CONFIG_GUESS_DEST)

# ----------------------------------------------------------
# How to retrieve the tools

ifneq ($(strip $(V)),2)
  wget_silent_opt = -o /dev/null
  curl_silent_opt = --silent
endif

ifneq ($(CT_WGET),)
download_cmd = $(CT_WGET) --passive-ftp $(wget_silent_opt) -O $@
else
ifneq ($(CT_CURL),)
download_cmd = $(CT_CURL) --ftp-pasv $(curl_silent_opt) -o $@
else
download_cmd = $(error wget or curl needed for downloads)
endif
endif

PHONY += scripts
scripts:
	@$(CT_ECHO) '  MKDIR $@'
	$(SILENT)mkdir -p $@

$(CONFIG_SUB_DEST): scripts FORCE
	@$(CT_ECHO) '  DOWNLOAD  $@'
	$(SILENT)$(download_cmd) $(CONFIG_SUB_SRC)
	$(SILENT)chmod u+rwx,go+rx-w $@

$(CONFIG_GUESS_DEST): scripts FORCE
	@$(CT_ECHO) '  DOWNLOAD  $@'
	$(SILENT)$(download_cmd) $(CONFIG_GUESS_SRC)
	$(SILENT)chmod u+rwx,go+rx-w $@

# ----------------------------------------------------------
# Clean up the mess

distclean::
	@$(CT_ECHO) "  CLEAN scripts"
	$(SILENT)[ $(CT_TOP_DIR) = $(CT_LIB_DIR) ] || rm -rf $(CT_TOP_DIR)/scripts
