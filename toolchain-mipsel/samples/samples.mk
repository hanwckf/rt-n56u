# Makefile to manage samples

# ----------------------------------------------------------
# Build the list of available samples
CT_TOP_SAMPLES := $(patsubst $(CT_TOP_DIR)/samples/%/crosstool.config,%,$(sort $(wildcard $(CT_TOP_DIR)/samples/*/crosstool.config)))
CT_LIB_SAMPLES := $(filter-out $(CT_TOP_SAMPLES),$(patsubst $(CT_LIB_DIR)/samples/%/crosstool.config,%,$(sort $(wildcard $(CT_LIB_DIR)/samples/*/crosstool.config))))
CT_SAMPLES := $(shell echo $(sort $(CT_TOP_SAMPLES) $(CT_LIB_SAMPLES))  \
                      |$(sed) -r -e 's/ /\n/g;'                         \
                      |$(sed) -r -e 's/(.*),(.*)/\2,\1/;'               \
                      |sort                                             \
                      |$(sed) -r -e 's/(.*),(.*)/\2,\1/;'               \
               )

# If set to yes on command line, updates the sample configuration
# instead of just dumping the diff.
CT_UPDATE_SAMPLES := no

# ----------------------------------------------------------
# This part deals with the samples help entries

help-config::
	@echo  '  show-config        - Show a brief overview of current configuration'
	@echo  '  saveconfig         - Save current config as a preconfigured target'

help-samples::
	@echo  '  list-samples       - Prints the list of all samples (for scripting)'
	@echo  '  show-<sample>      - Show a brief overview of <sample> (list with list-samples)'
	@echo  '  <sample>           - Preconfigure crosstool-NG with <sample> (list with list-samples)'
	@echo  '  build-all[.#]      - Build *all* samples (list with list-samples) and install in'
	@echo  '                       $${CT_PREFIX} (set to ~/x-tools by default)'

help-distrib::
	@echo  '  check-samples      - Verify if samples need updates due to Kconfig changes'
	@echo  '  update-samples     - Regenerate sample configurations using the current Kconfig'

help-env::
	@echo  '  CT_PREFIX=dir      - Install samples in dir (see action "build-all", above).'

# ----------------------------------------------------------
# This part deals with printing samples information

# Print the details of current configuration
PHONY += show-config
show-config: .config
	@cp .config .config.sample
	@$(bash) $(CT_LIB_DIR)/scripts/show-config.sh -v current
	@rm -f .config.sample

# Prints the details of a sample
PHONY += $(patsubst %,show-%,$(CT_SAMPLES))
$(patsubst %,show-%,$(CT_SAMPLES)): show-%:
	@KCONFIG_CONFIG=$$(pwd)/.config.sample	\
	    CT_VCHECK=load \
        $(CONF) --defconfig=$(call sample_dir,$*)/crosstool.config   \
	            $(KCONFIG_TOP) >/dev/null
	@$(bash) $(CT_LIB_DIR)/scripts/show-config.sh -v $*
	@rm -f .config.sample

# Prints the details of all samples
PHONY += show-all
show-all: $(patsubst %,show-%,$(CT_SAMPLES))

# print the list of all available samples
PHONY += list-samples
list-samples: list-samples-pre $(patsubst %,list-%,$(CT_SAMPLES))
	@echo ' L (Local)       : sample was found in current directory'
	@echo ' G (Global)      : sample was installed with crosstool-NG'
	@echo ' X (EXPERIMENTAL): sample may use EXPERIMENTAL features'
	@echo ' B (BROKEN)      : sample is currently broken'
	@echo ' O (OBSOLETE)    : sample needs to be upgraded'

PHONY += list-samples-pre
list-samples-pre: FORCE
	@echo 'Status  Sample name'

PHONY += $(patsubst %,list-%,$(CT_SAMPLES))
$(patsubst %,list-%,$(CT_SAMPLES)): list-%:
	@KCONFIG_CONFIG=$$(pwd)/.config.sample	\
	    CT_VCHECK=load \
	    $(CONF) --defconfig=$(call sample_dir,$*)/crosstool.config   \
	            $(KCONFIG_TOP) >/dev/null
	@$(bash) $(CT_LIB_DIR)/scripts/show-config.sh $*
	@rm -f .config.sample

PHONY += list-samples-short
list-samples-short: FORCE
	$(SILENT)for s in $(CT_SAMPLES); do \
	    printf "%s\n" "$${s}";          \
	done

