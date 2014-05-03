TARGET = $(NEU_HOME)/bin/neu-meta
MODULES = main.o

ifeq ($(PLATFORM), Darwin)
  STD_LIB = -stdlib=libc++
endif

COMPILE = clang++ -g -std=c++11 $(STD_LIB) -I$(NEU_HOME)/include -I/usr/local/include -I$(LLVM_DIR)/include -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS

LINK = clang++ $(STD_LIB) -L$(NEU_HOME)/lib -lneu_core -L$(LLVM_DIR)/lib -lLLVMAsmParser -lLLVMBitReader -lLLVMSupport -lLLVMMC -lLLVMOption -lLLVMAnalysis -lLLVMAsmPrinter -lLLVMBitWriter -lLLVMCodeGen -lLLVMCore -lLLVMExecutionEngine -lLLVMIRReader -lLLVMInstCombine -lLLVMInstrumentation -lLLVMInterpreter -lLLVMJIT -lLLVMLTO -lLLVMLinker -lLLVMMC -lLLVMMCDisassembler -lLLVMMCJIT -lLLVMMCParser -lLLVMObject -lLLVMScalarOpts -lLLVMSelectionDAG -lLLVMTableGen -lLLVMTarget -lLLVMTransformUtils -lLLVMVectorize -lclangFrontend -lclangSerialization -lclangDriver -lclangTooling -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangLex -lclangBasic -lcurses

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
