GXX=g++

simplefs: shell.o fs.o disk.o
	$(GXX) shell.o fs.o disk.o -o simplefs

shell.o: src/shell.cc
	$(GXX) -Wall src/shell.cc -c -o shell.o -g

fs.o: src/fs.cc src/fs.h
	$(GXX) -Wall src/fs.cc -c -o fs.o -g

disk.o: src/disk.cc src/disk.h
	$(GXX) -Wall src/disk.cc -c -o disk.o -g

clean:
	rm simplefs disk.o fs.o shell.o