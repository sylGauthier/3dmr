include config.mk

DEPS := glfw3 glew libpng libjpeg $(if $(OPENGEX),liboddl) $(if $(TTF),libsfnt) $(if $(GLTF),jansson)
NAME := 3dmr
LIB := lib$(NAME).a
VERSION ?= $(shell git describe --tags 2>/dev/null || printf '9999-%d-%s\n' "$$(git rev-list --count HEAD)" "$$(git rev-parse --short HEAD)")

CFLAGS += -I. $(shell pkg-config --cflags $(DEPS))
LDLIBS += -lm $(shell pkg-config --libs-only-l $(DEPS))
LDFLAGS += $(shell pkg-config --libs-only-L --libs-only-other $(DEPS))

LIB_OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c src/*/*.c))
LIB_OBJECTS := $(if $(OPENGEX),$(LIB_OBJECTS),$(filter-out src/opengex/%,$(LIB_OBJECTS)))
LIB_OBJECTS := $(if $(TTF),$(LIB_OBJECTS),$(filter-out src/font/%,$(LIB_OBJECTS)))
LIB_OBJECTS := $(if $(GLTF),$(LIB_OBJECTS),$(filter-out src/gltf/%,$(LIB_OBJECTS)))
TEST_EXECS := $(patsubst %.c,%,$(wildcard test/*.c))
TEST_EXECS := $(if $(or $(OPENGEX),$(GLTF)),$(TEST_EXECS),$(filter-out test/ogex% test/3dmrview%,$(TEST_EXECS)))
TEST_EXECS := $(if $(TTF),$(TEST_EXECS),$(filter-out test/font,$(TEST_EXECS)))
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
	rm -f $(wildcard $(LIB) $(NAME).pc src/*.o src/*/*.o test/*.o test/*/*.o $(TEST_EXECS) test/out/* test/scenes.h test/ubo.h)

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

SHADERS_PATH_SRC := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/shaders
SHADERS_PATH_DIST := $(PREFIX)/$(DATADIR)/shaders
CFLAGS += -DTDMR_SHADERS_PATH_SRC=\"$(SHADERS_PATH_SRC)\" -DTDMR_SHADERS_PATH_DIST=\"$(SHADERS_PATH_DIST)\"
$(TEST_EXECS): CFLAGS += $(if $(OPENGEX), -DTDMR_OPENGEX=1)
$(TEST_EXECS): CFLAGS += $(if $(GLTF), -DTDMR_GLTF=1)

.PHONY: install
D := $(if $(DESTDIR),$(DESTDIR)/)$(PREFIX)
install: $(LIB) $(NAME).pc $(if $(or $(OPENGEX),$(GLTF)),test/3dmrview)
	mkdir -p $(D)/$(INCLUDEDIR) $(D)/$(LIBDIR)/pkgconfig $(D)/$(DATADIR) $(D)/bin
	cp -R $(NAME) $(D)/$(INCLUDEDIR)
	cp -R shaders $(D)/$(DATADIR)
	cp $(LIB) $(D)/$(LIBDIR)
	cp $(NAME).pc $(D)/$(LIBDIR)/pkgconfig
	$(if $(or $(OPENGEX),$(GLTF)),cp test/3dmrview $(D)/bin/)

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
		'-I$${includedir} -I$${datadir} -DTDMR_SHADERS_PATH=\"$${datadir}/shaders\"$(if $(OPENGEX), -DTDMR_OPENGEX=1)$(if $(GLTF), -DTDMR_GLTF=1)' \
		'-L$${libdir} -l$(NAME) -lm' \
		'$(DEPS)' \
		> $@
