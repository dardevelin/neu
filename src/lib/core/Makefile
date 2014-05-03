export COMPILE_C = clang -O2 -fPIC -I$(NEU_HOME)/include

ifeq ($(PLATFORM), Darwin)
  STD_LIB = -stdlib=libc++
endif

ifdef NEU_RELEASE
  export COMPILE = clang++ -O2 -fPIC -DNDEBUG -DNEU_FAST -std=c++11 $(STD_LIB)
  export COMPILE_OBJC = clang++ -O2 -fPIC -DNDEBUG
else
  export COMPILE = clang++ -g -fPIC -std=c++11 $(STD_LIB)
  export COMPILE_OBJC = clang++ -g -fPIC
endif

export LINK = clang++ -L$(LIB) -fPIC $(STD_LIB)

COMPILE += -I$(NEU_HOME)/include -I$(NEU_HOME)/lib/src -I/usr/local/include -pthread

C_MODULES = compress.o

CPP_MODULES = global.o nreal.o nstr.o nvar.o NError.o NThread.o NRegex.o NClass.o NObjectBase.o NObject.o NCommand.o NResourceManager.o NSys.o NProgram.o NMLGenerator.o NRandom.o NProc.o NEncoder.o NCommunicator.o NServer.o NBroker.o

SUB_MODULES = nml/parse.tab.o nml/NMLParser.o nml/parse.l.o

ALL_MODULES = $(CPP_MODULES) $(C_MODULES) $(SUB_MODULES)

all: .depend libneu_core

neu_ml:
	(cd nml; $(MAKE))

.depend: $(CPP_MODULES:.o=.cpp)
	$(COMPILE) -MM $(CPP_MODULES:.o=.cpp) > .depend

-include .depend

libneu_core: $(CPP_MODULES) $(C_MODULES) neu_ml

ifeq ($(PLATFORM), Darwin)
	$(LINK) -single_module -dynamiclib -o $(LIB)/libneu_core.$(VERSION).dylib $(ALL_MODULES) -L/usr/local/lib -lgmp -lmpfr -lz -install_name $(NEU_HOME)/lib/libneu_core.$(VERSION).dylib
	(cd $(LIB); ln -f -s libneu_core.$(VERSION).dylib libneu_core.dylib)
	(cd $(LIB); ln -f -s libneu_core.$(VERSION).dylib libneu_core.$(MAJOR).dylib)
	(cd $(LIB); ln -f -s libneu_core.$(VERSION).dylib libneu_core.$(MAJOR).$(MINOR).dylib)
	rm -f $(LIB)/libneu_core_static.a
	ar rcs $(LIB)/libneu_core_static.a $(ALL_MODULES)
else
	$(LINK) -shared -Wl,-soname,libneu_core.so -o $(LIB)/libneu_core.so.$(VERSION) $(ALL_MODULES) -L/usr/local/lib -lgmp -lmpfr -lz
	(cd $(LIB); ln -f -s libneu_core.so.$(VERSION) libneu_core.so)
	(cd $(LIB); ln -f -s libneu_core.so.$(VERSION) libneu_core.so.$(MAJOR))
	(cd $(LIB); ln -f -s libneu_core.so.$(VERSION) libneu_core.so.$(MAJOR).$(MINOR))
	rm -f $(LIB)/libneu_core_static.a
	ar rcs $(LIB)/libneu_core_static.a $(ALL_MODULES)
endif

compress.o: compress.c $(NEU_HOME)/include/neu/compress.h
	$(COMPILE_C) -c compress.c -o compress.o

$(CPP_MODULES): $(@.o=.cpp)
	$(COMPILE) -c $< -o $@

clean:
	rm -f .depend
	rm -f $(CPP_MODULES)
	rm -f $(C_MODULES)
	(cd nml; $(MAKE) clean)

spotless: clean
	rm -f $(LIB)/libneu_core.*
	rm -f $(LIB)/libneu_core_static.a
	(cd nml; $(MAKE) spotless)
