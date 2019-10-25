rgb_%.png: %.png
	convert $< -background white -alpha remove -flatten $@
gray_alpha_%.png: %.png
	convert $< -set colorspace Gray -separate -average $@
gray_%.png: %.png
	convert $< -background white -alpha remove -flatten -set colorspace Gray -separate -average $@
indexed24_alpha_%.png: %.png
	convert $< -colors 24 $@
indexed24_%.png: %.png
	convert $< -background white -alpha remove -flatten -colors 24 $@
tux.png:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/200px-Tux.svg.png"
grad.png:
	convert -size 256x256 gradient:white-black $@
