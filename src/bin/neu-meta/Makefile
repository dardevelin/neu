TARGET = $(NEU_HOME)/bin/neu-meta
MODULES = main.o

COMPILE += -I$(LLVM_DIR)/include -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS

LINK += -L$(NEU_LIB) -lneu_core -L$(LLVM_DIR)/lib -lLLVMAsmParser -lLLVMBitReader -lLLVMSupport -lLLVMMC -lLLVMOption -lLLVMAnalysis -lLLVMAsmPrinter -lLLVMBitWriter -lLLVMCodeGen -lLLVMCore -lLLVMExecutionEngine -lLLVMIRReader -lLLVMInstCombine -lLLVMInstrumentation -lLLVMInterpreter -lLLVMJIT -lLLVMLTO -lLLVMLinker -lLLVMMC -lLLVMMCDisassembler -lLLVMMCJIT -lLLVMMCParser -lLLVMObject -lLLVMScalarOpts -lLLVMSelectionDAG -lLLVMTableGen -lLLVMTarget -lLLVMTransformUtils -lLLVMVectorize -lclangFrontend -lclangSerialization -lclangDriver -lclangTooling -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangLex -lclangBasic -lcurses

all: .depend $(TARGET)

.depend: $(MODULES:.o=.cpp)
	$(COMPILE) -MM $(MODULES:.o=.cpp) > .depend

-include .depend

$(MODULES): $(@.o=.cpp)
	$(COMPILE) -c $< -o $@

$(TARGET): $(MODULES)
	$(LINK) -o $(TARGET) $(MODULES)

clean:
	rm -f $(MODULES)
	rm -f .depend

spotless: clean
	rm -f $(TARGET)