# Check one sample. Note that we are not loading but rather copying the defconfig;
# loading it while it contains some removed options would reset them to currently
# supported default values.
PHONY += $(patsubst %,check-%,$(CT_SAMPLES))
$(patsubst %,check-%,$(CT_SAMPLES)): check-%:
	@set -e; export KCONFIG_CONFIG=$$(pwd)/.config.sample;                          \
	 CT_NG_SAMPLE=$(call sample_dir,$*)/crosstool.config;                           \
	 cp $${CT_NG_SAMPLE} .config.sample;                                            \
	 CT_UPGRADECONFIG=yes                                                           \
	     $(bash) $(CT_LIB_DIR)/scripts/version-check.sh .config.sample &>/dev/null; \
	 CT_VCHECK=load $(CONF) -s --olddefconfig                                       \
	     $(KCONFIG_TOP) &>/dev/null;                                                \
	 CT_VCHECK=save $(CONF) -s --savedefconfig=$$(pwd)/.defconfig                   \
	     $(KCONFIG_TOP) &>/dev/null;                                                \
	 if ! cmp -s "$${CT_NG_SAMPLE}" .defconfig; then                                \
		if [ $(CT_UPDATE_SAMPLES) = yes ]; then                                     \
			echo "Updating $*";                                                     \
			mv .defconfig "$${CT_NG_SAMPLE}";                                       \
	    else                                                                        \
			echo "$* needs update:";                                                \
			diff -d -U 0 "$${CT_NG_SAMPLE}" .defconfig |tail -n +4;                 \
	    fi;                                                                         \
	 fi
	@rm -f .config.sample* .defconfig

check-samples: $(patsubst %,check-%,$(CT_SAMPLES))

update-samples:
	$(SILENT)$(MAKE) -rf $(CT_NG) check-samples CT_UPDATE_SAMPLES=yes

# ----------------------------------------------------------
# This part deals with saving/restoring samples

PHONY += samples
samples:
	@$(CT_ECHO) '  MKDIR $@'
	$(SILENT)mkdir -p $@

# Save a sample
saveconfig: .config samples
	$(SILENT)CT_VCHECK=save CONF=$(CONF) $(bash) $(CT_LIB_DIR)/scripts/saveSample.sh

# The 'sample_dir' function prints the directory in which the sample is,
# searching first in local samples, then in global samples
define sample_dir
$$( [ -d $(CT_TOP_DIR)/samples/$(1) ] && echo "$(CT_TOP_DIR)/samples/$(1)" || echo "$(CT_LIB_DIR)/samples/$(1)")
endef

# How we do recall one sample
PHONY += $(CT_SAMPLES)
$(CT_SAMPLES): check-config
	@$(CT_ECHO) "  CONF  $@"
	$(SILENT)CT_VCHECK=load $(CONF) --defconfig=$(call sample_dir,$@)/crosstool.config $(KCONFIG_TOP)
	@CT_VCHECK=strict $(bash) $(CT_LIB_DIR)/scripts/version-check.sh .config
	@echo
	@echo  '***********************************************************'
	@echo
	$(SILENT)( . $(call sample_dir,$@)/reported.by;                             \
	   echo "Initially reported by: $${reporter_name}";                         \
	   echo "URL: $${reporter_url}";                                            \
	   if [ -n "$${reporter_comment}" ]; then                                   \
	     echo  ;                                                                \
	     echo  "Comment:";                                                      \
	     printf "$${reporter_comment}\n";                                       \
	   fi;                                                                      \
	   echo  ;                                                                  \
	   echo  '***********************************************************';     \
	 )
	$(SILENT)if $(grep) -E '^CT_EXPERIMENTAL=y$$' .config >/dev/null 2>&1; then \
	   echo  ;                                                                  \
	   echo  'WARNING! This sample may enable experimental features.';          \
	   echo  '         Please be sure to review the configuration prior';       \
	   echo  '         to building and using your toolchain!';                  \
	   echo  'Now, you have been warned!';                                      \
	   echo  ;                                                                  \
	   echo  '***********************************************************';     \
	 fi
	@echo
	@echo  'Now configured for "$@"'

# ----------------------------------------------------------
# Some helper functions
__comma = ,
host_triplet = $(if $(findstring $(__comma),$(1)),$(firstword $(subst $(__comma), ,$(1))))
target_triplet = $(if $(findstring $(__comma),$(1)),$(word 2,$(subst $(__comma), ,$(1))),$(1))

