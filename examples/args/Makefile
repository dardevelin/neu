include $(NEU_HOME)/Makefile.defs

TARGET = args
OBJECTS = main.o

LIBS = -L$(NEU_LIB) -lneu_core

all: .depend $(TARGET)

.depend: $(OBJECTS:.o=.cpp)
	$(COMPILE) -MM $(OBJECTS:.o=.cpp) -DMETA_GUARD > .depend

-include .depend

main.o: main.cpp
	$(COMPILE) -c main.cpp -o main.o

$(TARGET): $(OBJECTS)
	$(LINK) -o $(TARGET) $(OBJECTS) $(LIBS)

clean:
	rm -f *_meta.h
	rm -f $(OBJECTS)
	rm -f .depend

spotless: clean
	rm -f $(TARGET)

