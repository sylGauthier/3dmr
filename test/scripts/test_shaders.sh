#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f glslcheck ] || die "wrong directory"
mkdir -p out

for shader in ../shaders/*.{vert,frag}; do
    testing "$(basename "$shader")"; (
        ./glslcheck "$shader" 2>/dev/null || fail "compilation failed"
    ) && pass || set_failed
done

exit "$code"
