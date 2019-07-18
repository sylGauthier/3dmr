#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f test_scene ] || die "wrong directory"
mkdir -p out

assets rgb_tux.png tux.png || die "assets"

testing "small cube"; (
    ./test_scene solid_color_small "out/test_solid_color.png" >/dev/null 2>&1 || fail "screenshot"
    convert -size 640x480 xc:black -fill 'rgb(255,0,255)' -draw "rectangle 273,193 366,286" "out/expected_solid_color.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh "out/test_solid_color.png" "out/expected_solid_color.png" >/dev/null 2>&1 || fail "cmp_img"
) && pass || set_failed

testing "big cube"; (
    ./test_scene solid_color_big "out/test_solid_color_big.png" >/dev/null 2>&1 || fail "screenshot"
    convert -size 640x480 xc:'rgb(255,0,255)' "out/expected_solid_color_big.png" >/dev/null 2>&1 || fail "gen expected"
    ./scripts/cmp_img.sh "out/test_solid_color_big.png" "out/expected_solid_color_big.png" >/dev/null 2>&1 || fail "cmp_img"
) && pass || set_failed

testing "small cube - tux"; (
    ./test_scene solid_texture "out/test_solid_texture.png" >/dev/null 2>&1 || fail "screenshot"
    convert assets/tux.png -interpolate Nearest -filter point -geometry '94x94!' -flatten -gravity center -background black -extent 640x480 "out/expected_solid_texture.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh -t 5 "out/test_solid_texture.png" "out/expected_solid_texture.png" >/dev/null 2>&1 || fail "cmp_img"
) && pass || set_failed

exit "$code"
