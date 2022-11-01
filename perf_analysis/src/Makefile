OUT = usimm
BINDIR = ../bin
OBJDIR = ../obj
OBJS = $(OBJDIR)/main.o $(OBJDIR)/malloc_lib.o $(OBJDIR)/hash_lib.o $(OBJDIR)/os.o $(OBJDIR)/memory_controller.o $(OBJDIR)/scheduler.o $(OBJDIR)/cache.o
CC = gcc
DEBUG = -O3 -g
CFLAGS = -std=gnu99 -O3 -c -fcommon $(DEBUG) 
LFLAGS = $(DEBUG)


$(BINDIR)/$(OUT): $(OBJS)
	$(CC) -fcommon $(LFLAGS) $(OBJS) -lz -lm -o $(BINDIR)/$(OUT)
	chmod 777 $(BINDIR)/$(OUT)

$(OBJDIR)/malloc_lib.o: malloc_lib.c global_types.h malloc_lib.h
	$(CC) $(CFLAGS) malloc_lib.c -o $(OBJDIR)/malloc_lib.o
	chmod 777 $(OBJDIR)/malloc_lib.o

$(OBJDIR)/hash_lib.o: hash_lib.c malloc_lib.h params.h global_types.h os.h hash_lib.h
	$(CC) $(CFLAGS) hash_lib.c -o $(OBJDIR)/hash_lib.o
	chmod 777 $(OBJDIR)/hash_lib.o

$(OBJDIR)/os.o: os.c params.h global_types.h os.h hash_lib.h
	$(CC) $(CFLAGS) os.c -o $(OBJDIR)/os.o
	chmod 777 $(OBJDIR)/os.o

$(OBJDIR)/main.o: main.c processor.h configfile.h memory_controller.h scheduler.h params.h cache.h 
	$(CC) $(CFLAGS) main.c -o $(OBJDIR)/main.o
	chmod 777 $(OBJDIR)/main.o

$(OBJDIR)/cache.o: cache.c cache.h
	$(CC) $(CFLAGS) cache.c -o $(OBJDIR)/cache.o
	chmod 777 $(OBJDIR)/cache.o

$(OBJDIR)/memory_controller.o: memory_controller.c utlist.h utils.h params.h memory_controller.h processor.h
	$(CC) $(CFLAGS) memory_controller.c -o $(OBJDIR)/memory_controller.o
	chmod 777 $(OBJDIR)/memory_controller.o

$(OBJDIR)/scheduler.o: scheduler.c scheduler.h utlist.h utils.h memory_controller.h params.h
	$(CC) $(CFLAGS) scheduler.c -o $(OBJDIR)/scheduler.o
	chmod 777 $(OBJDIR)/scheduler.o


clean:
	rm -f $(BINDIR)/$(OUT) $(OBJS)

