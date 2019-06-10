#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f png_rw ] || die "wrong directory"
mkdir -p out

for png in assets/textures/*tux.png; do
    testing "$(basename "$png")"; (
        ./png_rw "$png" "out/${png##*/}" 2>/dev/null || fail "png_rw"
        ./scripts/cmp_img.sh "$png" "out/${png##*/}" 2>/dev/null || fail "cmp_img"
    ) && pass || set_failed
done

exit "$code"
