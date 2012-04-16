AM_CPPFLAGS =					\
	$(MYSQL_INC)				\
	$(GROONGA_CFLAGS)			\
	$(MYSQL_VERSION_CFLAGS)			\
	-I$(top_srcdir)

noinst_LTLIBRARIES =				\
	libmrn_no_mysql.la			\
	libmrn_need_mysql.la

include libmrn_no_mysql_sources.am
include libmrn_need_mysql_sources.am
