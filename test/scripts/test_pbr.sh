#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f test_scene ] || die "wrong directory"
mkdir -p out

testing "no light"; (
    ./test_scene pbr_no_light "out/pbr_no_light.png" >/dev/null 2>&1 || fail "screenshot"
    convert -size 640x480 xc:black "out/expected_pbr_no_light.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh "out/pbr_no_light.png" "out/expected_pbr_no_light.png" 2>/dev/null || fail "cmp_img"
) && pass || set_failed

testing "directional back"; (
    ./test_scene pbr_directional_back "out/pbr_directional_back.png" >/dev/null 2>&1 || fail "screenshot"
    convert "out/pbr_directional_back.png" -fill none -stroke black -strokewidth 3 -draw "circle 320,240 320,326" "out/mod_pbr_directional_back.png" 2>/dev/null || fail "gen mod"
    convert -size 640x480 xc:black "out/expected_pbr_directional_back.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh "out/mod_pbr_directional_back.png" "out/expected_pbr_directional_back.png" 2>/dev/null || fail "cmp_img"
) && pass || set_failed

testing "directional front"; (
    ./test_scene pbr_directional_front "out/pbr_directional_front.png" >/dev/null 2>&1 || fail "screenshot"
    printf '# ImageMagick pixel enumeration: 1,2,255,srgb\n0,0:(0,0,0)\n0,1:(128,0,128)\n' | convert "out/pbr_directional_front.png" +dither -remap txt:- -fill none -stroke white -strokewidth 8 -draw "circle 320,240 320,322" "out/mod_pbr_directional_front.png" || fail "gen_mod"
    convert -size 640x480 xc:black -depth 8 -fill 'rgb(128,0,128)' -draw "circle 320,240 320,322" -fill none -stroke white -strokewidth 8 -draw "circle 320,240 320,322" "out/expected_pbr_directional_front.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh "out/mod_pbr_directional_front.png" "out/expected_pbr_directional_front.png" 2>/dev/null || fail "cmp_img"
) && pass || set_failed

exit "$code"
