EXTRA_DIST =					\
	build.bat				\
	build-zip-32.bat			\
	build-zip-64.bat			\
	build-msi-32.bat			\
	build-msi-64.bat

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
