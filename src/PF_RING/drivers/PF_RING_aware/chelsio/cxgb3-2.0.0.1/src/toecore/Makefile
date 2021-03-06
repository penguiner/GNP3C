# Makefile for Chelsio T3 toecore driver.
# Copyright (c) 2006-2008 Chelsio Communications, Inc.
SHELL = /bin/bash

# The top-level makefile defines required variables and flags.
ifneq ($(shell [[ $(MAKELEVEL) -ge 1 ]] && echo 1),1)
  $(error Please use the top-level Makefile to build this driver)
endif

# Includes.
EXTRA_CFLAGS += $(FLAGS)
EXTRA_CFLAGS += -I$(srcdir)/include
EXTRA_CFLAGS += -I$(KSRC)/include
EXTRA_CFLAGS += -I$(KSRC)
EXTRA_CFLAGS += -I.

CFILES  = module_support.c offload.c toedev.c
ifneq ($(disable_bonding),1)
  CFILES += toe_bonding.c
endif
TARGET  = toecore.o
CLEAN_FILES := $(wildcard *.c)
CLEAN_FILES := $(CLEAN_FILES:.c=.o)

lib_path     := $(PREFIX)/lib/modules/$(utsrelease)
module_path   = updates/kernel/drivers/net/offload
install_path := $(lib_path)/$(module_path)

# Define grep error output to NULL, since -s is not portable.
grep = grep 2>/dev/null

ifeq ($(kseries),2.4)
  $(error offload driver is not supported on 2.4 series kernel)
  driver := $(TARGET)
  $(TARGET): $(filter-out $(TARGET),$(CFILES:.c=.o))
	$(LD) -r $^ -o $@
else
  driver := $(TARGET:.o=.ko)
endif

ifneq ($(modulesymfile),)
  override symverfile = symverfile="$(topdir)/$(modulesymfile) \
                                    -o $(drvdir)/$(modulesymfile)"
else
  override symverfile =
endif

obj-m := $(TARGET)
$(TARGET:.o=)-objs := $(CFILES:.c=.o)

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: default
default: symlink build

.PHONY: symlink
symlink:
	@[ -L module_support.c ] && /bin/rm -f module_support.c; true
	@function error() { echo "*** ERROR: Compiling $(TARGET:.o=)" >&2;\
	                    [ -n "$$1" ] && echo "*** ERROR: $$1" >&2;\
			    exit 1; };\
	 file=`for ver in $(kversions); do\
	         grep -l "\\$$SUPPORTED KERNEL $$ver\\$$\$$" module_support/*.c\
		 2>/dev/null && break;\
	       done`;\
	 if [ -z "$$file" ]; then\
	   error "Kernel version not supported.";\
	 fi;\
	 if ! ln -sf $$file module_support.c; then\
	   error "Failed to create symlink.";\
	 fi;

.PHONY: build
build:
	@$(MAKE) $(symverfile) -C $(KOBJ) SUBDIRS=$(shell pwd) modules

.PHONY: install
install:
	@install -D $(verbose) -m 644 $(driver) $(install_path)/$(driver)

.PHONY: uninstall
uninstall:
	@-if [ -n "$(verbose)" ]; then \
	    echo "Removing $(install_path)/$(driver)";\
	  fi;\
	  /bin/rm -f $(install_path)/$(driver) 2>/dev/null;

.PHONY: clean
clean:
	-/bin/rm -rf $(driver) $(TARGET) $(TARGET:.o=.mod.c) \
	             $(TARGET:.o=.mod.o) $(CLEAN_FILES) \
		     .*cmd .tmp_versions *.symvers
