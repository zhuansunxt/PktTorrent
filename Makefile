# Some variables
CC 	        = gcc
CFLAGS		= -g -Wall -DDEBUG
LDFLAGS		= -lm
TESTDEFS	= -DTESTING			# comment this out to disable debugging code

OBJS	        = peer.o bt_parse.o spiffy.o debug.o \
                  input_buffer.o chunk.o sha.o
MK_CHUNK_OBJS   = make_chunks.o chunk.o sha.o

BINS            = peer make-chunks
TESTBINS        = test_debug test_input_buffer
UTESTBINS       = test_hashmap test_packet

# Implicit .o target
.c.o:
	$(CC) $(TESTDEFS) -c $(CFLAGS) $<

# Explit build and testing targets

all: ${BINS} ${TESTBINS}

.PHONY: run test utest peer clean

run: peer_run
	./peer_run

test: peer_test
	./peer_test

utest: clean $(UTESTBINS)
	$(foreach bin,$(UTESTBINS),./$(bin);)

peer: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

make-chunks: $(MK_CHUNK_OBJS)
	$(CC) $(CFLAGS) $(MK_CHUNK_OBJS) -o $@ $(LDFLAGS)

clean:
	rm -f *.o $(BINS) $(TESTBINS) $(UTESTBINS)

bt_parse.c: bt_parse.h

# The debugging utility code

debug-text.h: debug.h
	./debugparse.pl < debug.h > debug-text.h

test_debug.o: debug.c debug-text.h
	${CC} debug.c ${INCLUDES} ${CFLAGS} -c -D_TEST_DEBUG_ -o $@

test_input_buffer:  test_input_buffer.o input_buffer.o

test_hashmap: test_hashmap.o hashmap.o

