lib_LTLIBRARIES = libmroonga.la

AM_CPPFLAGS = $(MYSQL_INC) $(GROONGA_INCLUDES) $(MECAB_INCLUDES) -save-temps
AM_INCLUDES = -l. -l..

libmroonga_la_SOURCES = ha_groonga.cc
libmroonga_la_LDFLAGS = -module  $(GROONGA_LIBS) $(MECAB_LIBS)
libmroonga_la_CXXFLAGS = -DMYSQL_DYNAMIC_PLUGIN
noinst_HEADERS = mroonga.h ha_groonga.h
