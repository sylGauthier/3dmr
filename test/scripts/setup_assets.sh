#!/bin/bash
[[ "${PWD##*/}" == "scripts" ]] && cd ..
if [[ "${PWD##*/}" != "test" ]]; then
    if [[ -d "test" ]]; then
	cd test
    else
	echo "Error: This must be run from the 'test/' directory" 1>&2
	exit 2
    fi
fi

# Usage: gen_png base.png out_dir
gen_png () {
    in_png="$1"
    name="${in_png##*/}"
    out_dir="$2"
    mkdir -p "$out_dir"
    convert "$in_png" -background white -alpha remove -flatten "${out_dir}/rgb_$name"
    convert "$in_png" -set colorspace Gray -separate -average "${out_dir}/gray_alpha_$name"
    convert "$in_png" -background white -alpha remove -flatten -set colorspace Gray -separate -average "${out_dir}/gray_$name"
    convert "$in_png" -colors 24 "${out_dir}/indexed24_alpha_$name"
    convert "$in_png" -background white -alpha remove -flatten -colors 24 "${out_dir}/indexed24_$name"
}

# Usage: setup_png out_dir
setup_png() {
    outdir="$1"
    mkdir -p "$outdir"
    base_png="${outdir}/tux.png"
    [[ -f "$base_png" ]] || wget -O "$base_png" "https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/200px-Tux.svg.png"
    gen_png "$base_png" "$outdir"
}

# Usage setup_font out_dir
setup_font () {
    outdir="$1"
    freefont_zip="assets/freefont-ttf.zip"
    [[ -f "$freefont_zip" ]] || wget -O "$freefont_zip" "https://ftp.gnu.org/gnu/freefont/freefont-ttf-20120503.zip"
    [[ -d "assets/freefont-20120503" ]] || unzip "$freefont_zip" -d "assets"

    mkdir -p "$outdir"
    for font in assets/freefont-20120503/*.ttf; do
	ln -frs "$font" "$outdir"
    done
    [[ -e "${outdir}/FreeSans.ttf" ]] && ln -fs "FreeSans.ttf" "${outdir}/test.ttf" || return 0
}

setup_png "assets/png/"

setup_font "assets/font/"

