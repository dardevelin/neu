export MAJOR = 1
export MINOR = 0
export RELEASE = 0

export VERSION = $(MAJOR).$(MINOR).$(RELEASE)

export PLATFORM = $(shell uname)
export MACHINE = $(shell uname -m)

CXX_OPTS = -I$(NEU_HOME)/include -I$(NEU_HOME)/lib/src -I/usr/local/include -pthread
export COMPILE_C = clang -O2 -fPIC -I$(NEU_HOME)/include

ifdef NEU_RELEASE
  export COMPILE = clang++ -O2 -fPIC -DNDEBUG -DNEU_FAST -std=c++11 -stdlib=libc++
  export COMPILE_OBJC = clang++ -O2 -fPIC -DNDEBUG
else
  export COMPILE = clang++ -g -fPIC -std=c++11 -stdlib=libc++
  export COMPILE_OBJC = clang++ -g -fPIC
endif

ifeq ($(PLATFORM), Darwin)
  export LINK = clang++ -L../ -fPIC -stdlib=libc++
else
  export LINK = clang++ -L../ -fPIC -stdlib=libc++
endif

CXX_OPTS += -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -I$(HOME)/llvm-3.4/build-release/include -I$(HOME)/llvm-3.4/llvm/include

COMPILE += $(CXX_OPTS)

C_MODULES = compress.o

CPP_MODULES = global.o nreal.o nstr.o nvar.o NError.o NThread.o NRegex.o NClass.o NObjectBase.o NObject.o NCommand.o NResourceManager.o NSys.o NProgram.o NMObject.o NMLGenerator.o NMGenerator.o NRandom.o NMetaGenerator.o NPQueue.o NProcTask.o NProc.o NNet.o NNModule.o

CORE_MODULES = nml/parse.tab.o nml/NMLParser.o nml/parse.l.o nm/parse.tab.o nm/NMParser.o nm/parse.l.o npl/parse.tab.o npl/NPLParser.o npl/parse.l.o npl/NPLModule.o

ALL_MODULES = $(CPP_MODULES) $(C_MODULES) $(CORE_MODULES)

all: .depend libneu

neu_ml:
	(cd nml; $(MAKE))

neu_m:
	(cd nm; $(MAKE))

neu_pl:
	(cd npl; $(MAKE))

.depend: $(CPP_MODULES:.o=.cpp)
	$(COMPILE) -MM $(CPP_MODULES:.o=.cpp) > .depend

-include .depend

LLVM_LINK = -L$(HOME)/llvm-3.4/build-release/lib -lLLVMAsmParser -lLLVMInstrumentation -lLLVMLinker -lLLVMBitReader -lLLVMDebugInfo -lLLVMJIT -lLLVMipo -lLLVMVectorize -lLLVMBitWriter -lLLVMTableGen -lLLVMX86Disassembler -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMX86AsmParser -lLLVMX86Desc -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMMCDisassembler -lLLVMMCParser -lLLVMInterpreter -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMMCJIT -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport -lLLVMOption -lclangFrontend -lclangSerialization -lclangDriver -lclangTooling -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangLex -lclangBasic

LIB_DEPENDS = $(CPP_MODULES) $(C_MODULES) neu_ml neu_m neu_pl

libneu: $(LIB_DEPENDS)

ifeq ($(PLATFORM), Darwin)
	$(LINK) -single_module -dynamiclib -o ../libneu.$(VERSION).dylib $(ALL_MODULES) -L/usr/local/lib -lgmp -lmpfr -ledit -ldl -lutil -lpthread -lcurses -lz -lcrypto -lobjc -framework Foundation -framework Cocoa -framework OpenGL -framework OpenCL -framework GLUT $(LLVM_LINK) -L$(HOME)/bullet/lib -lBulletCollision -lBulletSoftBody -lBulletDynamics -lLinearMath -install_name $(NEU_HOME)/lib/libneu.$(VERSION).dylib
	(cd ..; ln -f -s libneu.$(VERSION).dylib libneu.dylib)
	(cd ..; ln -f -s libneu.$(VERSION).dylib libneu.$(MAJOR).dylib)
	(cd ..; ln -f -s libneu.$(VERSION).dylib libneu.$(MAJOR).$(MINOR).dylib)
	rm -f ../libneu-static.a
	ar rcs ../libneu-static.a $(ALL_MODULES)
else
	$(LINK) -shared -Wl,-soname,libneu.so -o ../libneu.so.$(VERSION) $(ALL_MODULES) libedit-64.a -ldl -lutil -lboost_regex -lpthread -lcurses -lz -lrt -lgmp -lmpfr -lssl -lOpenCL $(LLVM_LINK) -L$(HOME)/bullet/lib -lBulletDynamics -lBulletSoftBody -lBulletCollision -lLinearMath -lglut
	(cd ..; ln -f -s libneu.so.$(VERSION) libneu.so)
	(cd ..; ln -f -s libneu.so.$(VERSION) libneu.so.$(MAJOR))
	(cd ..; ln -f -s libneu.so.$(VERSION) libneu.so.$(MAJOR).$(MINOR))
	rm -f ../libneu-static.a
	ar rcs ../libneu-static.a $(ALL_MODULES)
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
	(cd nm; $(MAKE) clean)
	(cd npl; $(MAKE) clean)

spotless: clean
	rm -f ../libneu.*
	(cd nml; $(MAKE) spotless)
	(cd nm; $(MAKE) spotless)
	(cd npl; $(MAKE) spotless)
