# metaposix — plain-make fallback (use CMake for the full flow).
#   make check
CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -O2
CPPFLAGS = -Iinclude
LDLIBS   = -lpthread
C23FLAGS ?= -std=c2x
BUILD_DIR ?= build/make
OBJ_DIR = $(BUILD_DIR)/obj
LIBRARY = $(BUILD_DIR)/libmetaposix.a
TEST_RUNNER = $(BUILD_DIR)/mp_tests

C23_BITINT128 := $(shell printf '' | $(CC) $(C23FLAGS) -x c -dM -E - 2>/dev/null | awk '/__BITINT_MAXWIDTH__/ && $$3 + 0 >= 128 { print "yes" }' | grep -q yes && echo yes)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
CPPFLAGS += -D_GNU_SOURCE
LDLIBS   += -lrt
endif

LIB_SRCS = src/mp_sem.c src/mp_barrier.c src/mp_ppoll.c src/mp_string.c src/mp_uchar.c
TEST_SRCS = tests/main.c tests/int_string.c tests/thread_sync.c tests/file_fs.c tests/net.c tests/aio.c
LIB_OBJS = $(addprefix $(OBJ_DIR)/,$(LIB_SRCS:.c=.o))
TEST_OBJS = $(addprefix $(OBJ_DIR)/,$(TEST_SRCS:.c=.o))

all: $(TEST_RUNNER)

$(LIBRARY): $(LIB_OBJS)
	mkdir -p $(@D)
	$(AR) rcs $@ $(LIB_OBJS)

$(TEST_RUNNER): $(LIBRARY) $(TEST_OBJS)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(LIBRARY) $(LDLIBS)

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

ifeq ($(C23_BITINT128),yes)
$(OBJ_DIR)/tests/int_string.o: tests/int_string.c
	$(CC) $(CFLAGS) $(C23FLAGS) $(CPPFLAGS) -c -o $@ $<
endif

check: $(TEST_RUNNER)
	$(TEST_RUNNER)

clean:
	rm -f $(TEST_RUNNER) $(LIBRARY) $(LIB_OBJS) $(TEST_OBJS)

.PHONY: all check clean
