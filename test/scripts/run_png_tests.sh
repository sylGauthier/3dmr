#!/bin/bash
if [[ "$_" != "$0" ]]
then
    echo "FATAL: do not source this file !"
else
    error () {
	echo "Error: $1" 1>&2 && exit 2;
    }
    [[ "${PWD##*/}" == "scripts" ]] && cd ..
    [[ "${PWD##*/}" == "test" ]] || error "This must be run from the 'test/' directory"
    export PATH=./scripts/:./main:$PATH

    gen_png () {
	rgba_png=$1
	out_dir=$2
	mkdir -p "$out_dir"
	convert "$rgba_png" -background white -alpha remove -flatten "${out_dir}/rgb_$rgba_png"
	convert "$rgba_png" -set colorspace Gray -separate -average "${out_dir}/gray_alpha_$rgba_png"
	convert "$rgba_png" -background white -alpha remove -flatten -set colorspace Gray -separate -average "${out_dir}/gray_$rgba_png"
	convert "$rgba_png" -colors 24 "${out_dir}/indexed24_alpha_$rgba_png"
	convert "$rgba_png" -background white -alpha remove -flatten -colors 24 "${out_dir}/indexed24_$rgba_png"
    }
    [[ -e tux.png ]] || wget -O tux.png "https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/200px-Tux.svg.png"
    gen_png tux.png "assets/"

    mkdir -p out/
    for png in $(find "assets/" -type f -name "*.png")
    do
	echo -n "testing $png ... "
	$(exec png_rw "$png" "out/${png##*/}")
	if [ $? -ne 0 ]; then
	    echo "png_rw exited with error"
	else
	    $(cmp_img.sh "$png" "out/${png##*/}") && echo "PASSED" || echo "FAIL"
	fi
    done
fi
