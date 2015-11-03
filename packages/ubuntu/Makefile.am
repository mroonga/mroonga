CODE_NAMES = precise,trusty,vivid,wily
SOURCE = ../$(PACKAGE)-$(VERSION).tar.gz
SOURCE_55_BASE = $(PACKAGE)-5.5
SOURCE_55 = $(SOURCE_55_BASE)_$(VERSION).orig.tar.gz
SOURCE_56_BASE = $(PACKAGE)-5.6
SOURCE_56 = $(SOURCE_56_BASE)_$(VERSION).orig.tar.gz

all:

ensure-launchpad-configuration:
	@if test -z "$(LAUNCHPAD_UPLOADER_PGP_KEY)"; then		\
	  echo "--with-launchpad-uploader-pgp-key configure option must be specified.";	\
	  false;							\
	fi

upload: source ensure-launchpad-configuration
	./upload.rb \
	  --package '$(PACKAGE)' \
	  --version '$(VERSION)' \
	  --source-archive-directory '$(builddir)/' \
	  --code-names '$(CODE_NAMES)' \
	  --debian-base-directory '$(srcdir)/../' \
	  --pgp-sign-key '$(LAUNCHPAD_UPLOADER_PGP_KEY)'

source: $(SOURCE) $(SOURCE_55) $(SOURCE_56)

$(SOURCE):
	ln -s $(abs_top_builddir)/$(PACKAGE)-$(VERSION).tar.gz $(SOURCE)

$(SOURCE_55): $(SOURCE)
	tar xf $(SOURCE)
	mv $(PACKAGE)-$(VERSION) $(SOURCE_55_BASE)-$(VERSION)
	tar cfz $(SOURCE_55) $(SOURCE_55_BASE)-$(VERSION)
	rm -r $(SOURCE_55_BASE)-$(VERSION)

$(SOURCE_56): $(SOURCE)
	tar xf $(SOURCE)
	mv $(PACKAGE)-$(VERSION) $(SOURCE_56_BASE)-$(VERSION)
	tar cfz $(SOURCE_56) $(SOURCE_56_BASE)-$(VERSION)
	rm -r $(SOURCE_56_BASE)-$(VERSION)
