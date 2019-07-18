CFLAGS ?= -std=c89 -pedantic -march=native -Wall -O3
PREFIX ?= $(HOME)/.local
INCLUDEDIR ?= include
LIBDIR ?= lib
DATADIR ?= share/$(NAME)-$(VERSION)

DEPS := glfw3 glew libpng
NAME := game
LIB := lib$(NAME).a
VERSION ?= $(shell git describe --tags 2>/dev/null || printf '9999.%d.%s\n' "$$(git rev-list --count HEAD)" "$$(git rev-parse --short HEAD)")

CFLAGS += $(shell pkg-config --cflags $(DEPS)) -I.
LDLIBS += -lm $(shell pkg-config --libs-only-l $(DEPS))
LDFLAGS += $(shell pkg-config --libs-only-L --libs-only-other $(DEPS))

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
	rm -f $(wildcard $(LIB) $(NAME).pc src/*.o src/*/*.o test/*.o test/*/*.o $(TEST_EXECS) tags test/out/* test/scenes.h test/ubo.h)

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

.PHONY: install
D := $(if $(DESTDIR),$(DESTDIR)/)$(PREFIX)
install: $(LIB) $(NAME).pc
	mkdir -p $(D)/$(INCLUDEDIR) $(D)/$(LIBDIR)/pkgconfig $(D)/$(DATADIR)/$(NAME)
	cp -R $(NAME) $(D)/$(INCLUDEDIR)
	find $(D)/$(INCLUDEDIR) -type f -name '*.h' -exec sed -i 's,^\(#include <\)\(shaders/\),\1game/\2,' {} +
	cp -R shaders $(D)/$(DATADIR)/$(NAME)
	cp $(LIB) $(D)/$(LIBDIR)
	cp $(NAME).pc $(D)/$(LIBDIR)/pkgconfig

.PHONY: $(NAME).pc
$(NAME).pc:
	printf 'prefix=%s\nincludedir=%s\nlibdir=%s\ndatadir=%s\n\nName: %s\nDescription: %s\nVersion: %s\nCflags: %s\nLibs: %s\nRequires: %s' \
		'$(PREFIX)' \
		'$${prefix}/$(INCLUDEDIR)' \
		'$${prefix}/$(LIBDIR)' \
		'$${prefix}/$(DATADIR)' \
		'$(NAME)' \
		'$(NAME)' \
		'$(VERSION)' \
		'-I$${includedir} -I$${datadir} -DGAME_SHADERS_PATH=\"$${datadir}/$(NAME)/shaders\"' \
		'-L$${libdir} -l$(NAME) -lm' \
		'$(DEPS)' \
		> $@
