SUBDIRS =					\
	deb					\
	rpm

EXTRA_DIST =					\
	$(sql_SOURCES)

sqldir = $(pkgdatadir)
dist_sql_DATA =					\
	install_plugin.sql				\
	uninstall.sql				\
	update.sql
sql_SOURCES =					\
	install.sql.in
sql_DATA =					\
	$(sql_SOURCES:.in=)

install.sql: install.sql.in
	sed -e 's|@MRN_DATA_DIR[@]|$(sqldir)|g' $< > $@
