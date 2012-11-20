/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2011 Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _mrn_mysql_compat_h
#define _mrn_mysql_compat_h

#if MYSQL_VERSION_ID >= 50500
#  define my_free(PTR, FLAG) my_free(PTR)
#endif

#if MYSQL_VERSION_ID < 50500
#  define mysql_mutex_lock(mutex) pthread_mutex_lock(mutex)
#  define mysql_mutex_unlock(mutex) pthread_mutex_unlock(mutex)
#endif

#if MYSQL_VERSION_ID >= 50604
#  define MRN_HAVE_MYSQL_TYPE_TIMESTAMP2
#  define MRN_HAVE_MYSQL_TYPE_DATETIME2
#  define MRN_HAVE_MYSQL_TYPE_TIME2
#endif

#if MYSQL_VERSION_ID < 50603
  typedef MYSQL_ERROR Sql_condition;
#endif

#if defined(MRN_MARIADB_P)
#  if MYSQL_VERSION_ID >= 50302 && MYSQL_VERSION_ID < 100000
     typedef COST_VECT Cost_estimate;
#  endif
#endif

#if MYSQL_VERSION_ID >= 50516
#  define MRN_PLUGIN_HAVE_FLAGS 1
#endif

// for MySQL < 5.5
#ifndef MY_ALL_CHARSETS_SIZE
#  define MY_ALL_CHARSETS_SIZE 256
#endif

#ifndef MRN_MARIADB_P
  typedef char *range_id_t;
#endif

#endif /* _mrn_mysql_compat_h */
