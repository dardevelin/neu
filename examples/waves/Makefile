include $(NEU_HOME)/Makefile.defs

TARGET = waves
OBJECTS = main.o

LINK += -L$(NEU_LIB) -lneu_core -lneu -framework Foundation -framework Cocoa -framework OpenGL -framework GLUT

all: .depend $(TARGET)

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

