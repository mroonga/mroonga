AM_CPPFLAGS =					\
	$(MYSQL_INC)				\
	$(GROONGA_CFLAGS)			\
	$(MYSQL_VERSION_CFLAGS)			\
	-I$(top_srcdir)

noinst_LTLIBRARIES =				\
	libmroonga.la				\
	libmrn_need_mysql.la

libmroonga_la_SOURCES =				\
	mrn_path_mapper.cpp			\
	mrn_path_mapper.hpp

libmrn_need_mysql_la_SOURCES =			\
	mrn_index_table_name.cpp		\
	mrn_index_table_name.hpp
