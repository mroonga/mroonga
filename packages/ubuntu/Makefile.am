CODE_NAMES = trusty,xenial,bionic,cosmic
SOURCE = ../$(PACKAGE)-$(VERSION).tar.gz
SOURCE_55_BASE = $(PACKAGE)-5.5
SOURCE_55 = $(SOURCE_55_BASE)_$(VERSION).orig.tar.gz
SOURCE_56_BASE = $(PACKAGE)-5.6
SOURCE_56 = $(SOURCE_56_BASE)_$(VERSION).orig.tar.gz
SOURCE_57_BASE = $(PACKAGE)-5.7
SOURCE_57 = $(SOURCE_57_BASE)_$(VERSION).orig.tar.gz
SOURCE_MARIADB_10_0_BASE = $(PACKAGE)-mariadb-10.0
SOURCE_MARIADB_10_0 = $(SOURCE_MARIADB_10_0_BASE)_$(VERSION).orig.tar.gz
SOURCE_MARIADB_10_1_BASE = $(PACKAGE)-mariadb-10.1
SOURCE_MARIADB_10_1 = $(SOURCE_MARIADB_10_1_BASE)_$(VERSION).orig.tar.gz

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
	  --ppa '$(LAUNCHPAD_PPA)' \
	  --pgp-sign-key '$(LAUNCHPAD_UPLOADER_PGP_KEY)'

source: $(SOURCE_55)
source: $(SOURCE_56)
source: $(SOURCE_57)
source: $(SOURCE_MARIADB_10_0)
source: $(SOURCE_MARIADB_10_1)

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

$(SOURCE_57): $(SOURCE)
	tar xf $(SOURCE)
	mv $(PACKAGE)-$(VERSION) $(SOURCE_57_BASE)-$(VERSION)
	tar cfz $(SOURCE_57) $(SOURCE_57_BASE)-$(VERSION)
	rm -r $(SOURCE_57_BASE)-$(VERSION)

$(SOURCE_MARIADB_10_0): $(SOURCE)
	tar xf $(SOURCE)
	mv $(PACKAGE)-$(VERSION) $(SOURCE_MARIADB_10_0_BASE)-$(VERSION)
	tar cfz $(SOURCE_MARIADB_10_0) $(SOURCE_MARIADB_10_0_BASE)-$(VERSION)
	rm -r $(SOURCE_MARIADB_10_0_BASE)-$(VERSION)

$(SOURCE_MARIADB_10_1): $(SOURCE)
	tar xf $(SOURCE)
	mv $(PACKAGE)-$(VERSION) $(SOURCE_MARIADB_10_1_BASE)-$(VERSION)
	tar cfz $(SOURCE_MARIADB_10_1) $(SOURCE_MARIADB_10_1_BASE)-$(VERSION)
	rm -r $(SOURCE_MARIADB_10_1_BASE)-$(VERSION)
