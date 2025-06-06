CC		=clang
#CC	=gcc
CC-flags		=-Wall -g #-DDEBUG

PROGRAMS	=slurpe-probe 
			

.SUFFIXES:	.c .o

.c.o:;	$(CC) $(CC-flags) -c $<

C-files		=slurpe-probe.c \
	UdpSocket.c 
	
	


all:	$(PROGRAMS)

UdpSocket.c:	UdpSocket.h

byteorder64.c:	byteorder64.h

slurpe-probe:	slurpe-probe.o UdpSocket.o byteorder64.o
	$(CC) -o $@ $+



.PHONY:	clean

clean:;	rm -rf *.o $(PROGRAMS) *~
