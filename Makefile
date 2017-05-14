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

CPPFLAGS=-Iinc -Ibuild
CFLAGS=-g -O2 -fPIC -Wall
DEPFLAGS= -MT $@ -MMD -MP -MF $(OBJDIR)/$*.d

EXE=p2pmem-test
SRCS=$(wildcard src/*.c)
OBJS=$(addprefix $(OBJDIR)/, $(patsubst %.c,%.o, $(LIB_SRCS)))

ifneq ($(V), 1)
Q=@
else
NQ=:
endif

ifeq ($(W), 1)
CFLAGS += -Werror
endif

$(OBJDIR)/version.h $(OBJDIR)/version.mk: FORCE $(OBJDIR)
	@$(SHELL_PATH) ./VERSION-GEN
$(OBJDIR)/cli/main.o: $(OBJDIR)/version.h
-include $(OBJDIR)/version.mk

$(OBJDIR):
	$(Q)mkdir -p $(OBJDIR)/cli $(OBJDIR)/lib

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	@$(NQ) echo "  CC    $<"
	$(Q)$(COMPILE.c) $(DEPFLAGS) $< -o $@

$(EXE): $(OBJS) libargconfig.a
	@$(NQ) echo "  LD    $@"
	$(Q)$(LINK.o) $^ $(LDLIBS) -o $@

.PHONY: clean compile
.PHONY: FORCE

-include $(patsubst %.o,%.d,$(LIB_OBJS))

CFLAGS += -g -std=gnu99

clean:
	rm -rf src/*.o *.a
