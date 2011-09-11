LEVELS:=0 25 50 75 100
READINGS:=$(patsubst %,src/readings/humid%.png,$(LEVELS)) $(patsubst %,src/readings/ilum%.png,$(LEVELS))

all: out/sprites.png out/favicon.ico

obj/16x16/%: %
	@mkdir -p $(dir $@) && convert $^ -resize 16x16 $@
obj/20x20/%: %
	@mkdir -p $(dir $@) && convert $^ -resize 20x20 $@
obj/32x32/%: %
	@mkdir -p $(dir $@) && convert $^ -resize 32x32 $@
obj/40x40/%: %
	@mkdir -p $(dir $@) && convert $^ -resize 40x40 $@
obj/48x48/%: %
	@mkdir -p $(dir $@) && convert $^ -resize 48x48 $@
obj/72x72/%: %
	@mkdir -p $(dir $@) && convert $^ -resize 72x72 $@

obj/big_icons.png: $(addprefix obj/48x48/,$(READINGS)) \
	obj/72x72/src/readings/status_box.png \
	obj/40x40/src/logo.png \
	obj/32x32/src/power_icon.png
	@mkdir -p $(dir $@) && montage -background Transparent -tile 1x -geometry 128x128\>+0+0 $^ $@

obj/small_icons.png: $(addprefix obj/20x20/,$(READINGS)) \
	$(addprefix obj/16x16/src/events/,info.png warning.png error.png)
	@mkdir -p $(dir $@) && montage -background Transparent -tile 1x -geometry 32x32\>+0+0 $^ $@

out/sprites.png: obj/big_icons.png obj/small_icons.png
	@mkdir -p $(dir $@) && convert $^ -background Transparent -append $@

out/favicon.ico: src/logo_alternate.png
	@mkdir -p $(dir $@) && convert $^ -resize 32x32 $@

clean:
	rm -rf ./obj ./out

.PHONY: clean
