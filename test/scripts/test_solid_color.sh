#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f test_solid_color ] || die "wrong directory"
mkdir -p out

testing "small cube"; (
    ./test_solid_color -t 1 -s "out/test_solid_color.png" 2.0165 2>&- || fail "screenshot"
    convert -size 640x480 xc:black -fill 'rgb(255,0,255)' -draw "rectangle 273,193 366,286" "out/expected_solid_color.png" 2>&- || fail "gen expected"
    ./scripts/cmp_img.sh "out/test_solid_color.png" "out/expected_solid_color.png" 2>&- || fail "cmp_img"
) && pass || set_failed

testing "big cube"; (
    ./test_solid_color -t 1 -s "out/test_solid_color_big.png" 40.0 2>&- || fail "screenshot"
    convert -size 640x480 xc:'rgb(255,0,255)' "out/expected_solid_color_big.png" 2>&- || fail "gen expected"
    ./scripts/cmp_img.sh "out/test_solid_color_big.png" "out/expected_solid_color_big.png" 2>&- || fail "cmp_img"
) && pass || set_failed

exit "$code"
