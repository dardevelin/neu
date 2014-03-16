PLATFORM = $(shell uname)

TARGET = $(NEU_HOME)/bin/neu-meta
MODULES = main.o

COMPILE = clang++ -g -std=c++11 -stdlib=libc++ -I$(NEU_HOME)/include -I/usr/local/include
LINK = clang++ -stdlib=libc++ -L$(NEU_HOME)/lib -lneu

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
