TARGET = cubes
OBJECTS = main.o

COMPILE = clang++ -O2 -I/usr/local/include -I$(NEU_HOME)/include -std=c++11 -stdlib=libc++
LINK = clang++ -L$(NEU_HOME)/lib -lneu_core -lneu -stdlib=libc++ -framework Foundation -framework Cocoa -framework OpenGL -framework GLUT

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

