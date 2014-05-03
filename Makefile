export MAJOR = 1
export MINOR = 0
export RELEASE = 0

export VERSION = $(MAJOR).$(MINOR).$(RELEASE)

export PLATFORM = $(shell uname)
export MACHINE = $(shell uname -m)

all: neu

neu: libneu
	(cd src/neu; $(MAKE))

neu-meta: libneu_core
	(cd src/neu-meta; $(MAKE))

libneu_core:
	(cd lib/core-src; $(MAKE))

libneu: libneu_core neu-meta
	(cd lib/src; $(MAKE))

spotless:
	(cd src/neu; $(MAKE) spotless)
	(cd lib/core-src; $(MAKE) spotless)
	(cd lib/src; $(MAKE) spotless)

clean:
	(cd src/neu; $(MAKE) clean)
	(cd lib/core-src; $(MAKE) clean)
	(cd lib/src; $(MAKE) clean)

fast:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1 NEU_FAST=1

release:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1
