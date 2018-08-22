CC = clang
CLFAGS = -Wall
CLIBS = -lstdc++ -lcurl
COBJ = 

getter: getter.cpp getter.hpp
	$(CC) $(CFLAGS) $(CLIBS) -o $@ $< $(COBJ)

%.o: %.cpp %.hpp
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: run clean

run: getter
	./getter messages.txt mtib lfalch

clean:
	rm -rf *.o
	rm -rf getter

