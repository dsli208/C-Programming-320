CC := gcc
SRCD := src
TSTD := tests
BLDD := build
BIND := bin
INCD := include
LIBD := lib

MAIN  := $(BLDD)/main.o
LIB := $(LIBD)/bavarde.a

ALL_SRCF := $(shell find $(SRCD) -type f -name *.c)
ALL_OBJF := $(patsubst $(SRCD)/%,$(BLDD)/%,$(ALL_SRCF:.c=.o))
ALL_FUNCF := $(filter-out $(MAIN), $(ALL_OBJF))
ALL_TESTF := $(wildcard $(TSTD)/*.c)

INC := -I $(INCD)

CFLAGS := -Wall -Werror
COLORF := -DCOLOR
DFLAGS := -g -DDEBUG
PRINT_STAMENTS := -DERROR -DSUCCESS -DWARN -DINFO

STD := -std=gnu11
TEST_LIB := -lcriterion
LIBS := -L $(LIBD) -l:bavarde.a -lpthread

CFLAGS += $(STD)

EXEC := bavarde
TEST_EXEC := $(EXEC)_tests

.PHONY: clean all

all: TEST_SRC = $(ALL_TESTF) 
all: setup $(EXEC) $(TEST_EXEC)

debug: CFLAGS += $(DFLAGS) $(PRINT_STAMENTS) $(COLORF)
debug: all

setup:
	mkdir -p $(BIND) $(BLDD) $(LIBD)

$(EXEC): $(MAIN) $(ALL_FUNCF)
	$(CC) $(CFLAGS) $^ -o $(BIND)/$(EXEC) $(LIBS)

$(TEST_EXEC): $(ALL_FUNCF)
	$(CC) $(CFLAGS) $(INC) $(TEST_SRC) $^ -o $(BIND)/$(TEST_EXEC) $(TEST_LIB) $(LIBS)

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm -rf $(BLDD) $(BIND)
