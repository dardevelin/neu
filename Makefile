include Makefile.defs

export MAJOR = 1
export MINOR = 0
export RELEASE = 0

export VERSION = $(MAJOR).$(MINOR).$(RELEASE)

export PLATFORM = $(shell uname)
export MACHINE = $(shell uname -m)

export NEU_LIB = $(NEU_HOME)/lib

ifeq ($(PLATFORM), Darwin)
  STD_LIB = -stdlib=libc++
endif

export COMPILE = $(CXX) -std=c++11 $(STD_LIB) -I$(NEU_HOME)/include $(INCLUDE_DIRS) -fPIC

export COMPILE_C = $(CC) -fPIC -I$(NEU_HOME)/include

ifdef NEU_RELEASE
  COMPILE += $(OPT) -DNDEBUG
  COMPILE_C += $(OPT) -DNDEBUG
else
  COMPILE += -g
  COMPILE_C += -g
endif

export LINK = $(CXX) $(STD_LIB)

all: neu

neu: libneu
	(cd src/bin/neu; $(MAKE))

neu-meta: libneu_core
	(cd src/bin/neu-meta; $(MAKE))

libneu_core:
	(cd src/lib/core; $(MAKE))

libneu: libneu_core neu-meta
	(cd src/lib/neu; $(MAKE))

spotless:
	(cd src/bin/neu; $(MAKE) spotless)
	(cd src/bin/neu-meta; $(MAKE) spotless)
	(cd src/lib/core; $(MAKE) spotless)
	(cd src/lib/neu; $(MAKE) spotless)

clean:
	(cd src/bin/neu; $(MAKE) clean)
	(cd src/bin/neu-meta; $(MAKE) clean)
	(cd src/lib/core; $(MAKE) clean)
	(cd src/lib/neu; $(MAKE) clean)

fast:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1 NEU_FAST=1

release:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1
