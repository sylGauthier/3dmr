#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f png_rw ] || die "wrong directory"
mkdir -p out

assets rgb_tux.png gray_alpha_tux.png gray_tux.png indexed24_alpha_tux.png indexed24_tux.png tux.png || die "assets"

for png in assets/*tux.png; do
    do_test "$(basename "$png")" <<END
        ./png_rw "$png" "out/${png##*/}" 2>/dev/null || fail "png_rw"
        ./scripts/cmp_img.sh "$png" "out/${png##*/}" 2>/dev/null || fail "cmp_img"
END
done

exit "$code"
