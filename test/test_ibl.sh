#!/bin/bash

"$(dirname "$0")/demo" \
    -La 1,1,1 -Ld "1,0,-1;1,1,1" \
    "skybox:100,devils-tooth_bk,devils-tooth_ft,devils-tooth_dn,devils-tooth_up,devils-tooth_lf,devils-tooth_rt" \
    "icosphere:5;pbr:color(red),0.8,0.2"
