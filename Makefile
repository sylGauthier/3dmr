DEPS := glfw3 glew libpng
LIB := libgame.a

CFLAGS ?= -std=c89 -pedantic -march=native -Wall -O3
CFLAGS += $(shell pkg-config --cflags $(DEPS)) -I.
LDLIBS += -lm $(shell pkg-config --libs $(DEPS))

LIB_OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c src/*/*.c))
TEST_EXECS := $(patsubst %.c,%,$(wildcard test/*.c))
TESTS := $(patsubst %.sh,%,$(notdir $(wildcard test/scripts/test_*.sh)))

PKG_CONFIG_CHECK := $(shell pkg-config --print-errors --short-errors --errors-to-stdout --exists $(DEPS) | sed "s/No package '\([^']*\)' found/\1/")
ifneq ($(PKG_CONFIG_CHECK),)
$(error Missing packages: $(PKG_CONFIG_CHECK))
endif

.PHONY: all
all: $(TEST_EXECS)

$(LIB): $(LIB_OBJECTS)
	$(AR) rcs $@ $^

.PHONY: clean
clean:
	rm -f $(wildcard $(LIB) src/*.o src/*/*.o test/*.o test/*/*.o $(TEST_EXECS) tags test/out/* test/scenes.h test/ubo.h)

tags: $(wildcard src/*.c src/*/*.c)
	ctags $^

.PHONY: test $(TESTS)
test: all
	@+$(MAKE) -k $(TESTS)
$(TESTS): test_%: ./test/scripts/test_%.sh
	@$<

test/scenes.h: $(wildcard test/scenes/*.c)
	for f in $(patsubst test/scenes/%.c,%,$^); do printf 'int %s_setup(struct Scene*);\nvoid %s_teardown(struct Scene*);\n' "$$f" "$$f"; done > $@
	printf 'static const struct DemoScene {\n    const char* name;\n    int (*setup)(struct Scene*);\n    void (*teardown)(struct Scene*);\n} scenes[] = {\n' >> $@
	for f in $(patsubst test/scenes/%.c,%,$^); do printf '    {"%s", %s_setup, %s_teardown},\n' "$$f" "$$f" "$$f"; done >> $@
	printf '    {0}\n};\n' >> $@
	printf '#define NUM_DEMO_SCENES %d\n' $(words $^) >> $@
test/test_scene: test/test_scene.c $(wildcard test/scenes/*.c) | test/scenes.h

test/ubo.o: test/ubo.h
test/ubo.h: src/render/lights_buffer_object.c src/render/camera_buffer_object.c
	grep -h '^#define' $^ > $@
$(TEST_EXECS): $(LIB)
