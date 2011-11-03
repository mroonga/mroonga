generated_images =		\
	mroonga.png		\
	storage-mode.png	\
	wrapper-mode.png

.SUFFIXES: .svg .png
.svg.png:
	inkscape --export-dpi 90 --export-background white --export-png $@ $<

update-images: $(generated_images)
