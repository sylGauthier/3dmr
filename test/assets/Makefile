.PHONY: test-assets test-assets-extra clean-assets

.DELETE_ON_ERROR:
DIRS := downloads meshes textures fonts
_ := $(shell mkdir -p $(DIRS) >/dev/null 2>/dev/null)

test-assets:


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
textures/%.png: downloads/%.jpg
	convert $< $@
textures/%.png: downloads/%.tga
	convert $< $@


ASSETS += $(addprefix textures/,$(addsuffix tux.png,rgb_ gray_alpha_ gray_ indexed24_alpha_ indexed24_))
textures/tux.png:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/200px-Tux.svg.png"


ASSETS += textures/earth_miller.png textures/earth_central_cylindric.png textures/earth_mercator.png textures/earth_equirectangular.png
downloads/earth_miller.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/5/5f/Miller_projection_SW.jpg"
textures/earth_miller.png: downloads/earth_miller.jpg
	convert $< -crop 2044x1498+7+8 -background black -extent 2048x2048 $@
downloads/earth_central_cylindric.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/1/19/Central_cylindric_projection_square.JPG"
textures/earth_central_cylindric.png: downloads/earth_central_cylindric.jpg
	convert $< -crop 702x702+4+4 -background black -extent 1024x1024 $@
downloads/earth_mercator.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/f/f4/Mercator_projection_SW.jpg"
textures/earth_mercator.png: downloads/earth_mercator.jpg
	convert $< -crop 2044x1730+7+9 -background black -extent 2048x2048 $@
downloads/earth_equirectangular.jpg:
	wget -O $@ "https://upload.wikimedia.org/wikipedia/commons/8/83/Equirectangular_projection_SW.jpg"
textures/earth_equirectangular.png: downloads/earth_equirectangular.jpg
	convert $< -crop 2042x1020+8+8 -background black -extent 2048x1024 $@


ASSETS += textures/brickwall.png textures/brickwall_normal.png
downloads/brickwall.jpg downloads/brickwall_normal.jpg:
	wget -O $@ https://learnopengl.com/img/textures/$(notdir $@)


ASSETS_EXTRA += $(foreach t,albedo ao metalness normal roughness, textures/chest_$(t).png) meshes/chest.obj
textures/chest_%.png:
	wget -O $@ https://github.com/emackey/testing-pbr/raw/6afba39ddf5631b6cbf08dd031b0edd541b1e7eb/models/Chest/$(notdir $@)
meshes/chest.obj:
	wget -O $@ https://github.com/emackey/testing-pbr/raw/6afba39ddf5631b6cbf08dd031b0edd541b1e7eb/models/Chest/Chest.obj

mats := $(foreach mat,gold plastic,$(foreach t,albedo ao metalness normal roughness,textures/$(mat)_$(t).png))
ASSETS_EXTRA += $(mats)
$(mats):
	wget -O $@ https://github.com/JoeyDeVries/LearnOpenGL/raw/239c456ae9416fdfd294a58e0ff127897810ab60/resources/textures/pbr/$(shell echo $(notdir $@) | sed 's,\(.*\)_\(.*\),\1/\2,;s,metalness,metallic,')


ASSETS_EXTRA += $(foreach t,albedo metalness normal roughness, textures/cerberus_$(t).png) meshes/cerberus.obj
downloads/Cerberus_by_Andrew_Maximov.zip:
	wget -O $@ http://artisaverb.info/Cerberus/$(notdir $@)
textures/cerberus_albedo.png: downloads/Cerberus_A.tga
textures/cerberus_normal.png: downloads/Cerberus_N.tga
textures/cerberus_metalness.png: downloads/Cerberus_M.tga
textures/cerberus_roughness.png: downloads/Cerberus_R.tga
textures/cerberus_%.png:
	convert $< $@
downloads/Cerberus_%.tga: downloads/Cerberus_by_Andrew_Maximov.zip
	unzip -p $< '*/Textures/$(notdir $@)' > $@
downloads/Cerberus_LP.FBX: downloads/Cerberus_by_Andrew_Maximov.zip
	unzip -p $< '*/$(notdir $@)' > $@
meshes/cerberus.obj: downloads/Cerberus_LP.FBX
	 blender --background --python-expr 'import bpy; bpy.ops.import_scene.fbx(filepath="$<"); bpy.ops.export_scene.obj(filepath="$@")'
	 sed -i '/^mtllib/d;/^usemtl/d' $@
	 rm $(patsubst %.obj,%.mtl,$@)
.INTERMEDIATE: $(foreach t,A N M R,downloads/Cerberus_$(t).tga ) downloads/Cerberus_LP.FBX


ASSETS += $(foreach t,bk dn ft lf rt up,textures/devils-tooth_$(t).png)
downloads/mp_deviltooth.zip:
	wget -O $@ http://www.custommapmakers.org/skyboxes/zips/$(notdir $@)
downloads/devils-tooth_%.tga: downloads/mp_deviltooth.zip
	unzip -p $< mp_deviltooth/$(notdir $@) > $@


ASSETS_EXTRA += textures/tropical_beach.hdr
textures/tropical_beach.hdr: downloads/Tropical_Beach.zip
	unzip -p $< Tropical_Beach/Tropical_Beach_3k.hdr > $@
downloads/Tropical_Beach.zip:
	wget -O $@ http://www.hdrlabs.com/sibl/archive/downloads/$(notdir $@)


ASSETS += fonts/FreeSans.ttf
downloads/freefont-20120503.zip:
	wget -O $@ "https://ftp.gnu.org/gnu/freefont/freefont-ttf-20120503.zip"
fonts/FreeSans.ttf: downloads/freefont-20120503.zip
	unzip -j $< '*/$(notdir $@)' -d $(dir $@)
	touch $@


test-assets: $(ASSETS)
test-assets-extra: $(ASSETS_EXTRA)
clean-assets:
	rm -f $(wildcard $(foreach d,$(DIRS),$(d)/*))
