/* -*- c-basic-offset: 2 -*- */
/*
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

#ifndef _mrn_table_h
#define _mrn_table_h

#if MYSQL_VERSION_ID >= 50500
#define my_free(A,B) my_free(A)
#endif

typedef struct st_mroonga_share
{
  char               *table_name;
  uint               table_name_length;
  uint               use_count;
  pthread_mutex_t    mutex;
  THR_LOCK           lock;
  TABLE_SHARE        *table_share;
  TABLE_SHARE        *wrap_table_share;

  char               *engine;
  int                engine_length;
  plugin_ref         plugin;
  handlerton         *hton;
  char               **key_parser;
  uint               *key_parser_length;
  uint               *wrap_key_nr;
  uint               wrap_keys;
  uint               base_keys;
  KEY                *wrap_key_info;
  KEY                *base_key_info;
  uint               wrap_primary_key;
  uint               base_primary_key;
  bool               wrapper_mode;
} MRN_SHARE;

#define MRN_SET_WRAP_SHARE_KEY(share, table_share)
/*
  table_share->keys = share->wrap_keys; \
  table_share->key_info = share->wrap_key_info; \
  table_share->primary_key = share->wrap_primary_key;
*/

#define MRN_SET_BASE_SHARE_KEY(share, table_share)
/*
  table_share->keys = share->base_keys; \
  table_share->key_info = share->base_key_info; \
  table_share->primary_key = share->base_primary_key;
*/

#define MRN_SET_WRAP_TABLE_KEY(file, table) \
  table->key_info = file->wrap_key_info; \
  table->s = share->wrap_table_share;

#define MRN_SET_BASE_TABLE_KEY(file, table) \
  table->key_info = file->base_key_info; \
  table->s = share->table_share;

char *mrn_create_string(const char *str, uint length);
char *mrn_get_string_between_quote(char *ptr, bool alloc);
#ifdef WITH_PARTITION_STORAGE_ENGINE
void mrn_get_partition_info(const char *table_name, uint table_name_length,
                            const TABLE *table, partition_element **part_elem,
                            partition_element **sub_elem);
#endif
int mrn_parse_table_param(MRN_SHARE *share, TABLE *table);
int mrn_add_index_param(MRN_SHARE *share, KEY *key_info, int i);
int mrn_parse_index_param(MRN_SHARE *share, TABLE *table);
MRN_SHARE *mrn_get_share(const char *table_name, TABLE *table, int *error);
int mrn_free_share_alloc(MRN_SHARE *share);
int mrn_free_share(MRN_SHARE *share);
TABLE_SHARE *mrn_get_table_share(TABLE_LIST *table_list, int *error);
void mrn_free_table_share(TABLE_SHARE *share);
KEY *mrn_create_key_info_for_table(MRN_SHARE *share, TABLE *table, int *error);
void mrn_set_bitmap_by_key(MY_BITMAP *map, KEY *key_info);
uint mrn_decode(uchar *buf_st, uchar *buf_ed,
                const uchar *st, const uchar *ed);

#endif /* _mrn_table_h */
