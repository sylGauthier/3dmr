IMAGES := tux

.PHONY: test-assets images $(IMAGES) fonts
test-assets: images fonts

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

images: $(IMAGES) png/earth_miller.png png/earth_central_cylindric.png png/earth_mercator.png png/earth_equirectangular.png
tux: $(addprefix png/,$(addsuffix tux.png,rgb_ gray_alpha_ gray_ indexed24_alpha_ indexed24_))
png/tux.png:
	@mkdir -p $(dir $@)
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/200px-Tux.svg.png"

.INTERMEDIATE: png/earth_miller.jpg png/earth_central_cylindric.jpg png/earth_mercator.jpg png/earth_equirectangular.jpg
png/earth_miller.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/5/5f/Miller_projection_SW.jpg"
png/earth_miller.png: png/earth_miller.jpg
	convert $< -crop 2044x1498+7+8 -background black -extent 2048x2048 $@
png/earth_central_cylindric.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/1/19/Central_cylindric_projection_square.JPG"
png/earth_central_cylindric.png: png/earth_central_cylindric.jpg
	convert $< -crop 702x702+4+4 -background black -extent 1024x1024 $@
png/earth_mercator.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/f/f4/Mercator_projection_SW.jpg"
png/earth_mercator.png: png/earth_mercator.jpg
	convert $< -crop 2044x1730+7+9 -background black -extent 2048x2048 $@
png/earth_equirectangular.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/8/83/Equirectangular_projection_SW.jpg"
png/earth_equirectangular.png: png/earth_equirectangular.jpg
	convert $< -crop 2042x1020+8+8 -background black -extent 2048x1024 $@

fonts: font/FreeSans.ttf
font/FreeSans.ttf: freefont-20120503.zip
	unzip $<
	mkdir -p font
	touch $(basename $<)/*.ttf
	mv $(basename $<)/*.ttf font
	rm -r $(basename $<)

freefont-20120503.zip:
	wget -O $@ "https://ftp.gnu.org/gnu/freefont/freefont-ttf-20120503.zip"

.PHONY: clean-assets
clean-assets:
	rm -f $(wildcard *.zip png/*.png font/*.ttf)
