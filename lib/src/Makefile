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
  export COMPILE_OBJC = clang++ -O2 -fPIC -DNDEBUG
else
  export COMPILE = clang++ -g -fPIC -std=c++11 $(STD_LIB)
  export COMPILE_OBJC = clang++ -g -fPIC
endif

export LINK = clang++ -L../ -fPIC $(STD_LIB)

COMPILE += -I$(NEU_HOME)/include -I$(NEU_HOME)/lib/src -I/usr/local/include -pthread -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -I$(HOME)/llvm-3.4/build-release/include -I$(HOME)/llvm-3.4/llvm/include

CPP_MODULES = NMObject.o NMGenerator.o NPQueue.o NNet.o NNModule.o NConcept.o

SUB_MODULES = nm/parse.tab.o nm/NMParser.o nm/parse.l.o npl/parse.tab.o npl/NPLParser.o npl/parse.l.o npl/NPLModule.o

ALL_MODULES = $(CPP_MODULES) $(C_MODULES) $(SUB_MODULES)

META_MODULES = NConcept_meta.h

all: $(META_MODULES) .depend libneu

meta:
	neu-meta NConcept.cpp

neu_m:
	(cd nm; $(MAKE))

neu_pl:
	(cd npl; $(MAKE))

.depend: $(CPP_MODULES:.o=.cpp)
	$(COMPILE) -MM $(CPP_MODULES:.o=.cpp) > .depend

-include .depend

LLVM_LINK = -L$(HOME)/llvm-3.4/build-release/lib -lLLVMAsmParser -lLLVMInstrumentation -lLLVMLinker -lLLVMBitReader -lLLVMDebugInfo -lLLVMJIT -lLLVMipo -lLLVMVectorize -lLLVMBitWriter -lLLVMTableGen -lLLVMX86Disassembler -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMX86AsmParser -lLLVMX86Desc -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMMCDisassembler -lLLVMMCParser -lLLVMInterpreter -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMMCJIT -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport -lLLVMOption -lcurses

libneu: $(CPP_MODULES) neu_m neu_pl

ifeq ($(PLATFORM), Darwin)
	$(LINK) -single_module -dynamiclib -o ../libneu.$(VERSION).dylib $(ALL_MODULES) -L$(NEU_HOME)/lib -lneu_core $(LLVM_LINK) -install_name $(NEU_HOME)/lib/libneu.$(VERSION).dylib
	(cd ..; ln -f -s libneu.$(VERSION).dylib libneu.dylib)
	(cd ..; ln -f -s libneu.$(VERSION).dylib libneu.$(MAJOR).dylib)
	(cd ..; ln -f -s libneu.$(VERSION).dylib libneu.$(MAJOR).$(MINOR).dylib)
	rm -f ../libneu-static.a
	ar rcs ../libneu-static.a $(ALL_MODULES)
else
	$(LINK) -shared -Wl,-soname,libneu.so -o ../libneu.so.$(VERSION) $(ALL_MODULES) -L$(NEU_HOME)/lib -lneu_core $(LLVM_LINK)
	(cd ..; ln -f -s libneu.so.$(VERSION) libneu.so)
	(cd ..; ln -f -s libneu.so.$(VERSION) libneu.so.$(MAJOR))
	(cd ..; ln -f -s libneu.so.$(VERSION) libneu.so.$(MAJOR).$(MINOR))
	rm -f ../libneu-static.a
	ar rcs ../libneu-static.a $(ALL_MODULES)
endif

$(CPP_MODULES): $(@.o=.cpp)
	$(COMPILE) -c $< -o $@

clean:
	rm -f .depend
	rm -f $(CPP_MODULES)
	rm -f $(C_MODULES)
	(cd nm; $(MAKE) clean)
	(cd npl; $(MAKE) clean)

spotless: clean
	rm -f ../libneu.*
	rm -f ../libneu-static.a	
	(cd nm; $(MAKE) spotless)
	(cd npl; $(MAKE) spotless)
