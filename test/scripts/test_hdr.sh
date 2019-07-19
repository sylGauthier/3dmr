#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f hdr2png ] || die "wrong directory"
mkdir -p out

assets rgb_tux.png || die "assets"

for png in assets/rgb_tux.png; do
    testing "$(basename "$png")"; (
        convert "$png" "out/$(basename "$png" .png).hdr" 2>/dev/null || fail "convert to hdr"
        ./hdr2png "out/$(basename "$png" .png).hdr" "out/$(basename "$png" .png).hdr.png" 2>/dev/null || fail "hdr2png"
        ./scripts/cmp_img.sh -t 2500 "$png" "out/$(basename "$png" .png).hdr.png" 2>/dev/null || fail "cmp_img"
    ) && pass || set_failed
done

exit "$code"
