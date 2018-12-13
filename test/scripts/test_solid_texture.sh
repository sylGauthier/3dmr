#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f test_solid_texture ] || die "wrong directory"
mkdir -p out

testing "small cube - tux"; (
    ./test_solid_texture -t 1 -s "out/test_solid_texture.png" 2.0165 2>&- || fail "screenshot"
    convert assets/textures/tux.png -flip -interpolate Nearest -filter point -geometry '94x94!' -flatten -gravity center -background black -extent 640x480 "out/expected_solid_texture.png" 2>&- || fail "gen expected"
    ./scripts/cmp_img.sh "out/test_solid_texture.png" "out/expected_solid_texture.png" 2>&- || fail "cmp_img"
) && pass || set_failed

exit "$code"
