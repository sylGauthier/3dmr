DEPS := glfw3 glew libpng freetype2
APP := game
LIB := libgame.a

CFLAGS ?= -std=c89 -pedantic -march=native -Wall -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200112L -O3
CFLAGS += -DM_PI=3.14159265358979 $(shell pkg-config --cflags $(DEPS)) -I. -Isrc
LDLIBS += -lm $(shell pkg-config --libs $(DEPS))
LIB_SOURCES := $(wildcard src/*.c src/geometry/*.c src/mesh/*.c src/light/*.c src/img/*.c)
LIB_OBJECTS := $(LIB_SOURCES:.c=.o)
TEST_SOURCES := $(wildcard test/*.c)
TEST_MAINS := $(wildcard test/main/*.c)
TEST_EXECS := $(basename $(TEST_MAINS))
TEST_SOURCES := $(filter-out $(TEST_MAINS),$(TEST_SOURCES))
TEST_OBJECTS := $(TEST_SOURCES:.c=.o)

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
	rm -f $(wildcard $(APP) $(APP).o $(LIB) $(LIB_OBJECTS) $(APP_OBJECTS) test/out/* $(TEST_OBJECTS) $(addsuffix .o,$(TEST_EXECS)) $(TEST_EXECS) tags)

tags: $(LIB_SOURCES)
	ctags $^

.PHONY: test test-assets clean-assets
test: all
	$(foreach test,$(wildcard test/scripts/test_*.sh),$(test);)
test-assets:
	@+$(MAKE) --no-print-directory -C test/assets all
clean-assets:
	@+$(MAKE) --no-print-directory -C test/assets clean

$(TEST_EXECS): %:%.o $(TEST_OBJECTS) $(LIB)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
