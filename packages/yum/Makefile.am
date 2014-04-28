REPOSITORIES_PATH = repositories
DISTRIBUTIONS = centos
ARCHITECTURES = i386 x86_64
CHROOT_BASE = /var/lib/chroot
HAVE_DEVELOPMENT_BRANCH = no
USE_RPMFORGE = no
USE_ATRPMS = no
PACKAGES = mysql-mroonga mariadb-mroonga mysql55-mroonga

all:

release: build sign-packages update-repository upload

remove-existing-packages:
	for distribution in $(DISTRIBUTIONS); do	\
	  find $${distribution} -name "*.rpm" -delete;	\
	done

ensure-rsync-path:
	@if test -z "$(RSYNC_PATH)"; then				\
	  echo "--with-rsync-path configure option must be specified.";	\
	  false;							\
	fi

download: ensure-rsync-path
	mkdir -p $(REPOSITORIES_PATH)
	for distribution in $(DISTRIBUTIONS); do			\
	  rsync -avz --progress --delete				\
	    $(RSYNC_PATH)/$${distribution} $(REPOSITORIES_PATH)/;	\
	done

sign-packages:
	./sign-rpm.sh '$(GPG_UID)' '$(REPOSITORIES_PATH)/' '$(DISTRIBUTIONS)'

update-repository:
	./update-repository.sh '$(REPOSITORIES_PATH)/' '$(DISTRIBUTIONS)'

upload: ensure-rsync-path
	for distribution in $(DISTRIBUTIONS); do		\
	  rsync -avz --progress --exclude .gitignore --delete	\
	    $(REPOSITORIES_PATH)/$${distribution}/		\
	    $(RSYNC_PATH)/$${distribution};			\
	done

build: build-in-chroot

build-in-chroot: source
	for package in $(PACKAGES); do		\
	  ./build-in-chroot.sh			\
	    $${package}				\
	    $(VERSION)				\
	    $(top_builddir)/$(PACKAGE)		\
	    ../rpm				\
	    $(REPOSITORIES_PATH)/		\
	    $(CHROOT_BASE)			\
	    '$(ARCHITECTURES)'			\
	    '$(DISTRIBUTIONS)'			\
	    '$(HAVE_DEVELOPMENT_BRANCH)'	\
	    '$(USE_RPMFORGE)'			\
	    '$(USE_ATRPMS)';			\
	done

source: ../$(PACKAGE)-$(VERSION).tar.gz

../$(PACKAGE)-$(VERSION).tar.gz:
	ln -s $(abs_top_builddir)/$(PACKAGE)-$(VERSION).tar.gz ../
