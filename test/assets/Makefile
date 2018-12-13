.PHONY: test-assets clean-assets images fonts
test-assets: images fonts

.INTERMEDIATE: textures/%.jpg
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
%.png: %.jpg
	convert $< $@

IMAGES += $(addprefix textures/,$(addsuffix tux.png,rgb_ gray_alpha_ gray_ indexed24_alpha_ indexed24_))
textures/tux.png:
	@mkdir -p $(dir $@)
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/200px-Tux.svg.png"

IMAGES += textures/earth_miller.png textures/earth_central_cylindric.png textures/earth_mercator.png textures/earth_equirectangular.png
textures/earth_miller.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/5/5f/Miller_projection_SW.jpg"
textures/earth_miller.png: textures/earth_miller.jpg
	convert $< -crop 2044x1498+7+8 -background black -extent 2048x2048 $@
textures/earth_central_cylindric.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/1/19/Central_cylindric_projection_square.JPG"
textures/earth_central_cylindric.png: textures/earth_central_cylindric.jpg
	convert $< -crop 702x702+4+4 -background black -extent 1024x1024 $@
textures/earth_mercator.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/f/f4/Mercator_projection_SW.jpg"
textures/earth_mercator.png: textures/earth_mercator.jpg
	convert $< -crop 2044x1730+7+9 -background black -extent 2048x2048 $@
textures/earth_equirectangular.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/8/83/Equirectangular_projection_SW.jpg"
textures/earth_equirectangular.png: textures/earth_equirectangular.jpg
	convert $< -crop 2042x1020+8+8 -background black -extent 2048x1024 $@

IMAGES += textures/brickwall.png textures/brickwall_normal.png
textures/brickwall.jpg textures/brickwall_normal.jpg:
	wget -O $@ https://learnopengl.com/img/$@

FONTS += font/FreeSans.ttf
font/FreeSans.ttf: freefont-20120503.zip
	unzip $<
	mkdir -p font
	touch $(basename $<)/*.ttf
	mv $(basename $<)/*.ttf font
	rm -r $(basename $<)

freefont-20120503.zip:
	wget -O $@ "https://ftp.gnu.org/gnu/freefont/freefont-ttf-20120503.zip"

images: $(IMAGES)
fonts: $(FONTS)
clean-assets:
	rm -f $(wildcard *.zip textures/*.png font/*.ttf)
