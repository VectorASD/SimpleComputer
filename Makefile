APP_NAME = main
LIB_NAME = libmySimpleComputer
LIB_NAME2 = libmyTerm
LIB_NAME3 = libmyBigChars
PROJECT = simplecomputer

FreeTypeARCHIVE = freetype-2.12.1
FreeTypeLIB_PATH = $(FreeTypeARCHIVE)/objs/libfreetype.a

CFLAGS = -Wall -Werror -I include -MMD -I $(FreeTypeARCHIVE)/include
LFLAGS = -lmyBigChars -lmyTerm -lmySimpleComputer -lm -lfreetype -L obj/src/lib$(PROJECT) -L $(FreeTypeARCHIVE)/objs/
# Тут библиотеки линкером в обратном порядке оказывается грузятся O_o Т.е. первее будет последняя

APP_SRC = $(wildcard src/*.c)
APP_OBJ = $(APP_SRC:src/%.c=obj/src/%.o)
APP_PATH = bin/$(PROJECT)/$(APP_NAME)

LIB_SRC = src/lib$(PROJECT)/lib.c
LIB_SRC2 = src/lib$(PROJECT)/myTerm.c
LIB_SRC3 = src/lib$(PROJECT)/myBigChars.c
LIB_OBJ = $(LIB_SRC:src/%.c=obj/src/%.o)
LIB_OBJ2 = $(LIB_SRC2:src/%.c=obj/src/%.o)
LIB_OBJ3 = $(LIB_SRC3:src/%.c=obj/src/%.o)
LIB_PATH = obj/src/lib$(PROJECT)/$(LIB_NAME).a
LIB_PATH2 = obj/src/lib$(PROJECT)/$(LIB_NAME2).a
LIB_PATH3 = obj/src/lib$(PROJECT)/$(LIB_NAME3).a

LIBS = $(LIB_PATH) $(LIB_PATH2) $(LIB_PATH3) $(FreeTypeARCHIVE)
OBJ = $(APP_OBJ) $(LIB_OBJ) $(LIB_OBJ2) $(LIB_OBJ3)
DEPS = $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)

DIRS = bin bin/$(PROJECT) obj obj/src obj/src/lib$(PROJECT)

.PHONY: all
all: $(DIRS) $(FreeTypeLIB_PATH) $(OBJ) $(APP_PATH)

-include $(DEPS)

$(APP_PATH): $(APP_OBJ) $(LIBS)
	gcc $< -o $@ $(LFLAGS)

$(LIB_PATH): $(LIB_OBJ)
	ar rcs $@ $^
$(LIB_PATH2): $(LIB_OBJ2)
	ar rcs $@ $^
$(LIB_PATH3): $(LIB_OBJ3)
	ar rcs $@ $^

$(OBJ):
	gcc $(CFLAGS) -o $@ -c $(@:obj/src/%.o=src/%.c)

run: all
	./$(APP_PATH)

.PHONY: clean
clean:
	rm -fr $(DIRS) $(FreeTypeARCHIVE)
#ifneq ($(wildcard $(FreeTypeARCHIVE)),)
#	cd $(FreeTypeARCHIVE) && make clean
#endif

.PHONY: format
format:
	git ls-files *.c | xargs clang-format -i --verbose && git diff --exit-code

.NOPARALLEL: $(DIRS)
.PHONY: $(DIRS)
$(DIRS):
	@if [ ! -d $@ ] ; then echo "creating $@"; mkdir $@; fi
	@if [ ! -d $@ ] ; then echo "$@ not created, error!"; exit 1; fi

$(FreeTypeLIB_PATH):
ifeq ($(wildcard $(FreeTypeARCHIVE)),)
	tar -jxf $(FreeTypeARCHIVE).tar.bz
endif
	cd $(FreeTypeARCHIVE) && make

.PHONY: archivate
archivate:
	tar -jcf $(FreeTypeARCHIVE).tar.bz $(FreeTypeARCHIVE)

