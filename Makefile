DEPS := glfw3 glew libpng
APP := game

CFLAGS ?= -std=c89 -pedantic -march=native -Wall -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200112L -O3
CFLAGS += -DM_PI=3.14159265358979 $(shell pkg-config --cflags $(DEPS)) -I. -Isrc
LDLIBS += -lm $(shell pkg-config --libs $(DEPS))
C_FILES := $(wildcard src/*.c src/geometries/*.c)
OBJECTS := $(C_FILES:.c=.o)

.PHONY: all
all: check_deps $(APP)

$(APP): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: check_deps
check_deps:
	@pkg-config --exists $(DEPS) || (echo "One of the following dependencies was not found: $(DEPS)"; false)

.PHONY: clean
clean:
	rm -f $(APP) $(OBJECTS)

tags: $(C_FILES)
	ctags $^

$(APP): textures/tux.png
textures/tux.png:
	mkdir -p textures
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/200px-Tux.svg.png"
	convert $@ -background white -alpha remove -flatten $@
