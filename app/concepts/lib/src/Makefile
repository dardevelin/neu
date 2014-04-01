export MAJOR = 1
export MINOR = 0
export RELEASE = 0

export VERSION = $(MAJOR).$(MINOR).$(RELEASE)

export PLATFORM = $(shell uname)
export MACHINE = $(shell uname -m)

ifeq ($(PLATFORM), Darwin)
  STD_LIB = -stdlib=libc++
endif

ifdef NEU_RELEASE
  export COMPILE = clang++ -O2 -fPIC -DNDEBUG -DNEU_FAST -std=c++11 $(STD_LIB)
else
  export COMPILE = clang++ -g -fPIC -std=c++11 $(STD_LIB)
endif

export LINK = clang++ -L../ -L$(NEU_HOME)/lib -fPIC $(STD_LIB)

COMPILE += -I$(NEU_HOME)/include -I$(NEU_HOME)/lib/src -I/usr/local/include -pthread -I$(NEU_HOME)/app/concepts/include

META_MODULES = Angle.o Count.o Index.o Length.o Real.o Volume.o Area.o Integer.o Mancala.o Truth.o

ALL_MODULES = $(META_MODULES)

all: .depend libconcepts

.depend: $(META_MODULES:.o=.cpp)
	$(COMPILE) -MM $(META_MODULES:.o=.cpp) -DMETA_GUARD > .depend

-include .depend

LIB = $(NEU_HOME)/lib

libconcepts: $(ALL_MODULES)

ifeq ($(PLATFORM), Darwin)
	$(LINK) -single_module -dynamiclib -o $(LIB)/libconcepts.$(VERSION).dylib $(ALL_MODULES) -L/usr/local/lib -lneu_core -lneu -install_name $(NEU_HOME)/lib/libconcepts.$(VERSION).dylib
	(cd $(LIB); ln -f -s libconcepts.$(VERSION).dylib libconcepts.dylib)
	(cd $(LIB); ln -f -s libconcepts.$(VERSION).dylib libconcepts.$(MAJOR).dylib)
	(cd $(LIB); ln -f -s libconcepts.$(VERSION).dylib libconcepts.$(MAJOR).$(MINOR).dylib)
	rm -f $(LIB)/libconcepts-static.a
	ar rcs $(LIB)/libconcepts-static.a $(ALL_MODULES)
else
	$(LINK) -shared -Wl,-soname,libconcepts.so -o $(LIB)/libconcepts.so.$(VERSION) $(ALL_MODULES) -L/usr/local/lib -lneu
	(cd $(LIB); ln -f -s libconcepts.so.$(VERSION) libconcepts.so)
	(cd $(LIB); ln -f -s libconcepts.so.$(VERSION) libconcepts.so.$(MAJOR))
	(cd $(LIB); ln -f -s libconcepts.so.$(VERSION) libconcepts.so.$(MAJOR).$(MINOR))
	rm -f $(LIB)/libconcepts-static.a
	ar rcs $(LIB)/libconcepts-static.a $(ALL_MODULES)
endif

$(META_MODULES): $(@.o=.cpp)
	neu-meta --I $(NEU_HOME)/app/concepts/include --class $(basename $<) $<
	$(COMPILE) -c $< -o $@

clean:
	rm -f .depend
	rm -f *_meta.h
	rm -f $(META_MODULES)

spotless: clean
	rm -f $(LIB)/libconcepts.*
	rm -f $(LIB)/libconcepts-static.a
