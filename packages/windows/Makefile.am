EXTRA_DIST =					\
	README.md				\
	build-vc2010.bat			\
	build-vc2010-zip-32.bat			\
	build-vc2010-zip-64.bat			\
	build-vc2010-msi-32.bat			\
	build-vc2010-msi-64.bat			\
	build-vc2013.bat			\
	build-vc2013-zip-32.bat			\
	build-vc2013-zip-64.bat			\
	build-vc2013-msi-32.bat			\
	build-vc2013-msi-64.bat

all:

release: upload

ensure-rsync-path:
	@if test -z "$(RSYNC_PATH)"; then				\
	  echo "--with-rsync-path configure option must be specified.";	\
	  false;							\
	fi

download: ensure-rsync-path
	rsync -avz --progress $(RSYNC_PATH)/windows/mroonga/ files

upload: ensure-rsync-path
	rsync -avz --progress --delete files/ $(RSYNC_PATH)/windows/mroonga
