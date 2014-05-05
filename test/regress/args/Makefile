include $(NEU_HOME)/Makefile.defs

TARGET = test
OBJECTS = main.o

LINK += -L$(NEU_LIB) -lneu_core

all: .depend $(TARGET)
	./test &> test.out

.depend: $(OBJECTS:.o=.cpp) $(OBJECTS:.o=.h)
	$(COMPILE) -MM $(OBJECTS:.o=.cpp) > .depend

-include .depend

%.o: %.cpp %.h
	$(COMPILE) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(LINK) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(OBJECTS)
	rm -f .depend

spotless: clean
	rm -f $(TARGET)

