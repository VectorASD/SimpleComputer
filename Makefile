APP_NAME = main
LIB_NAME = libyeah
LIB_NAME2 = libmyterm
PROJECT = simplecomputer

CFLAGS = -Wall -Werror -I include -MMD
LFLAGS = -lm

APP_SRC = $(wildcard src/*.c)
APP_OBJ = $(APP_SRC:src/%.c=obj/src/%.o)

LIB_SRC = src/lib$(PROJECT)/lib.c
LIB_SRC2 = src/lib$(PROJECT)/myTerm.c
LIB_OBJ = $(LIB_SRC:src/%.c=obj/src/%.o)
LIB_OBJ2 = $(LIB_SRC2:src/%.c=obj/src/%.o)
LIB_PATH = obj/src/lib$(PROJECT)/$(LIB_NAME).a
LIB_PATH2 = obj/src/lib$(PROJECT)/$(LIB_NAME2).a

OBJ = $(APP_OBJ) $(LIB_OBJ) $(LIB_OBJ2)
DEPS = $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)

APP_PATH = bin/$(PROJECT)/$(APP_NAME)

.PHONY: all
all: $(OBJ) $(APP_PATH)

-include $(DEPS)

$(APP_PATH): $(APP_OBJ) $(LIB_PATH) $(LIB_PATH2)
	gcc $^ -o $@ $(LFLAGS)

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
	rm -f $(APP_PATH) $(LIB_PATH) $(LIB_PATH2) $(OBJ) $(DEPS)

.PHONY: format
format:
	git ls-files *.c | xargs clang-format -i --verbose && git diff --exit-code
