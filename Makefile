########################################################################
##
## Raithlin Consulting Inc. p2pmem test suite
## Copyright (c) 2017, Raithlin Consulting Inc.
##
## This program is free software; you can redistribute it and/or modify it
## under the terms and conditions of the GNU General Public License,
## version 2, as published by the Free Software Foundation.
##
## This program is distributed in the hope it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
## more details.
##
########################################################################

OBJDIR=build

DESTDIR ?=

LIBARGCONFIGDIR=libargconfig

CPPFLAGS=-Iinc -Ibuild
CFLAGS=-std=gnu99 -g -O2 -fPIC -Wall -Werror -I$(LIBARGCONFIGDIR)/inc
DEPFLAGS= -MT $@ -MMD -MP -MF $(OBJDIR)/$*.d

EXE=p2pmem-test
SRCS=$(wildcard src/*.c)
OBJS=$(addprefix $(OBJDIR)/, $(patsubst %.c,%.o, $(SRCS)))

ifneq ($(V), 1)
Q=@
MAKEFLAGS+=-s --no-print-directory
else
NQ=:
endif

compile: $(EXE)

clean:
	@$(NQ) echo "  CLEAN  $(EXE)"
	$(Q)rm -rf $(EXE) build *~ ./src/*~
	$(Q)$(MAKE) -C $(LIBARGCONFIGDIR) clean

$(OBJDIR)/version.h $(OBJDIR)/version.mk: FORCE $(OBJDIR)
	@$(SHELL_PATH) ./VERSION-GEN
$(OBJDIR)/src/main.o: $(OBJDIR)/version.h
-include $(OBJDIR)/version.mk

$(OBJDIR):
	$(Q)mkdir -p $(OBJDIR)/src

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	@$(NQ) echo "  CC     $<"
	$(Q)$(COMPILE.c) $(DEPFLAGS) $< -o $@

$(LIBARGCONFIGDIR)/libargconfig.a: FORCE
	@$(NQ) echo "  MAKE   $@"
	$(Q)$(MAKE) -C $(LIBARGCONFIGDIR)

$(EXE): $(OBJS) $(LIBARGCONFIGDIR)/libargconfig.a
	@$(NQ) echo "  LD     $@"
	$(Q)$(LINK.o) $^ $(LDLIBS) -o $@

.PHONY: clean compile FORCE

-include $(patsubst %.o,%.d,$(OBJS))
