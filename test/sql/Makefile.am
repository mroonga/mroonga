SUBDIRS =					\
	include					\
	suite/mroonga_storage			\
	suite/mroonga_wrapper

update-files:
	cd $(srcdir)/suite/mroonga_storage/t; ../../../update-test-files.sh > files.am
	cd $(srcdir)/suite/mroonga_wrapper/t; ../../../update-test-files.sh > files.am
	cd $(srcdir)/suite/mroonga_storage/r; ../../../update-result-files.sh > files.am
	cd $(srcdir)/suite/mroonga_wrapper/r; ../../../update-result-files.sh > files.am
