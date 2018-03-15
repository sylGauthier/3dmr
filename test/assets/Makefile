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

images: $(IMAGES)
tux: $(addprefix png/,$(addsuffix tux.png,rgb_ gray_alpha_ gray_ indexed24_alpha_ indexed24_))
png/tux.png:
	@mkdir -p $(dir $@)
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/200px-Tux.svg.png"

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
