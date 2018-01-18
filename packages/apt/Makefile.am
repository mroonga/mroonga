REPOSITORIES_PATH = repositories
DISTRIBUTIONS = debian
ARCHITECTURES = i386 amd64
CODE_NAMES = jessie stretch
MYSQL_VARIANTS =				\
	5.5					\
	mariadb-10.0				\
	mariadb-10.1

all:

release: download build sign-packages update-repository sign-repository upload

remove-existing-packages:
	for distribution in $(DISTRIBUTIONS); do		\
	  find $(REPOSITORIES_PATH)/$${distribution}/pool	\
	    -type f -delete;					\
	done

download:
	for distribution in $(DISTRIBUTIONS); do			\
	  rsync -avz --progress --delete				\
	    $(RSYNC_PATH)/$${distribution} $(REPOSITORIES_PATH)/;	\
	done

sign-packages:
	./sign-packages.sh '$(GPG_UID)' '$(REPOSITORIES_PATH)/' '$(CODE_NAMES)'

update-repository:
	./update-repository.sh '$(PACKAGE_NAME)' '$(REPOSITORIES_PATH)/' \
	  '$(ARCHITECTURES)' '$(CODE_NAMES)'

sign-repository:
	./sign-repository.sh '$(GPG_UID)' '$(REPOSITORIES_PATH)/' '$(CODE_NAMES)'

ensure-rsync-path:
	@if test -z "$(RSYNC_PATH)"; then				\
	  echo "--with-rsync-path configure option must be specified.";	\
	  false;							\
	fi

upload: ensure-rsync-path
	for distribution in $(DISTRIBUTIONS); do	\
	  (cd $(REPOSITORIES_PATH)/$${distribution};				\
	   rsync -avz --progress --delete		\
	   dists pool $(RSYNC_PATH)/$${distribution};	\
	  );						\
	done

build: build-package-deb

build-package-deb: source env.sh
	vagrant destroy --force
	for variant in $(MYSQL_VARIANTS); do				\
	  cp env.sh tmp/;						\
	  echo "MYSQL_VARIANT=$${variant}" >> tmp/env.sh;		\
	  for architecture in $(ARCHITECTURES); do			\
	    for code_name in $(CODE_NAMES); do				\
	      is_target=false;						\
	      case "$${variant}-$${code_name}" in			\
	        5.5-jessie)						\
	          is_target=true;					\
	          ;;							\
	        mariadb-10.0-jessie)					\
	          is_target=true;					\
	          ;;							\
	        mariadb-10.1-stretch)					\
	          is_target=true;					\
	          ;;							\
	      esac;							\
	      if [ "$${is_target}" = "false" ]; then			\
	        continue;						\
	      fi;							\
	      rm -rf tmp/debian;					\
	      cp -rp $(srcdir)/../debian-$${variant} tmp/debian;	\
	      id=debian-$${code_name}-$${architecture};			\
	      vagrant up $${id} || exit 1;				\
	      vagrant destroy --force $${id};				\
	    done;							\
	  done;								\
	done

source: tmp/$(PACKAGE)-$(VERSION).tar.gz

tmp/$(PACKAGE)-$(VERSION).tar.gz: $(abs_top_builddir)/$(PACKAGE)-$(VERSION).tar.gz
	mkdir -p tmp
	cp $(abs_top_builddir)/$(PACKAGE)-$(VERSION).tar.gz $@
