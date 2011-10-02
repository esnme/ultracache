PROGRAM=ultracached
CPP=g++
SOURCE=./ultracache
LIBS=-lpthread -lm -lstdc++ 
OBJS=benchmark.o Client.o JThread.o PacketReader.o Response.o tests.o ByteStream.o Hash.o main.o PacketWriter.o Server.o Cache.o Heap.o Packet.o Request.o Spinlock.o
CPPFLAGS=-I./ultracache/

all : ultracached

debug : CPPFLAGS += -O0 -ggdb -D_DEBUG
debug : ultracached

release : CPPFLAGS += -O3
release : ultracached

ultracached : $(OBJS)
	$(CPP) $(LINKARGS) $(OBJS) $(LIBS) -o ./$(PROGRAM)

%.o:	$(SOURCE)/%.cpp
	$(CPP) $(CPPFLAGS) -c $< -o ./$@

clean:
	rm -rf $(OBJS)
	rm -rf $(PROGRAM)

