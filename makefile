############################################################################
# Definition of the project directories.
############################################################################

INC_DIR = inc
SRC_DIR = src
OBJ_DIR = obj

############################################################################
# Definition of the compiler and its flags.
############################################################################

CC       = gcc
DEBUG    = -DDEBUG -g
WARNINGS = -Wall -Wextra -Wpedantic -Werror
CFLAGS   = $(WARNINGS) -I$(INC_DIR)  $(shell ncursesw5-config --cflags) $(DEBUG) -std=c11
LIBS     = $(shell ncursesw5-config --libs)

############################################################################
# LIBS
############################################################################

SRC_LIBS += $(SRC_DIR)/ncv_table.c
SRC_LIBS += $(SRC_DIR)/ncv_parser.c
SRC_LIBS += $(SRC_DIR)/ncv_curses.c
SRC_LIBS += $(SRC_DIR)/ncv_common.c 

OBJ_LIBS = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_LIBS)))

INC_LIBS = $(subst $(SRC_DIR),$(INC_DIR),$(subst .c,.h,$(SRC_LIBS)))

############################################################################
# MAIN
############################################################################

EXEC     = ccsvv

SRC_MAIN = $(SRC_DIR)/ncv_ccsvv.c

OBJ_MAIN = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_MAIN)))

############################################################################
# TEST
############################################################################

TEST     = ncv_test

SRC_TEST = $(SRC_DIR)/ncv_test.c

OBJ_TEST = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_TEST)))

############################################################################
# Definitions of the build commands.
############################################################################

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_LIBS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

$(EXEC): $(OBJ_LIBS) $(OBJ_MAIN)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

$(TEST): $(OBJ_LIBS) $(OBJ_TEST)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

all: $(EXEC) $(TEST)
	@echo SRC: $(SRC_LIBS)
	@echo OBJ: $(OBJ_LIBS)
	@echo INC: $(INC_LIBS)
	
############################################################################
# Definition of the cleanup and run task.
############################################################################

.PHONY: run clean

run:
	./$(EXEC)

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(SRC_DIR)/*.c~
	rm -f $(INC_DIR)/*.h~
	rm -f $(EXEC)
	rm -f $(TEST)
