#!/bin/bash

div() {
    dc -e "3k$1 $2/p"
}

case "$(($1))" in
    0) texture="earth_central_cylindric"; uv="cylindric,$(div 702 1024),$(div 702 1024),1";;
    1) texture="earth_mercator"; uv="mercator,$(div 2044 2048),$(div 1730 2048),1";;
    2) texture="earth_miller"; uv="miller,$(div 2044 2048),$(div 1498 2048),1";;
    3) texture="earth_equirectangular"; uv="equirect,$(div 2042 2048),$(div 1020 1024),2";;
esac

"$(dirname "$0")/demo" "icosphere:5,4,$uv;solid:texture($texture)"
