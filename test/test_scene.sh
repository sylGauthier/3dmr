#!/bin/bash

"$(dirname "$0")/demo" \
    -La 1,1,1 -Lp "0,0,0;1,1,1;5" \
    "grid:10,10,2,cube:1;phong:texture(checkerboard),phong(default);r(1,0,0,1.57)" \
    "grid:11,11,2,icosphere:0.5;phong:color(green),phong(default);r(1,0,0,1.57)"
