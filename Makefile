export MAJOR = 1
export MINOR = 0
export RELEASE = 0

export VERSION = $(MAJOR).$(MINOR).$(RELEASE)

export PLATFORM = $(shell uname)
export MACHINE = $(shell uname -m)

ifeq ($(PLATFORM), Darwin)
  export PLATFORM_NAME = Mac
else
  export PLATFORM_NAME = Linux64
endif

all: libs neu

neu: libs
	(cd src/neu; $(MAKE))

libs:
	(cd lib/src; $(MAKE))

spotless:
	(cd src/neu; $(MAKE) spotless)
	(cd lib/src; $(MAKE) spotless)

clean:
	(cd src/neu; $(MAKE) clean)
	(cd lib/src; $(MAKE) clean)

fast:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1 NEU_FAST=1

release:
	$(MAKE) clean
	$(MAKE) -j $(BUILD_THREADS) NEU_RELEASE=1
