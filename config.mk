# Build options
CFLAGS ?= -std=c89 -pedantic -march=native -Wall -O3

# Install prefixes
PREFIX ?= $(HOME)/.local
INCLUDEDIR ?= include
LIBDIR ?= lib
DATADIR ?= share/$(NAME)-$(VERSION)

# 3dmr features
OPENGEX=1
TTF ?=
