AM_CPPFLAGS =					\
	$(MYSQL_INCLUDES)			\
	$(GROONGA_CFLAGS)			\
	$(MYSQL_VERSION_CFLAGS)			\
	-I$(top_srcdir)

libmrn_need_mysql_la_LDFLAGS  = -module $(GROONGA_LIBS)
libmrn_need_mysql_la_CXXFLAGS = $(AM_CXXFLAGS) $(MYSQL_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN
libmrn_need_mysql_la_CFLAGS   = $(AM_CFLAGS) $(MYSQL_CFLAGS) -DMYSQL_DYNAMIC_PLUGIN

noinst_LTLIBRARIES =				\
	libmrn_no_mysql.la			\
	libmrn_need_mysql.la

include libmrn_no_mysql_sources.am
include libmrn_need_mysql_sources.am
