APP_NAME = main
APP_NAME2 = main2
APP_NAME3 = main3
APP_NAME4 = main4
APP_NAME5 = main5
LIB_NAME = libmySimpleComputer
LIB_NAME2 = libmyTerm
LIB_NAME3 = libmyBigChars
LIB_NAME4 = libmyReadKey
PROJECT = simplecomputer
USE_APP = $(APP_PATH5)

CFLAGS = -Wall -Werror -I include -MMD
# Тут библиотеки линкером в обратном порядке оказывается грузятся O_o Т.е. первее будет последняя ;'-}

APP_PATH = bin/$(PROJECT)/$(APP_NAME)
LFLAGS = -lmySimpleComputer -L obj/src/lib$(PROJECT)
APP_SRC = src/$(APP_NAME).c
APP_OBJ = $(APP_SRC:src/%.c=obj/src/%.o)

APP_PATH2 = bin/$(PROJECT)/$(APP_NAME2)
LFLAGS2 = -lmyTerm -L obj/src/lib$(PROJECT)
APP_SRC2 = src/$(APP_NAME2).c
APP_OBJ2 = $(APP_SRC2:src/%.c=obj/src/%.o)

APP_PATH3 = bin/$(PROJECT)/$(APP_NAME3)
LFLAGS3 = -lmyBigChars -lmyTerm -lmySimpleComputer -L obj/src/lib$(PROJECT)
APP_SRC3 = src/$(APP_NAME3).c
APP_OBJ3 = $(APP_SRC3:src/%.c=obj/src/%.o)

APP_PATH4 = bin/$(PROJECT)/$(APP_NAME4)
LFLAGS4 = -lmyReadKey -lmyBigChars -lmyTerm -lmySimpleComputer -L obj/src/lib$(PROJECT)
APP_SRC4 = src/$(APP_NAME4).c
APP_OBJ4 = $(APP_SRC4:src/%.c=obj/src/%.o)

APP_PATH5 = bin/$(PROJECT)/$(APP_NAME5)
LFLAGS5 = -lmyReadKey -lmyBigChars -lmyTerm -lmySimpleComputer -L obj/src/lib$(PROJECT)
APP_SRC5 = src/$(APP_NAME5).c
APP_OBJ5 = $(APP_SRC5:src/%.c=obj/src/%.o)

APP_OBJS = $(APP_OBJ) $(APP_OBJ2) $(APP_OBJ3) $(APP_OBJ4) $(APP_OBJ5)
APP_PATHS = $(APP_PATH) $(APP_PATH2) $(APP_PATH3) $(APP_PATH4) $(APP_PATH5)

LIB_SRC = src/lib$(PROJECT)/lib.c
LIB_SRC2 = src/lib$(PROJECT)/myTerm.c
LIB_SRC3 = src/lib$(PROJECT)/myBigChars.c
LIB_SRC4 = src/lib$(PROJECT)/myReadKey.c
LIB_OBJ = $(LIB_SRC:src/%.c=obj/src/%.o)
LIB_OBJ2 = $(LIB_SRC2:src/%.c=obj/src/%.o)
LIB_OBJ3 = $(LIB_SRC3:src/%.c=obj/src/%.o)
LIB_OBJ4 = $(LIB_SRC4:src/%.c=obj/src/%.o)
LIB_OBJS = $(LIB_OBJ) $(LIB_OBJ2) $(LIB_OBJ3) $(LIB_OBJ4)
LIB_PATH = obj/src/lib$(PROJECT)/$(LIB_NAME).a
LIB_PATH2 = obj/src/lib$(PROJECT)/$(LIB_NAME2).a
LIB_PATH3 = obj/src/lib$(PROJECT)/$(LIB_NAME3).a
LIB_PATH4 = obj/src/lib$(PROJECT)/$(LIB_NAME4).a

LIBS = $(LIB_PATH)
LIBS2 = $(LIB_PATH2)
LIBS3 = $(LIB_PATH) $(LIB_PATH2) $(LIB_PATH3)
LIBS4 = $(LIB_PATH) $(LIB_PATH2) $(LIB_PATH3) $(LIB_PATH4)

OBJ = $(APP_OBJS) $(LIB_OBJS)
DEPS = $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)

DIRS = bin bin/$(PROJECT) obj obj/src obj/src/lib$(PROJECT)

.PHONY: all
all: $(DIRS) $(OBJ) $(APP_PATHS)
run: all
	./$(USE_APP)

-include $(DEPS)

$(APP_PATH): $(APP_OBJ) $(LIBS)
	gcc $< -o $@ $(LFLAGS)
$(APP_PATH2): $(APP_OBJ2) $(LIBS2)
	gcc $< -o $@ $(LFLAGS2)
$(APP_PATH3): $(APP_OBJ3) $(LIBS3)
	gcc $< -o $@ $(LFLAGS3)
$(APP_PATH4): $(APP_OBJ4) $(LIBS4)
	gcc $< -o $@ $(LFLAGS4)
$(APP_PATH5): $(APP_OBJ5) $(LIBS4)
	gcc $< -o $@ $(LFLAGS5)

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
