all: base_design

base_design : create_folder Maya_6BaseWays

#-------------------------
# DEFINES
#-------------------------
CC=g++
CFLAGS=-std=c++0x -O3 

create_folder:
	mkdir -p  bin

#-------------------------
# BINARIES
#-------------------------
Maya_6BaseWays : src/security_maya.cpp src/mtrand.h
	$(CC) $(CFLAGS) src/security_maya.cpp -o bin/maya6Ways.o 

clean:
	rm -rf bin/*.o ; 
