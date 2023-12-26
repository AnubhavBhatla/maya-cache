all: base_design sens_assoc sens_reuse

base_design : create_folder BaseWaysPerSkew_6

sens_assoc : create_folder BaseWaysPerSkew_12 BaseWaysPerSkew_3

sens_reuse : create_folder ReuseWaysPerSkew_1 ReuseWaysPerSkew_5 ReuseWaysPerSkew_7

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
BaseWaysPerSkew_6 : src/security_maya.cpp src/mtrand.h
	$(CC) $(CFLAGS) src/security_maya.cpp -o bin/maya6Ways.o 

BaseWaysPerSkew_12 : src/security_maya.cpp src/mtrand.h
	$(CC) $(CFLAGS) -DCUSTOM_BASE_WAYS_PER_SKEW=12  -DCUSTOM_REUSE_WAYS_PER_SKEW=6  -DCUSTOM_MAX_FILL=32  src/security_maya.cpp -o bin/maya12Ways.o 

BaseWaysPerSkew_3 : src/security_maya.cpp src/mtrand.h
	$(CC) $(CFLAGS) -DCUSTOM_BASE_WAYS_PER_SKEW=3  -DCUSTOM_REUSE_WAYS_PER_SKEW=1  src/security_maya.cpp -o bin/maya3Ways.o 
	
ReuseWaysPerSkew_1 : src/security_maya.cpp src/mtrand.h
	$(CC) $(CFLAGS) -DCUSTOM_REUSE_WAYS_PER_SKEW=1  src/security_maya.cpp -o bin/maya6Ways_1Reuse.o 
	
ReuseWaysPerSkew_5 : src/security_maya.cpp src/mtrand.h
	$(CC) $(CFLAGS) -DCUSTOM_REUSE_WAYS_PER_SKEW=5  src/security_maya.cpp -o bin/maya6Ways_5Reuse.o 
	
ReuseWaysPerSkew_7 : src/security_maya.cpp src/mtrand.h
	$(CC) $(CFLAGS) -DCUSTOM_REUSE_WAYS_PER_SKEW=7  src/security_maya.cpp -o bin/maya6Ways_7Reuse.o 
	
clean:
	rm -rf bin/*.o ; 