# Create the rule to build a sample
# $1: sample name (target tuple, or host/target tuples separated by a comma)
define build_sample
	@$(CT_ECHO) '  CONF  $(1)'
	$(SILENT)CT_VCHECK=load $(CONF) -s --defconfig=$(call sample_dir,$(1))/crosstool.config $(KCONFIG_TOP)
	$(SILENT)[ -n "$(CT_PREFIX)" ] && $(sed) -i -r -e 's:^(CT_PREFIX=).*$$:\1"$(CT_PREFIX)":;' .config || :
	$(SILENT)$(sed) -i -r -e 's:^.*(CT_LOG_(WARN|INFO|EXTRA|DEBUG|ALL)).*$$:# \1 is not set:;' .config
	$(SILENT)$(sed) -i -r -e 's:^.*(CT_LOG_ERROR).*$$:\1=y:;' .config
	$(SILENT)$(sed) -i -r -e 's:^(CT_LOG_LEVEL_MAX)=.*$$:\1="ERROR":;' .config
	$(SILENT)$(CONF) -s --oldconfig $(KCONFIG_TOP)
	@$(CT_ECHO) '  BUILD $(1)'
	$(SILENT)h=$(call host_triplet,$(1)); \
	if [ -n "$${h}" -a -r ".build-all/PASS/$${h}/prefix" ]; then \
		PATH=`cat .build-all/PASS/$${h}/prefix`/bin:$${PATH}; \
	fi; \
	if $(MAKE) -rf $(CT_NG) V=0 build; then \
		status=PASS; \
	elif [ -e $(call sample_dir,$(1))/broken ]; then \
		status=XFAIL; \
	else \
		status=FAIL; \
	fi; \
	printf '\r  %-5s %s\n' $$status '$(1)'; \
	mkdir -p .build-all/$$status/$(1); \
	bzip2 < build.log > .build-all/$$status/$(1)/build.log.bz2; \
	if [ "$$status" = PASS ]; then \
		blddir=`$(bash) $(CT_LIB_DIR)/scripts/show-tuple.sh '$${CT_BUILD_TOP_DIR}'`; \
		[ -z "$(CT_PRESERVE_PASSED_BUILDS)" ] && rm -rf $${blddir}; \
		$(bash) $(CT_LIB_DIR)/scripts/show-tuple.sh '$${CT_PREFIX_DIR}' > .build-all/PASS/$(1)/prefix; \
	fi; \
	:
endef

# ----------------------------------------------------------
# Build samples for use (not regtest!)

# Check that PREFIX is set if building samples
ifneq ($(strip $(MAKECMDGOALS)),)
ifneq ($(strip $(filter $(patsubst %,build-%,$(CT_SAMPLES)) build-all,$(MAKECMDGOALS))),)

endif # MAKECMDGOALS contains a build sample rule
endif # MAKECMDGOALS != ""

# Build a single sample
$(patsubst %,build-%,$(CT_SAMPLES)): build-%:
	$(call build_sample,$*)

# Cross samples (build==host)
CT_SAMPLES_CROSS = $(strip $(foreach s,$(CT_SAMPLES),$(if $(findstring $(__comma),$(s)),, $(s))))
# Canadian cross (build!=host)
CT_SAMPLES_CANADIAN = $(strip $(foreach s,$(CT_SAMPLES),$(if $(findstring $(__comma),$(s)), $(s),)))

# Build all samples; first, build simple cross as canadian configurations may depend on
# build-to-host cross being pre-built.
build-all: build-all-pre $(patsubst %,build-%,$(CT_SAMPLES_CROSS) $(CT_SAMPLES_CANADIAN))
	@echo
	@if [ -d .build-all/PASS ]; then \
		echo 'Success:'; \
		(cd .build-all/PASS && ls | sed 's/^/  - /'); \
		echo; \
	fi
	@if [ -d .build-all/XFAIL ]; then \
		echo 'Expected failure:'; \
		(cd .build-all/XFAIL && ls | sed 's/^/  - /'); \
		echo; \
	fi
	@if [ -d .build-all/FAIL ]; then \
		echo 'Failure:'; \
		(cd .build-all/FAIL && ls | sed 's/^/  - /'); \
		echo; \
	fi
	@[ ! -d .build-all/FAIL ]

build-all-pre:
	@rm -rf .build-all

# Build all samples, overiding the number of // jobs per sample
build-all.%:
	$(SILENT)$(MAKE) -rf $(CT_NG) build-all CT_JOBS=$*

