############################################################################
# Definition of the project directories.
############################################################################

INC_DIR = inc
SRC_DIR = src
OBJ_DIR = obj

############################################################################
# Definition of the compiler and its flags.
############################################################################

CC     = gcc
DEBUG  = -DDEBUG -g
CFLAGS = -I$(INC_DIR) -Wall -Werror -g $(shell ncursesw5-config --cflags) $(DEBUG)
LIBS   = $(shell ncursesw5-config --libs)

############################################################################
# Definition of the project files.
############################################################################

EXEC     = ccsvv

INCLUDES = $(INC_DIR)/ncv_table.h \
           $(INC_DIR)/ncv_parser.h \
           $(INC_DIR)/ncv_common.h 
           
OBJECTS  = $(OBJ_DIR)/ncv_ccsvv.o \
           $(OBJ_DIR)/ncv_table.o \
           $(OBJ_DIR)/ncv_parser.o \
           $(OBJ_DIR)/ncv_common.o
           
############################################################################
# Definitions of the build commands.
############################################################################

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDES)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

$(EXEC): $(OBJECTS)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

all: $(EXEC) $(OBJECTS)

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
