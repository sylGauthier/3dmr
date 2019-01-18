#define GAMMA 2.2
#define EXPOSURE 1.0
#define PURE_WHITE 1.0

vec3 reinhard_tonemapping(vec3 color, float gamma, float exposure, float pureWhite) {
    color *= exposure;
	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

	// Scale color by ratio of average luminances
	vec3 mappedColor = (mappedLuminance / luminance) * color;

	// Gamma correction.
	return pow(mappedColor, vec3(1.0 / gamma));
}
