###############################################################################
# Definition of the project directories.
###############################################################################

INC_DIR = inc
SRC_DIR = src
OBJ_DIR = build
TST_DIR = tests

###############################################################################
# Definition of the compiler and its flags.
###############################################################################

CC       = gcc
#DEBUG    = -DDEBUG -g
WARNINGS = -Wall -Wextra -Wpedantic -Werror
CFLAGS   = $(WARNINGS) -I$(INC_DIR)  $(shell ncursesw5-config --cflags) $(DEBUG) -std=c11
LIBS     = $(shell ncursesw5-config --libs) -lformw -lmenuw -lm

###############################################################################
# LIBS
###############################################################################

SRC_LIBS += $(SRC_DIR)/ncv_common.c
SRC_LIBS += $(SRC_DIR)/ncv_ncurses.c
SRC_LIBS += $(SRC_DIR)/ncv_parser.c
SRC_LIBS += $(SRC_DIR)/ncv_table.c
SRC_LIBS += $(SRC_DIR)/ncv_table_part.c
SRC_LIBS += $(SRC_DIR)/ncv_corners.c
SRC_LIBS += $(SRC_DIR)/ncv_field.c
SRC_LIBS += $(SRC_DIR)/ncv_filter.c
SRC_LIBS += $(SRC_DIR)/ncv_ui_loop.c
SRC_LIBS += $(SRC_DIR)/ncv_forms.c
SRC_LIBS += $(SRC_DIR)/ncv_popup.c
SRC_LIBS += $(SRC_DIR)/ncv_win_header.c
SRC_LIBS += $(SRC_DIR)/ncv_win_filter.c
SRC_LIBS += $(SRC_DIR)/ncv_win_table.c
SRC_LIBS += $(SRC_DIR)/ncv_win_footer.c
SRC_LIBS += $(SRC_DIR)/ncv_win_help.c
SRC_LIBS += $(SRC_DIR)/ut_utils.c

OBJ_LIBS = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_LIBS)))

INC_LIBS = $(subst $(SRC_DIR),$(INC_DIR),$(subst .c,.h,$(SRC_LIBS)))

###############################################################################
# MAIN
###############################################################################

EXEC     = ccsvv

SRC_MAIN = $(SRC_DIR)/ncv_ccsvv.c

OBJ_MAIN = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_MAIN)))

###############################################################################
# TEST
###############################################################################

SRC_TEST += $(SRC_DIR)/ut_parser.c 
SRC_TEST += $(SRC_DIR)/ut_table.c
SRC_TEST += $(SRC_DIR)/ut_table_part.c 
SRC_TEST += $(SRC_DIR)/ut_field.c
SRC_TEST += $(SRC_DIR)/ut_common.c
SRC_TEST += $(SRC_DIR)/ut_filter.c

TEST     = $(subst $(SRC_DIR),$(TST_DIR),$(subst .c,,$(SRC_TEST)))

OBJ_TEST = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_TEST)))

###############################################################################
# DEMO
###############################################################################

SRC_DEMO += $(SRC_DIR)/demo_windows.c 

DEMO     = $(subst $(SRC_DIR),$(TST_DIR),$(subst .c,,$(SRC_DEMO)))

OBJ_DEMO = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_DEMO)))

###############################################################################
# Definitions of the build commands.
###############################################################################

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_LIBS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

$(EXEC): $(OBJ_LIBS) $(OBJ_MAIN)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

$(TEST): $(OBJ_LIBS) $(OBJ_TEST)
	gcc -o $@ $(OBJ_LIBS) $(subst $(TST_DIR),$(OBJ_DIR),$@.o) $(CFLAGS) $(LIBS)

unit_test: $(TEST)
	for ut_test in $(TEST) ; do \
		./$$ut_test || exit 1 ; \
    done

$(DEMO): $(OBJ_LIBS) $(OBJ_DEMO)
	gcc -o $@ $(OBJ_LIBS) $(subst $(TST_DIR),$(OBJ_DIR),$@.o) $(CFLAGS) $(LIBS) 

all: $(EXEC) unit_test $(DEMO)
	
###############################################################################
# Definition of the cleanup and run task.
###############################################################################

.PHONY: run clean

run:
	./$(EXEC)

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(SRC_DIR)/*.c~
	rm -f $(INC_DIR)/*.h~
	rm -f $(TST_DIR)/ut_*
	rm -f $(EXEC)
