CC := clang

CFLAGS := \
	-std=c11 \
	-fshort-enums \
	-ftrivial-auto-var-init=zero \
	-W \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wconversion \
	-Wimplicit-fallthrough \
	-Wmissing-prototypes \
	-Wshadow \
	-Wstrict-prototypes \
	-Wno-unused-parameter

NAME := kobresia
HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.c)

BUILD_DIR := out
BUILD_DIR_DEVELOP := $(BUILD_DIR)/develop
BUILD_DIR_RELEASE := $(BUILD_DIR)/release

DEVELOP_OBJECTS += $(addprefix $(BUILD_DIR_DEVELOP)/, $(SOURCES:.c=.o))
RELEASE_OBJECTS += $(addprefix $(BUILD_DIR_RELEASE)/, $(SOURCES:.c=.o))

all: develop tidy

test: develop
	@ $(BUILD_DIR_DEVELOP)/$(NAME) --test

develop: CFLAGS += -fsanitize=address,undefined -g3 -DDEVELOP
develop: $(BUILD_DIR_DEVELOP)/$(NAME)

release: CFLAGS += -O1 -flto -fwrapv
release: $(BUILD_DIR_RELEASE)/$(NAME)

$(BUILD_DIR_DEVELOP)/$(NAME): $(DEVELOP_OBJECTS) | $(BUILD_DIR_DEVELOP)
	@ printf "LD $@\n"
	@ $(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR_RELEASE)/$(NAME): $(RELEASE_OBJECTS) | $(BUILD_DIR_RELEASE)
	@ printf "LD $@\n"
	@ $(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR_DEVELOP)/%.o: %.c $(HEADERS) | $(BUILD_DIR_DEVELOP)
	@ printf "CC $@\n"
	@ $(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR_RELEASE)/%.o: %.c $(HEADERS) | $(BUILD_DIR_RELEASE)
	@ printf "CC $@\n"
	@ $(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR_DEVELOP):
	@ mkdir -p $@

$(BUILD_DIR_RELEASE):
	@ mkdir -p $@

tidy: $(HEADERS) $(SOURCES)
	@ clang-format -i $^

clean:
	@ rm -rf $(BUILD_DIR)

.PHONY: all develop release test tidy clean
