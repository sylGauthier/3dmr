#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f glslcheck ] || die "wrong directory"
mkdir -p out

for shader in ../shaders/*.{vert,frag}; do
    do_test "$(basename "$shader")" <<END
        ./glslcheck -DHAVE_NORMAL "$shader" 2>/dev/null || fail "compilation failed"
END
done

exit "$code"
