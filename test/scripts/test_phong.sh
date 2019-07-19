#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f test_scene ] || die "wrong directory"
mkdir -p out

testing "no light"; (
    ./test_scene phong_no_light "out/phong_no_light.png" >/dev/null 2>&1 || fail "screenshot"
    convert -size 640x480 xc:black "out/expected_phong_no_light.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh "out/phong_no_light.png" "out/expected_phong_no_light.png" 2>/dev/null || fail "cmp_img"
) && pass || set_failed

testing "ambient only"; (
    ./test_scene phong_ambient_only "out/phong_ambient_only.png" >/dev/null 2>&1 || fail "screenshot"
    convert "out/phong_ambient_only.png" -fill none -stroke white -strokewidth 3 -draw "circle 320,240 320,326" "out/mod_phong_ambient_only.png" 2>/dev/null || fail "gen mod"
    convert -size 640x480 xc:black -depth 8 -fill 'rgb(25,0,25)' -draw "circle 320,240 320,325" -fill none -stroke white -strokewidth 3 -draw "circle 320,240 320,326" "out/expected_phong_ambient_only.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh -t 20 "out/mod_phong_ambient_only.png" "out/expected_phong_ambient_only.png" 2>/dev/null || fail "cmp_img"
) && pass || set_failed

testing "directional back"; (
    ./test_scene phong_directional_back "out/phong_directional_back.png" >/dev/null 2>&1 || fail "screenshot"
    convert "out/phong_directional_back.png" -fill none -stroke black -strokewidth 3 -draw "circle 320,240 320,326" "out/mod_phong_directional_back.png" 2>/dev/null || fail "gen mod"
    convert -size 640x480 xc:black "out/expected_phong_directional_back.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh "out/mod_phong_directional_back.png" "out/expected_phong_directional_back.png" 2>/dev/null || fail "cmp_img"
) && pass || set_failed

testing "directional front"; (
    ./test_scene phong_directional_front "out/phong_directional_front.png" >/dev/null 2>&1 || fail "screenshot"
    printf '# ImageMagick pixel enumeration: 1,2,255,srgb\n0,0:(0,0,0)\n0,1:(128,0,128)\n' | convert "out/phong_directional_front.png" +dither -remap txt:- -fill none -stroke white -strokewidth 8 -draw "circle 320,240 320,322" "out/mod_phong_directional_front.png" || fail "gen_mod"
    convert -size 640x480 xc:black -depth 8 -fill 'rgb(128,0,128)' -draw "circle 320,240 320,322" -fill none -stroke white -strokewidth 8 -draw "circle 320,240 320,322" "out/expected_phong_directional_front.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh "out/mod_phong_directional_front.png" "out/expected_phong_directional_front.png" 2>/dev/null || fail "cmp_img"
) && pass || set_failed

exit "$code"
