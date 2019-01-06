#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f demo ] || die "wrong directory"
mkdir -p out

testing "small cube - tux"; (
    ./demo -t 1 -s "out/test_solid_texture.png" "cube:2.0165;solid:texture(rgb_tux)" 2>&- || fail "screenshot"
    convert assets/textures/tux.png -interpolate Nearest -filter point -geometry '94x94!' -flatten -gravity center -background black -extent 640x480 "out/expected_solid_texture.png" 2>&- || fail "gen expected"
    ./scripts/cmp_img.sh -t 5 "out/test_solid_texture.png" "out/expected_solid_texture.png" 2>&- || fail "cmp_img"
) && pass || set_failed

exit "$code"
