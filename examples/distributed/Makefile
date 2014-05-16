include $(NEU_HOME)/Makefile.defs

TARGET = test
MODULES = main.o
META_MODULES = Object.o
ALL_MODULES = $(MODULES) $(META_MODULES)

LIBS = -L$(NEU_LIB) -lneu_core -lneu

all: .depend $(TARGET)

.depend: $(OBJECTS:.o=.cpp) $(OBJECTS:.o=.h)
	$(COMPILE) -MM $(MODULES:.o=.cpp) $(META_MODULES:.o=.cpp) -DMETA_GUARD > .depend

-include .depend

$(MODULES): $(@.o=.cpp)
	$(COMPILE) -c $< -o $@

%.o: %.cpp %.h
	$(COMPILE) -c $< -o $@

Object.o: $(@.o=.cpp)
	neu-meta -class Object Object.cpp
	$(COMPILE) -c $< -o $@

$(TARGET): $(ALL_MODULES)
	$(LINK) -o $(TARGET) $(ALL_MODULES) $(LIBS)

clean:
	rm -f Object_meta.h
	rm -f $(ALL_MODULES)
	rm -f .depend

spotless: clean
	rm -f $(TARGET)

