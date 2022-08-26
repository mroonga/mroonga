SUBDIRS =					\
	deb					\
	rpm

sqldir = $(pkgdatadir)
dist_sql_DATA =					\
	install_plugin.sql			\
	uninstall.sql				\
	update.sql
EXTRA_DIST =					\
	install.sql.in
sql_DATA =					\
	install.sql

install.sql: install.sql.in
	sed -e 's|@MRN_FULL_DATA_DIR[@]|$(sqldir)|g' $< > $@
