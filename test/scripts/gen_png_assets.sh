#!/bin/bash
gen_png () {
    in_png="$1"
    name="${in_png##*/}"
    out_dir="$2"
    convert "$in_png" -background white -alpha remove -flatten "${out_dir}/rgb_$name"
    convert "$in_png" -set colorspace Gray -separate -average "${out_dir}/gray_alpha_$name"
    convert "$in_png" -background white -alpha remove -flatten -set colorspace Gray -separate -average "${out_dir}/gray_$name"
    convert "$in_png" -colors 24 "${out_dir}/indexed24_alpha_$name"
    convert "$in_png" -background white -alpha remove -flatten -colors 24 "${out_dir}/indexed24_$name"
}
base_png="assets/tux.png"
outdir="assets"

[[ "${PWD##*/}" == "scripts" ]] && cd ..
if [[ "${PWD##*/}" != "test" ]]; then
    if [[ -d "test" ]]; then
	cd test
    else
	echo "Error: This must be run from the 'test/' directory" 1>&2
	exit 2
    fi
fi

mkdir -p "$outdir"
[[ -f "$base_png" ]] || wget -O "$base_png" "https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/200px-Tux.svg.png"
gen_png "$base_png" "$outdir"
