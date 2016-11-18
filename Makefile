MAKE FILE

CC = g++
CFLAGS  = -w -g -Wall
LFLAGS = -lsocket -lnsl -lresolv -lstdc++

all: 
	 $(CC) $(CFLAGS) -o aws_1 aws.cpp $(LFLAGS)
	 $(CC) $(CFLAGS) -o client client.cpp $(LFLAGS)
	 $(CC) $(CFLAGS) -o server_A serverA.cpp $(LFLAGS)
	 $(CC) $(CFLAGS) -o server_B serverB.cpp $(LFLAGS)
	 $(CC) $(CFLAGS) -o server_C serverC.cpp $(LFLAGS)

serverA:
	./server_A
serverB:
	./server_B
serverC:
	./server_C
aws:
	./aws_1



