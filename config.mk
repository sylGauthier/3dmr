# Build options
CFLAGS ?= -std=c99 -pedantic -march=native -Wall -O3

# Install prefixes
PREFIX ?= $(HOME)/.local
INCLUDEDIR ?= include
LIBDIR ?= lib
DATADIR ?= share/$(NAME)-$(VERSION)

# 3dmr features (set to 1 to enable/empty to disable)
OPENGEX ?=
TTF ?=
GLTF ?=
