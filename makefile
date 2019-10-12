################################################################################
# Definition of the project directories.
################################################################################

INC_DIR = inc
SRC_DIR = src
OBJ_DIR = build
TST_DIR = tests

################################################################################
# A variable that collects the optional flags.
################################################################################

OPTION_FLAGS=

################################################################################
# A flag to enable / disable ACS borders. If the flag is 'false', ASCII 
# characters are used. This may be helpful if, for example putty has an issue
# with ACS and UTF-8.
################################################################################

USE_ACS_BORDERS = true

ifeq ($(USE_ACS_BORDERS),false)
  OPTION_FLAGS += -DNO_ACS_BORDER
endif

################################################################################
# A debug flag for the application. If set to 'true' the application has to pipe
# stderr to a file.
################################################################################

DEBUG = false

ifeq ($(DEBUG),true)
  OPTION_FLAGS += -DDEBUG -g
endif

################################################################################
# Ncurses has a major version and that version determines some programs and
# library names, especially the ncurses config program, which contains 
# informations for the compiler.
################################################################################

NCURSES_MAJOR  = 5

NCURSES_CONFIG = ncursesw$(NCURSES_MAJOR)-config

################################################################################
# Definition of compiler flags. CC is defined by make and CFLAGS can be set by
# the user. The flag _GNU_SOURCE is necessary for qsort_r.
################################################################################

WARN_FLAGS  = -Wall -Wextra -Wpedantic -Werror

BUILD_FLAGS = -std=c11 -O2 -D_GNU_SOURCE

FLAGS      = $(BUILD_FLAGS) $(OPTION_FLAGS) $(WARN_FLAGS) -I$(INC_DIR) $(shell $(NCURSES_CONFIG) --cflags)

LIBS        = $(shell $(NCURSES_CONFIG) --libs) -lformw -lmenuw -lm

################################################################################
# The list of sources that are used to build the executable. The last one:
# ut_utils.c is only used to build the test programs. Each of the source files
# has a header file with the same name.
################################################################################

SRC_LIBS = \
	$(SRC_DIR)/ncv_common.c \
	$(SRC_DIR)/ncv_ncurses.c \
	$(SRC_DIR)/ncv_parser.c \
	$(SRC_DIR)/ncv_table.c \
	$(SRC_DIR)/ncv_table_part.c \
	$(SRC_DIR)/ncv_table_header.c \
	$(SRC_DIR)/ncv_table_sort.c \
	$(SRC_DIR)/ncv_corners.c \
	$(SRC_DIR)/ncv_field.c \
	$(SRC_DIR)/ncv_filter.c \
	$(SRC_DIR)/ncv_sort.c \
	$(SRC_DIR)/ncv_ui_loop.c \
	$(SRC_DIR)/ncv_forms.c \
	$(SRC_DIR)/ncv_popup.c \
	$(SRC_DIR)/ncv_wbuf.c \
	$(SRC_DIR)/ncv_win_header.c \
	$(SRC_DIR)/ncv_win_filter.c \
	$(SRC_DIR)/ncv_win_table.c \
	$(SRC_DIR)/ncv_win_footer.c \
	$(SRC_DIR)/ncv_win_help.c \
	$(SRC_DIR)/ut_utils.c \

OBJ_LIBS = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_LIBS)))

INC_LIBS = $(subst $(SRC_DIR),$(INC_DIR),$(subst .c,.h,$(SRC_LIBS)))

################################################################################
# The main program.
################################################################################

EXEC     = ccsvv

SRC_EXEC = $(SRC_DIR)/ncv_ccsvv.c

OBJ_EXEC = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_EXEC)))

################################################################################
# The definitions for the test programs.
################################################################################

SRC_TEST = \
	$(SRC_DIR)/ut_parser.c \
	$(SRC_DIR)/ut_table.c \
	$(SRC_DIR)/ut_table_part.c \
	$(SRC_DIR)/ut_table_header.c \
	$(SRC_DIR)/ut_table_sort.c \
	$(SRC_DIR)/ut_sort.c \
	$(SRC_DIR)/ut_field.c \
	$(SRC_DIR)/ut_common.c \
	$(SRC_DIR)/ut_filter.c \
	$(SRC_DIR)/ut_wbuf.c \

TESTS    = $(subst $(SRC_DIR),$(TST_DIR),$(subst .c,,$(SRC_TEST)))

OBJ_TEST = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_TEST)))

################################################################################
# Definition of the top-level targets. 
#
# A phony target is one that is not the name of a file. If a file 'all' exists,
# nothing will happen.
################################################################################

.PHONY: all

all: $(EXEC) test $(DEMO)

