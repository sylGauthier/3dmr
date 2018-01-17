DEPS := glfw3 glew libpng freetype2
APP := game
LIB := libgame.a

CFLAGS ?= -std=c89 -pedantic -march=native -Wall -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200112L -O3
CFLAGS += -DM_PI=3.14159265358979 $(shell pkg-config --cflags $(DEPS)) -I. -Isrc
LDLIBS += -lm $(shell pkg-config --libs $(DEPS))
LIB_SOURCES := $(wildcard src/*.c src/geometry/*.c src/mesh/*.c src/light/*.c)
LIB_OBJECTS := $(LIB_SOURCES:.c=.o)

PKG_CONFIG_CHECK := $(shell pkg-config --print-errors --short-errors --errors-to-stdout --exists $(DEPS) | sed "s/No package '\([^']*\)' found/\1/")
ifneq ($(PKG_CONFIG_CHECK),)
$(error Missing packages: $(PKG_CONFIG_CHECK))
endif

.PHONY: all
all: $(APP) textures/tux.png

$(APP): $(APP).o $(LIB)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(LIB): $(LIB_OBJECTS)
	$(AR) rcs $@ $^

.PHONY: clean
clean:
	rm -f $(wildcard $(APP) $(APP).o $(LIB) $(LIB_OBJECTS))

tags: $(LIB_SOURCES)
	ctags $^

textures/tux.png:
	mkdir -p textures
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/200px-Tux.svg.png"
	convert $@ -background white -alpha remove -flatten $@
