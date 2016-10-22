# Some variables
CC 	        := gcc
LD          := gcc
CFLAGS		:= -g -Wall -DDEBUG
LDFLAGS		:= -lm
TESTDEFS	:= -DTESTING    # comment this out to disable debugging code

MODULES := hashmap \
	       network \
		   packet  \
		   utilities
CFLAGS += $(foreach MODULE,$(MODULES),-I$(MODULE))

BINS           := peer client server make_chunks
TESTBINS       := test_debug test_input_buffer  # not automatic
UTESTBINS      := test_hashmap test_packet

BUILD     := build
SRCS_DEP  := $(shell find $(MODULES) -name "*.c")
OBJS_DEP  := $(patsubst %.c,$(BUILD)/%.o,$(SRCS_DEP))

DEBUG   := debug
SRC_DEBUG := $(DEBUG)/debug.c
HDR_DEBUG := $(DEBUG)/debug.h
HDR_DEBUG_TEXT := $(BUILD)/$(DEBUG)/debug-text.h
OBJ_DEBUG := $(BUILD)/$(DEBUG)/debug.o
OBJ_TEST_DEBUG := $(BUILD)/$(DEBUG)/test_debug.o
CFLAGS += -I$(DEBUG) -I$(BUILD)/$(DEBUG)

TEST := utests

SCRIPTS := scripts

.PHONY: pre run test utest clean

all: pre $(BINS) $(TESTBINS) $(UTESTBINS)

pre:
	mkdir -p $(BUILD)
	$(foreach MODULE,$(MODULES),$(shell mkdir -p $(BUILD)/$(MODULE)))
	mkdir -p $(BUILD)/$(DEBUG)
	mkdir -p $(BUILD)/$(TEST)

define gen_OBJ
$$(BUILD)/$$(patsubst %c,%o,$(1)): $(1) $$(patsubst %c,%h,$(1))
	@echo "$$@ : $$^"
	$$(CC) $$< $$(CFLAGS) $$(TESTDEFS) -c -o $$@
endef

$(foreach SRC,$(SRCS_DEP),$(eval $(call gen_OBJ,$(SRC))))

# Begin debugging utility code

$(HDR_DEBUG_TEXT): $(HDR_DEBUG)
	$(SCRIPTS)/debugparse.pl < $(HDR_DEBUG) \
		> $(HDR_DEBUG_TEXT)

$(OBJ_DEBUG): $(SRC_DEBUG) $(HDR_DEBUG) $(HDR_DEBUG_TEXT)
	$(CC) $(CFLAGS) $< -c -o $@

$(OBJ_TEST_DEBUG): $(SRC_DEBUG) $(HDR_DEBUG) $(HDR_DEBUG_TEXT)
	$(CC) $(CFLAGS) $< -D_TEST_DEBUG_ -c -o $@

test_debug: $(OBJ_TEST_DEBUG)
	$(LD) $(LDFLAGS) $^ -o $@

test_input_buffer.o: test_input_buffer.c
	$(CC) $(CFLAGS) $< -c -o $@

test_input_buffer: test_input_buffer.o $(OBJ_DEBUG) $(OBJS_DEP)
	$(LD) $(LDFLAGS) $^ -o $@

# End debug

define gen_BIN
$$(BUILD)/$(1).o: $(1).c
	$$(CC) $$< $$(CFLAGS) -c -o $$@

$(1): $$(BUILD)/$(1).o $$(OBJ_DEBUG) $$(OBJS_DEP)
	$$(LD) $$^ $$(LDFLAGS) -o $$@
endef

$(foreach BIN,$(BINS),$(eval $(call gen_BIN,$(BIN))))

define gen_UTESTBIN
$$(BUILD)/$$(TEST)/$(1).o: $$(TEST)/$(1).c
	$$(CC) $$< $$(CFLAGS) -c -o $$@

$(1): $$(BUILD)/$$(TEST)/$(1).o $$(OBJ_DEBUG) $$(OBJS_DEP)
	$$(LD) $$^ $$(LDFLAGS) -o $$@
endef

$(foreach UTESTBIN,$(UTESTBINS),$(eval $(call gen_UTESTBIN,$(UTESTBIN))))

run: peer_run
	./peer_run

test: peer_test
	./peer_test

utest: pre $(UTESTBINS)
	$(foreach bin,$(UTESTBINS),./$(bin);)

clean:
	rm -rf *.o $(BUILD) $(BINS) $(TESTBINS) $(UTESTBINS)
