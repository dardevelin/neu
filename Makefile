include Makefile.defs

export MAJOR = 1
export MINOR = 0
export RELEASE = 0

export VERSION = $(MAJOR).$(MINOR).$(RELEASE)

all: neu

neu: libneu
	(cd src/bin/neu; $(MAKE))

neu-meta: libneu_core
	(cd src/bin/neu-meta; $(MAKE))

libneu_core:
	(cd src/lib/core; $(MAKE))

libneu: libneu_core neu-meta
	(cd src/lib/neu; $(MAKE))

concepts: libneu
	(cd app/concepts/src/lib; $(MAKE))

spotless:
	(cd src/bin/neu; $(MAKE) spotless)
	(cd src/bin/neu-meta; $(MAKE) spotless)
	(cd src/lib/core; $(MAKE) spotless)
	(cd src/lib/neu; $(MAKE) spotless)
	(cd app/concepts/src/lib; $(MAKE) spotless)

clean:
	(cd src/bin/neu; $(MAKE) clean)
	(cd src/bin/neu-meta; $(MAKE) clean)
	(cd src/lib/core; $(MAKE) clean)
	(cd src/lib/neu; $(MAKE) clean)
	(cd app/concepts/src/lib; $(MAKE) clean)

fast:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1 NEU_FAST=1

release:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1
