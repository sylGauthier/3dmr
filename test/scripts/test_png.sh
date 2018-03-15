#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f main/png_rw ] || die "wrong directory"
mkdir -p out

for png in $(find "assets/" -type f -name "*.png"); do
    testing "$png"
    main/png_rw "$png" "out/${png##*/}" 2>&- || fail "png_rw"
    scripts/cmp_img.sh "$png" "out/${png##*/}" 2>&- && pass || fail "cmp_img"
done
