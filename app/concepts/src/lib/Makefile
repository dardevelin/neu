COMPILE += -I$(NEU_HOME)/lib/src -I$(NEU_HOME)/app/concepts/include

LINK += -L$(NEU_LIB)

META_MODULES = Angle.o Count.o Index.o Length.o Real.o Volume.o Area.o Integer.o Mancala.o Truth.o

ALL_MODULES = $(META_MODULES)

all: .depend libconcepts

.depend: $(META_MODULES:.o=.cpp)
	$(COMPILE) -MM $(META_MODULES:.o=.cpp) -DMETA_GUARD > .depend

-include .depend

libconcepts: $(ALL_MODULES)

ifeq ($(PLATFORM), Darwin)
	$(LINK) -single_module -dynamiclib -o $(NEU_LIB)/libconcepts.$(VERSION).dylib $(ALL_MODULES) -L/usr/local/lib -lneu_core -lneu -install_name $(NEU_HOME)/lib/libconcepts.$(VERSION).dylib
	(cd $(NEU_LIB); ln -f -s libconcepts.$(VERSION).dylib libconcepts.dylib)
	(cd $(NEU_LIB); ln -f -s libconcepts.$(VERSION).dylib libconcepts.$(MAJOR).dylib)
	(cd $(NEU_LIB); ln -f -s libconcepts.$(VERSION).dylib libconcepts.$(MAJOR).$(MINOR).dylib)
	rm -f $(NEU_LIB)/libconcepts-static.a
	ar rcs $(NEU_LIB)/libconcepts-static.a $(ALL_MODULES)
else
	$(LINK) -shared -Wl,-soname,libconcepts.so -o $(NEU_LIB)/libconcepts.so.$(VERSION) $(ALL_MODULES) -L/usr/local/lib -lneu
	(cd $(NEU_LIB); ln -f -s libconcepts.so.$(VERSION) libconcepts.so)
	(cd $(NEU_LIB); ln -f -s libconcepts.so.$(VERSION) libconcepts.so.$(MAJOR))
	(cd $(NEU_LIB); ln -f -s libconcepts.so.$(VERSION) libconcepts.so.$(MAJOR).$(MINOR))
	rm -f $(NEU_LIB)/libconcepts-static.a
	ar rcs $(NEU_LIB)/libconcepts-static.a $(ALL_MODULES)
endif

$(META_MODULES): $(@.o=.cpp)
	neu-meta -I $(NEU_HOME)/app/concepts/include -class $(basename $<) $<
	$(COMPILE) -c $< -o $@

clean:
	rm -f .depend
	rm -f *_meta.h
	rm -f $(META_MODULES)

spotless: clean
	rm -f $(NEU_LIB)/libconcepts.*
	rm -f $(NEU_LIB)/libconcepts-static.a
