CC = clang
CLFAGS = -Wall
CLIBS = -lstdc++
COBJ = 

.PHONY: run clean

run: getter
	./getter messages.txt mtib lfalch

clean:
	rm -rf *.o

getter: getter.cpp getter.hpp
	$(CC) $(CFLAGS) $(CLIBS) -o $@ $< $(COBJ)

