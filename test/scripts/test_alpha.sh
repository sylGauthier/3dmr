#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f test_scene ] || die "wrong directory"
mkdir -p out

assets grad.png || die "assets"

testing "alpha blending"; (
    ./test_scene alpha_blend "out/test_alpha_blend.png" >/dev/null 2>&1 || fail "screenshot"
    convert -gravity northwest '(' -size 94x94 gradient:'rgb(255,0,255)-black' ')' -background black -gravity center -extent 640x480 "out/expected_alpha_blend.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh -t 20 "out/test_alpha_blend.png" "out/expected_alpha_blend.png" >/dev/null 2>&1 || fail "cmp_img"
) && pass || set_failed

testing "alpha testing"; (
    ./test_scene alpha_test "out/test_alpha_test.png" >/dev/null 2>&1 || fail "screenshot"
    convert -size 640x480 xc:black -fill 'rgb(255,0,255)' -draw "rectangle 273,193 366,239" "out/expected_alpha_test.png" 2>/dev/null || fail "gen expected"
    ./scripts/cmp_img.sh -t 20 "out/test_alpha_test.png" "out/expected_alpha_test.png" >/dev/null 2>&1 || fail "cmp_img"
) && pass || set_failed

exit "$code"
