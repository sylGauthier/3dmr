DEPS := glfw3 glew libpng freetype2
APP := game
LIB := libgame.a

CFLAGS ?= -std=c89 -pedantic -march=native -Wall -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200112L -O3
CFLAGS += $(shell pkg-config --cflags $(DEPS)) -I.
LDLIBS += -lm $(shell pkg-config --libs $(DEPS))

LIB_OBJECTS := $(patsubst %.c,%.o,$(wildcard game/*.c game/*/*.c))
TEST_EXECS := $(patsubst %.c,%,$(wildcard test/*.c))
TESTS := $(patsubst %.sh,%,$(notdir $(wildcard test/scripts/test_*.sh)))
DEMO_UTIL := $(patsubst %.c,%.o,$(wildcard test/demo_util/*.c))

PKG_CONFIG_CHECK := $(shell pkg-config --print-errors --short-errors --errors-to-stdout --exists $(DEPS) | sed "s/No package '\([^']*\)' found/\1/")
ifneq ($(PKG_CONFIG_CHECK),)
$(error Missing packages: $(PKG_CONFIG_CHECK))
endif

.PHONY: all
all: $(TEST_EXECS) test-assets

$(LIB): $(LIB_OBJECTS)
	$(AR) rcs $@ $^

.PHONY: clean
clean:
	rm -f $(wildcard $(LIB) game/*.o game/*/*.o test/*.o test/*/*.o $(TEST_EXECS) tags test/out/*)

tags: $(wildcard game/*.c game/*/*.c test/demo_util/*.c)
	ctags $^

.PHONY: test test-assets clean-assets $(TESTS)
test: all
	@+$(MAKE) -k $(TESTS)
$(TESTS): test_%: ./test/scripts/test_%.sh
	@$<
test-assets clean-assets:
	@+$(MAKE) --no-print-directory -C test/assets $@

$(TEST_EXECS): | $(LIB)
$(TEST_EXECS): LDLIBS+=$(LIB)
test/demo: test/demo.o $(DEMO_UTIL)
