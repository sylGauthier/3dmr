# Build options
CFLAGS ?= -fPIC -std=c89 -pedantic -march=native -Wall -O3

# Install prefixes
PREFIX ?= $(HOME)/.local
INCLUDEDIR ?= include
LIBDIR ?= lib
DATADIR ?= share/$(NAME)-$(VERSION)

# game features
OPENGEX=1
