CC=clang

CFLAGS=\
	-std=c11 \
	-fshort-enums \
	-fsanitize=address,undefined \
	-g3 \
	-W \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wimplicit-fallthrough \
	-Wimplicit-int-conversion \
	-Wshadow \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Wno-unused-parameter

NAME=kobresia
BUILD_DIR=out
HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.o))

all: $(BUILD_DIR)/$(NAME) tidy

$(BUILD_DIR)/$(NAME): $(OBJECTS)
	@ mkdir -p $(BUILD_DIR)
	@ $(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c $(HEADERS)
	@ mkdir -p $(BUILD_DIR)
	@ $(CC) $(CFLAGS) -c -o $@ $<

tidy: $(HEADERS) $(SOURCES)
	@ clang-format -i $^

clean:
	@ rm -r $(BUILD_DIR)

.PHONY: all tidy clean
