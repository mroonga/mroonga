SUBDIRS =					\
	include					\
	suite/groonga_storage			\
	suite/groonga_wrapper

update-files:
	cd $(srcdir)/suite/groonga_storage/t; ../../update-test-files.sh > files.am
	cd $(srcdir)/suite/groonga_wrapper/t; ../../update-test-files.sh > files.am
	cd $(srcdir)/suite/groonga_storage/r; ../../update-result-files.sh > files.am
	cd $(srcdir)/suite/groonga_wrapper/r; ../../update-result-files.sh > files.am
