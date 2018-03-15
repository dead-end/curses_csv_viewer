############################################################################
# Definition of the project directories.
############################################################################

INC_DIR = include
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

INCLUDES = $(INC_DIR)/ccsvv.h \
           $(INC_DIR)/common.h \
           $(INC_DIR)/utils.h
           
OBJECTS  = $(OBJ_DIR)/ccsvv.o \
           $(OBJ_DIR)/utils.o

############################################################################
# Definitions of the build commands.
############################################################################

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDES)
	echo $(ncursesw5-config --libs)
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
