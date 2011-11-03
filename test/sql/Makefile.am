SUBDIRS =					\
	groonga_include				\
	groonga_storage				\
	groonga_wrapper

update-files:
	cd $(srcdir)/groonga_storage/t; ../../update-test-files.sh > files.am
	cd $(srcdir)/groonga_wrapper/t; ../../update-test-files.sh > files.am
	cd $(srcdir)/groonga_storage/r; ../../update-result-files.sh > files.am
	cd $(srcdir)/groonga_wrapper/r; ../../update-result-files.sh > files.am
