#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f test_scene ] || die "wrong directory"
mkdir -p out

assets rgb_tux.png tux.png tux.jpeg || die "assets"

do_test "small cube" <<END
    ./test_scene solid_color_small "out/test_solid_color.png" || fail "screenshot"
    convert -size 640x480 xc:black -fill 'rgb(255,0,255)' -draw "rectangle 273,193 366,286" "out/expected_solid_color.png" || fail "gen expected"
    ./scripts/cmp_img.sh "out/test_solid_color.png" "out/expected_solid_color.png" || fail "cmp_img"
END

do_test "big cube" <<END
    ./test_scene solid_color_big "out/test_solid_color_big.png" || fail "screenshot"
    convert -size 640x480 xc:'rgb(255,0,255)' "out/expected_solid_color_big.png" || fail "gen expected"
    ./scripts/cmp_img.sh "out/test_solid_color_big.png" "out/expected_solid_color_big.png" || fail "cmp_img"
END

do_test "small cube - tux png" <<END
    ./test_scene solid_texture "out/test_solid_texture.png" || fail "screenshot"
    convert assets/tux.png -interpolate Nearest -filter point -geometry '94x94!' -flatten -gravity center -background black -extent 640x480 "out/expected_solid_texture.png" || fail "gen expected"
    ./scripts/cmp_img.sh -t 5 "out/test_solid_texture.png" "out/expected_solid_texture.png" || fail "cmp_img"
END

do_test "small cube - tux jpeg" <<END
    ./test_scene solid_jpeg_texture "out/test_solid_jpeg_texture.png" || fail "screenshot"
    convert assets/tux.jpeg -interpolate Nearest -filter point -geometry '94x94!' -flatten -gravity center -background black -extent 640x480 "out/expected_solid_jpeg_texture.png" || fail "gen expected"
    ./scripts/cmp_img.sh -t 5 "out/test_solid_jpeg_texture.png" "out/expected_solid_jpeg_texture.png" || fail "cmp_img"
END

exit "$code"
