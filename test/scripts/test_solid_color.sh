#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f test_solid_color ] || die "wrong directory"
mkdir -p out

testing "solid color"
./test_solid_color "out/test_solid_color.png" 2>&- || fail "screenshot"
convert "out/test_solid_color.png" -fill transparent -stroke white -draw "rectangle 273,193 366,286" "out/mod_solid_color.png" || fail "gen mod"
convert -size 640x480 xc:black -fill 'rgb(255,0,255)' -stroke white -draw "rectangle 273,193 366,286" -type Palette "out/expected_solid_color.png" || fail "gen expected"
./scripts/cmp_img.sh "out/mod_solid_color.png" "out/expected_solid_color.png" 2>&- && pass || fail "cmp_img"
