#ifdef ALPHA_ENABLED

#ifdef ALPHA_TEXTURED
uniform sampler2D alpha;
#else
uniform float alpha;
#endif

float get_alpha() {
#ifdef ALPHA_TEXTURED
    float a = texture(alpha, coordTexture).r;
#else
    float a = alpha;
#endif
#ifdef ALPHA_TEST
    a = round(a);
#endif
    return a;
}

#else

float get_alpha() {
    return 1.0;
}

#endif
