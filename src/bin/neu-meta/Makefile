PLATFORM = $(shell uname)

TARGET = $(NEU_HOME)/bin/neu-meta
MODULES = main.o

COMPILE = clang++ -g -std=c++11 -stdlib=libc++ -I$(NEU_HOME)/include -I/usr/local/include -I$(HOME)/llvm-3.4/build-release/include -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS

LINK = clang++ -stdlib=libc++ -L$(NEU_HOME)/lib -lneu_core -L$(HOME)/llvm-3.4/build-release/lib -lLLVMAsmParser -lLLVMBitReader -lLLVMSupport -lLLVMMC -lLLVMOption -lLLVMAnalysis -lLLVMAsmPrinter -lLLVMBitWriter -lLLVMCodeGen -lLLVMCore -lLLVMExecutionEngine -lLLVMIRReader -lLLVMInstCombine -lLLVMInstrumentation -lLLVMInterpreter -lLLVMJIT -lLLVMLTO -lLLVMLinker -lLLVMMC -lLLVMMCDisassembler -lLLVMMCJIT -lLLVMMCParser -lLLVMObject -lLLVMScalarOpts -lLLVMSelectionDAG -lLLVMTableGen -lLLVMTarget -lLLVMTransformUtils -lLLVMVectorize -lclangFrontend -lclangSerialization -lclangDriver -lclangTooling -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangLex -lclangBasic -lcurses

all: $(TARGET)

main.o: main.cpp
	$(COMPILE) -c main.cpp

$(TARGET): $(MODULES)
	$(LINK) -o $(TARGET) $(MODULES)

clean:
	rm -f $(MODULES)
	rm -f .depend

spotless: clean
	rm -f $(TARGET)
