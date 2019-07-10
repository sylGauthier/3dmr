#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f test_scene ] || die "wrong directory"
mkdir -p out

assets rgb_tux.png tux.png || die "assets"

testing "small cube - tux"; (
    ./test_scene solid_texture "out/test_solid_texture.png" >/dev/null 2>&1 || fail "screenshot"
    convert assets/tux.png -interpolate Nearest -filter point -geometry '94x94!' -flatten -gravity center -background black -extent 640x480 "out/expected_solid_texture.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh -t 5 "out/test_solid_texture.png" "out/expected_solid_texture.png" >/dev/null 2>&1 || fail "cmp_img"
) && pass || set_failed

exit "$code"
