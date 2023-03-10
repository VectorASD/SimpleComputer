APP_NAME = main
LIB_NAME = libmySimpleComputer
LIB_NAME2 = libmyTerm
LIB_NAME3 = libmyBigChars
LIB_NAME4 = libmyReadKey
PROJECT = simplecomputer

CFLAGS = -Wall -Werror -I include -MMD
LFLAGS = -lmyBigChars -lmyTerm -lmySimpleComputer -lm -L obj/src/lib$(PROJECT)
# Тут библиотеки линкером в обратном порядке оказывается грузятся O_o Т.е. первее будет последняя ;'-}

APP_SRC = $(wildcard src/*.c)
APP_OBJ = $(APP_SRC:src/%.c=obj/src/%.o)
APP_PATH = bin/$(PROJECT)/$(APP_NAME)

LIB_SRC = src/lib$(PROJECT)/lib.c
LIB_SRC2 = src/lib$(PROJECT)/myTerm.c
LIB_SRC3 = src/lib$(PROJECT)/myBigChars.c
LIB_SRC4 = src/lib$(PROJECT)/myReadKey.c
LIB_OBJ = $(LIB_SRC:src/%.c=obj/src/%.o)
LIB_OBJ2 = $(LIB_SRC2:src/%.c=obj/src/%.o)
LIB_OBJ3 = $(LIB_SRC3:src/%.c=obj/src/%.o)
LIB_OBJ4 = $(LIB_SRC4:src/%.c=obj/src/%.o)
LIB_PATH = obj/src/lib$(PROJECT)/$(LIB_NAME).a
LIB_PATH2 = obj/src/lib$(PROJECT)/$(LIB_NAME2).a
LIB_PATH3 = obj/src/lib$(PROJECT)/$(LIB_NAME3).a
LIB_PATH4 = obj/src/lib$(PROJECT)/$(LIB_NAME4).a

LIBS = $(LIB_PATH) $(LIB_PATH2) $(LIB_PATH3) $(LIB_PATH4)
OBJ = $(APP_OBJ) $(LIB_OBJ) $(LIB_OBJ2) $(LIB_OBJ3) $(LIB_OBJ4)
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
$(LIB_PATH3): $(LIB_OBJ3)
	ar rcs $@ $^
$(LIB_PATH4): $(LIB_OBJ4)
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
	git ls-files *.c | xargs clang-format --style GNU -i --verbose && git diff --exit-code

.NOPARALLEL: $(DIRS)
.PHONY: $(DIRS)
$(DIRS):
	@if [ ! -d $@ ] ; then echo "creating $@"; mkdir $@; fi
	@if [ ! -d $@ ] ; then echo "$@ not created, error!"; exit 1; fi

