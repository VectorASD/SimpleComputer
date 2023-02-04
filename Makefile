APP_NAME = main
LIB_NAME = libyeah
PROJECT = simplecomputer

CFLAGS = -Wall -Werror -I include
LFLAGS = -lm

APP_SRC = $(wildcard src/*.c)
LIB_SRC = $(wildcard src/lib$(PROJECT)/*.c)
APP_OBJ = $(APP_SRC:src/%.c=obj/src/%.o)
LIB_OBJ = $(LIB_SRC:src/%.c=obj/src/%.o)
OBJ = $(APP_OBJ) $(LIB_OBJ)

APP_PATH = bin/$(PROJECT)/$(APP_NAME)
LIB_PATH = obj/src/lib$(PROJECT)/$(LIB_NAME).a

.PHONY: all
all: $(OBJ) $(APP_PATH)

$(APP_PATH): $(APP_OBJ) $(LIB_PATH)
	gcc $^ -o $@ $(LFLAGS)

$(LIB_PATH): $(LIB_OBJ)
	ar rcs $@ $^

$(OBJ):
	gcc $(CFLAGS) -o $@ -c $(@:obj/src/%.o=src/%.c)

run: all
	./$(APP_PATH)

.PHONY: clean
clean:
	rm -f $(APP_PATH) $(LIB_PATH) $(OBJ)

.PHONY: format
format:
	git ls-files *.c | xargs clang-format -i --verbose && git diff --exit-code
