export MAJOR = 1
export MINOR = 0
export RELEASE = 0

export VERSION = $(MAJOR).$(MINOR).$(RELEASE)

export PLATFORM = $(shell uname)
export MACHINE = $(shell uname -m)

export LIB = $(NEU_HOME)/lib

all: neu

neu: libneu
	(cd src/neu; $(MAKE))

neu-meta: libneu_core
	(cd src/neu-meta; $(MAKE))

libneu_core:
	(cd src/lib/core; $(MAKE))

libneu: libneu_core neu-meta
	(cd src/lib/neu; $(MAKE))

spotless:
	(cd src/neu; $(MAKE) spotless)
	(cd src/lib/core; $(MAKE) spotless)
	(cd src/lib/neu; $(MAKE) spotless)

clean:
	(cd src/neu; $(MAKE) clean)
	(cd src/lib/core; $(MAKE) clean)
	(cd src/lib/neu; $(MAKE) clean)

fast:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1 NEU_FAST=1

release:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1
