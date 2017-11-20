#
# Makefile to build aged related programs for any supported OSTYPE/MACHTYPE
#
# Phil Harvey - 12/2/98
#
# Notes:
#
# 1) Do a do a 'make clean' between builds of different aged versions
#    (normal, root and library versions), because these versions share
#    the same object files but are compiled with different options.
#
# 2) Set the AGED_OPTIONS environment variable to change the AGED
#    version compiler options.
#

ifndef OSTYPE
OSTYPE = $(shell uname -s | tr '[:upper:]' '[:lower:]')
endif
ifndef MACHTYPE
MACHTYPE = $(shell uname -m)
endif

PLATFORM      = $(OSTYPE)_$(MACHTYPE)$(AGED_MAKE_VER)

PROGS         = libaged.so

all:	$(PROGS)

# Set all these dependencies to files that don't exist,
# thereby forcing the appropriate make to be performed each time.

#nt
libaged.so: aged_
	@echo Done.

# Call make again for the proper platform

aged_:
	@echo ==== Making libaged.so ====
	make "VERFLAGS=$(AGED_OPTIONS)" -f Makefile.$(PLATFORM) libaged.so

# Utilities

clean:
	rm -f *.o *.C *.so *.log core
	@echo Clean.
