APP_NAME = main
LIB_NAME = libmySimpleComputer
LIB_NAME2 = libmyTerm
PROJECT = simplecomputer

CFLAGS = -Wall -Werror -I include -MMD
LFLAGS = -lm -lmySimpleComputer -lmyTerm -L obj/src/lib$(PROJECT)

APP_SRC = $(wildcard src/*.c)
APP_OBJ = $(APP_SRC:src/%.c=obj/src/%.o)
APP_PATH = bin/$(PROJECT)/$(APP_NAME)

LIB_SRC = src/lib$(PROJECT)/lib.c
LIB_SRC2 = src/lib$(PROJECT)/myTerm.c
LIB_OBJ = $(LIB_SRC:src/%.c=obj/src/%.o)
LIB_OBJ2 = $(LIB_SRC2:src/%.c=obj/src/%.o)
LIB_PATH = obj/src/lib$(PROJECT)/$(LIB_NAME).a
LIB_PATH2 = obj/src/lib$(PROJECT)/$(LIB_NAME2).a

LIBS = $(LIB_PATH) $(LIB_PATH2)
OBJ = $(APP_OBJ) $(LIB_OBJ) $(LIB_OBJ2)
DEPS = $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)

DIRS = bin bin/$(PROJECT) obj obj/src obj/src/lib$(PROJECT)

.PHONY: all
all: $(DIRS) $(OBJ) $(APP_PATH)

-include $(DEPS)

$(APP_PATH): $(APP_OBJ) $(LIBS)
	gcc $< -o $@ $(LFLAGS)

$(LIB_PATH): $(LIB_OBJ)
	ar rcs $@ $^
$(LIB_PATH2): $(LIB_OBJ2)
	ar rcs $@ $^

$(OBJ):
	gcc $(CFLAGS) -o $@ -c $(@:obj/src/%.o=src/%.c)

run: all
	./$(APP_PATH)

.PHONY: clean
clean:
	rm -fr $(DIRS)

.PHONY: format
format:
	git ls-files *.c | xargs clang-format -i --verbose && git diff --exit-code

.NOPARALLEL: $(DIRS)
.PHONY: $(DIRS)
$(DIRS):
	@if [ ! -d $@ ] ; then echo "creating $@"; mkdir $@; fi
	@if [ ! -d $@ ] ; then echo "$@ not created, error!"; exit 1; fi

