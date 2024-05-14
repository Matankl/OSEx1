# CC = gcc
# CFLAGS = -g3 -lstdc++


# all: MainStackOF MainMemoryFail MainDivBy0 

# MainStackOF: MainStackOF.o 
# 	$(CC) $(CFLAGS) -o $@ $^ 

# MainMemoryFail: MainMemoryFail.o
# 	$(CC) $(CFLAGS) -o $@ $^ 

# MainDivBy0: MainDivBy0.o
# 	$(CC) $(CFLAGS) -o $@ $^ 

# MainStackOF.o: MainStackOF.cpp
# 	$(CC) $(CFLAGS) -c -o $@ $<

# MainMemoryFail.o: MainMemoryFail.cpp
# 	$(CC) $(CFLAGS) -c -o $@ $<

# MainDivBy0.o: MainDivBy0.cpp
# 	$(CC) $(CFLAGS) -c -o $@ $<

# clean:
# 	rm -f *.o *.a MainStackOF MainMemoryFail MainDivBy0 


CC = gdb
CFLAGS = -g3
LDFLAGS = -lstdc++

all: MainStackOF MainMemoryFail MainDivBy0 

MainStackOF: MainStackOF.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

MainMemoryFail: MainMemoryFail.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

MainDivBy0: MainDivBy0.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

MainStackOF.o: MainStackOF.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

MainMemoryFail.o: MainMemoryFail.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

MainDivBy0.o: MainDivBy0.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.a MainStackOF MainMemoryFail MainDivBy0 
