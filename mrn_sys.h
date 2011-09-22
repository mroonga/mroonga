/*
  Copyright(C) 2010 Tetsuro IKEDA
  Copyright(C) 2011 Kentoku SHIBA
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

#ifndef _mrn_sys_h
#define _mrn_sys_h

#include <groonga.h>

/* constants */
#define MRN_PACKAGE_STRING "groonga-storage-engine"
#define MRN_MAX_KEY_SIZE 1024
#define MRN_MAX_PATH_SIZE 256
#define MRN_MAX_EXPRS 32
#define MRN_DB_FILE_SUFFIX ".mrn"
#define MRN_LOG_FILE_NAME "groonga.log"
#define MRN_LEX_SUFFIX "_lex"
#define MRN_HASH_SUFFIX "_hash"
#define MRN_PAT_SUFFIX "_pat"
#define MRN_ID_COL_NAME "_id"
#define MRN_SCORE_COL_NAME "_score"

/* functions */
int mrn_hash_put(grn_ctx *ctx, grn_hash *hash, const char *key, void *value);
int mrn_hash_get(grn_ctx *ctx, grn_hash *hash, const char *key, void **value);
int mrn_hash_remove(grn_ctx *ctx, grn_hash *hash, const char *key);

char *mrn_db_path_gen(const char *arg, char *dest);
char *mrn_db_name_gen(const char *arg, char *dest);
char *mrn_table_name_gen(const char *arg, char *dest);
char *mrn_index_table_name_gen(const char *arg, const char *idx_name, char *dest);

#endif /* _mrn_sys_h */