################################################################################
# A static pattern, that builds an object file from its source. The automatic
# variable $@ is the target and $< is the first prerequisite, which is the
# corrosponding source file. Example:
#
#   build/ncv_common.o: src/ncv_common.c src/ncv_common.h src/ncv_ncurses.h...
#	  gcc -c -o build/ncv_common.o src/ncv_common.c ...
#
# The complete list of header files is not necessary as a prerequisite, but it
# does not hurt.
################################################################################

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_LIBS)
	$(CC) -c -o $@ $< $(FLAGS) $(LIBS)

################################################################################
# The goal compiles the executable from the object files. The automatic $^ is 
# the list of all prerequisites, which are the object files in this case.
################################################################################

$(EXEC): $(OBJ_LIBS) $(OBJ_EXEC)
	$(CC) -o $@ $^ $(FLAGS) $(LIBS)

################################################################################
# The goal compiles the executables of the test programs. The valiable TEST is
# the list of all test programs. The expression: 
#
#   $(subst $(TST_DIR),$(OBJ_DIR),$@.o)
#
# creates the corresponding object file from the test program. Example:
#
#   tests/ut_parser => build/ut_parser.o
################################################################################

$(TESTS): $(OBJ_LIBS) $(OBJ_TEST)
	$(CC) -o $@ $(OBJ_LIBS) $(subst $(TST_DIR),$(OBJ_DIR),$@.o) $(FLAGS) $(LIBS)

################################################################################
# The test goal invokes all test programs.
################################################################################

.PHONY: test

test: $(TESTS)
	@for ut_test in $(TESTS) ; do ./$$ut_test || exit 1 ; done
	@echo "Tests: OK"

################################################################################
# Goals to install and uninstall the executable.
# --owner=root --group=root 
################################################################################

PREFIX = /usr/local

BINDIR = $(PREFIX)/bin

DOCDIR = $(PREFIX)/share/doc/$(EXEC)

MANDIR = $(PREFIX)/share/man/man1

MANPAGE = ccsvv.1

.PHONY: install uninstall

DEB_INSTALL=false

install: $(EXEC)
	gzip -9n -c man/$(MANPAGE) > $(OBJ_DIR)/$(MANPAGE).gz
	install -D --mode=644 $(OBJ_DIR)/$(MANPAGE).gz --target-directory=$(MANDIR)
	install -D --mode=755 $(EXEC) --target-directory=$(BINDIR) --strip
	if [ "$(DEB_INSTALL)" = "false" ]; then \
		install -D --mode=644 LICENSE $(DOCDIR)/copyright ; \
		install -D --mode=644 changelog $(DOCDIR)/changelog ; \
	fi

uninstall:
	rm -f $(DOCDIR)/copyright
	if [ -d "$(DOCDIR)" ]; then rmdir $(DOCDIR); fi
	rm -f $(BINDIR)/$(EXEC)
	rm -f $(MANDIR)/$(MANPAGE).gz

################################################################################
# The cleanup goal deletes the executable, the test programs, all object files
# and some editing remains.
################################################################################

.PHONY: clean

clean:
	rm -rf $(OBJ_DIR)/root/
	rm -f $(OBJ_DIR)/*.gz
	rm -f $(OBJ_DIR)/*.o
	rm -f $(SRC_DIR)/*.c~
	rm -f $(INC_DIR)/*.h~
	rm -f $(TST_DIR)/ut_*
	rm -f $(EXEC)

################################################################################
# The goal prints a help message the the ccsvv specific options for the build.
################################################################################

.PHONY: help

help:
	@echo "Targets:"
	@echo ""
	@echo "  make | make all              : Triggers the build of the executable."
	@echo "  make test                    : Triggers unit tests."
	@echo "  make clean                   : Removes executables and temporary files from the build."
	@echo "  make install | uninstall     : Installs / uninstalles the program files."
	@echo "  make help                    : Prints this message."
	@echo ""
	@echo "Parameter:"
	@echo ""
	@echo "  DEBUG=[true|false]           : A debug flag for the application. (default: false)"
	@echo "  NCURSES_MAJOR=[5|6]          : The major verion of ncurses. (default: 5)"
	@echo "  USE_ACS_BORDERS=[true|false] : Use ASCII characters as borders. (default: false)"
	@echo "  PREFIX=<PATH>                : Prefix for install / uninstall. (default: /usr/local)"

################################################################################
# Todo: remove
################################################################################

SRC_DEMO += $(SRC_DIR)/demo_windows.c 

DEMO     = $(subst $(SRC_DIR),$(TST_DIR),$(subst .c,,$(SRC_DEMO)))

OBJ_DEMO = $(subst $(SRC_DIR),$(OBJ_DIR),$(subst .c,.o,$(SRC_DEMO)))

$(DEMO): $(OBJ_LIBS) $(OBJ_DEMO)
	$(CC) -o $@ $(OBJ_LIBS) $(subst $(TST_DIR),$(OBJ_DIR),$@.o) $(FLAGS) $(LIBS) 
