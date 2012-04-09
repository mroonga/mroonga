/*
  Copyright(C) 2010 Tetsuro IKEDA
  Copyright(C) 2011 Kentoku SHIBA
  Copyright(C) 2011-2012 Kouhei Sutou <kou@clear-code.com>

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

#ifndef _mrn_sys_h
#define _mrn_sys_h

#include <groonga.h>
#include "mrn_macro.h"

MRN_BEGIN_DECLS

/* constants */
#define MRN_BUFFER_SIZE 1024
#define MRN_MAX_KEY_SIZE GRN_TABLE_MAX_KEY_SIZE
#if defined(MAX_PATH)
#  define MRN_MAX_PATH_SIZE (MAX_PATH + 1)
#elif defined(PATH_MAX)
#  define MRN_MAX_PATH_SIZE (PATH_MAX)
#elif defined(MAXPATHLEN)
#  define MRN_MAX_PATH_SIZE (MAXPATHLEN)
#else
#  define MRN_MAX_PATH_SIZE (256)
#endif
#define MRN_DB_FILE_SUFFIX ".mrn"
#define MRN_LOG_FILE_PATH "groonga.log"
#define MRN_COLUMN_NAME_ID "_id"
#define MRN_COLUMN_NAME_KEY "_key"
#define MRN_COLUMN_NAME_SCORE "_score"
#ifndef MRN_PARSER_DEFAULT
#  define MRN_PARSER_DEFAULT "TokenBigram"
#endif

/* functions */
bool mrn_hash_put(grn_ctx *ctx, grn_hash *hash, const char *key, grn_obj *value);
bool mrn_hash_get(grn_ctx *ctx, grn_hash *hash, const char *key, grn_obj **value);
bool mrn_hash_remove(grn_ctx *ctx, grn_hash *hash, const char *key);

char *mrn_index_table_name_gen(const char *arg, const char *idx_name, char *dest);

MRN_END_DECLS

#endif /* _mrn_sys_h */
