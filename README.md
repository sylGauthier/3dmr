## About

`3dmr` is a minimal, modular and efficient 3D renderer, written entirely in ANSI
C. It supports many features commonly found in game engines such as hierarchical
scene layout, keyframed animations with various interpolation methods, partial
`gltf` and `opengex` support.

It is written with an emphasis on clarity, simplicity, and versatility. No
structures are hidden from the user, who is free to tweak the behaviour down to
the openGL call.

## Install

`3dmr` is meant to build into a static library `3dmr.a`. This means that only
the developer needs to build and install it. A program using `3dmr` will embed
the library directly, reducing the dependencies for the end user.

It declares itself to the system using `pkg-config` so make sure you have it
installed.

To build and install `3dmr`:

```
git clone https://github.com/sylGauthier/3dmr
cd 3dmr
sudo make install PREFIX=/usr/local GLTF=1
```

For more info, check out its [main page](https://pedantic.software/projects/3dmr.html).
