/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2011-2013  Kentoku SHIBA
  Copyright(C) 2011-2023  Sutou Kouhei <kou@clear-code.com>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <list>

#include "mrn_mysql_compat.h"

#include <groonga.h>

typedef struct st_mroonga_long_term_share
{
  char                *table_name;
  uint                table_name_length;

  // for auto_increment (storage mode only)
  mysql_mutex_t       auto_inc_mutex;
  bool                auto_inc_inited;
  ulonglong           auto_inc_value;
} MRN_LONG_TERM_SHARE;

typedef struct st_mroonga_share
{
  char                *table_name;
  uint                table_name_length;
  uint                use_count;
  mysql_mutex_t       record_mutex;
  THR_LOCK            lock;
  TABLE_SHARE         *table_share;
#ifdef MRN_ENABLE_WRAPPER_MODE
  TABLE_SHARE         *wrap_table_share;
#endif
  MRN_LONG_TERM_SHARE *long_term_share;

  char                *table_flags;
  int                 table_flags_length;
#ifdef MRN_ENABLE_WRAPPER_MODE
  char                *engine;
  int                 engine_length;
#endif
  char                *tokenizer;
  int                 tokenizer_length;
  char                *default_tokenizer;
  int                 default_tokenizer_length;
  char                *normalizer;
  int                 normalizer_length;
  char                *token_filters;
  int                 token_filters_length;
#ifdef MRN_ENABLE_WRAPPER_MODE
  plugin_ref          plugin;
  handlerton          *hton;
#endif
  char                **col_flags;
  char                **col_type;
  uint                *col_flags_length;
  uint                *col_type_length;
#ifdef MRN_ENABLE_WRAPPER_MODE
  uint                *wrap_key_nr;
  uint                wrap_keys;
  uint                base_keys;
  KEY                 *wrap_key_info;
  KEY                 *base_key_info;
  uint                wrap_primary_key;
  uint                base_primary_key;
  bool                wrapper_mode;
#endif
  bool                disable_keys;
} MRN_SHARE;

#ifdef MRN_ENABLE_WRAPPER_MODE
struct st_mrn_wrap_hton
{
  char path[FN_REFLEN + 1];
  handlerton *hton;
  st_mrn_wrap_hton *next;
};
#endif

namespace mrn {
  struct SlotData {
    SlotData()
      : last_insert_record_id(GRN_ID_NIL),
#ifdef MRN_ENABLE_WRAPPER_MODE
        first_wrap_hton(NULL),
#endif
        alter_create_info(NULL),
        disable_keys_create_info(NULL),
        alter_connect_string(NULL),
        alter_comment(NULL),
        associated_grn_ctxs() {
    }

    ~SlotData() {
      clear();
    }

    void clear() {
#ifdef MRN_ENABLE_WRAPPER_MODE
      if (first_wrap_hton) {
        st_mrn_wrap_hton *wrap_hton = first_wrap_hton;
        while (wrap_hton) {
          st_mrn_wrap_hton *tmp_wrap_hton = wrap_hton->next;
          free(wrap_hton);
          wrap_hton = tmp_wrap_hton;
        }
        first_wrap_hton = NULL;
      }
#endif
      alter_create_info = NULL;
      disable_keys_create_info = NULL;
      if (alter_connect_string) {
        my_free(alter_connect_string);
        alter_connect_string = NULL;
      }
      if (alter_comment) {
        my_free(alter_comment);
        alter_comment = NULL;
      }
    }

    void add_associated_grn_ctx(grn_ctx *ctx) {
      associated_grn_ctxs.push_back(ctx);
    }

    void remove_associated_grn_ctx(grn_ctx *ctx) {
      associated_grn_ctxs.remove(ctx);
    }

    grn_id last_insert_record_id;
#ifdef MRN_ENABLE_WRAPPER_MODE
    st_mrn_wrap_hton *first_wrap_hton;
#endif
    HA_CREATE_INFO *alter_create_info;
    HA_CREATE_INFO *disable_keys_create_info;
    char *alter_connect_string;
    char *alter_comment;
    std::list<grn_ctx *> associated_grn_ctxs;
  };
};

#define MRN_SET_WRAP_ALTER_KEY(file, ha_alter_info) \
  mrn_alter_flags base_handler_flags = ha_alter_info->handler_flags; \
  KEY  *base_key_info_buffer = ha_alter_info->key_info_buffer; \
  uint base_key_count = ha_alter_info->key_count; \
  uint base_index_drop_count = ha_alter_info->index_drop_count; \
  KEY  **base_index_drop_buffer = ha_alter_info->index_drop_buffer; \
  uint base_index_add_count = ha_alter_info->index_add_count; \
  uint *base_index_add_buffer = ha_alter_info->index_add_buffer; \
  ha_alter_info->handler_flags = file->alter_handler_flags; \
  ha_alter_info->key_info_buffer = file->alter_key_info_buffer; \
  ha_alter_info->key_count = file->alter_key_count; \
  ha_alter_info->index_drop_count = file->alter_index_drop_count; \
  ha_alter_info->index_drop_buffer = &file->alter_index_drop_buffer; \
  ha_alter_info->index_add_count = file->alter_index_add_count; \
  ha_alter_info->index_add_buffer = file->alter_index_add_buffer;

#define MRN_SET_BASE_ALTER_KEY(share, table_share) \
  ha_alter_info->handler_flags = base_handler_flags; \
  ha_alter_info->key_info_buffer = base_key_info_buffer; \
  ha_alter_info->key_count = base_key_count; \
  ha_alter_info->index_drop_count = base_index_drop_count; \
  ha_alter_info->index_drop_buffer = base_index_drop_buffer; \
  ha_alter_info->index_add_count = base_index_add_count; \
  ha_alter_info->index_add_buffer = base_index_add_buffer;

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

#ifdef WITH_PARTITION_STORAGE_ENGINE
void mrn_get_partition_info(const char *table_name, uint table_name_length,
                            const TABLE *table, partition_element **part_elem,
                            partition_element **sub_elem);
#endif
int mrn_parse_table_param(MRN_SHARE *share, TABLE *table);
bool mrn_is_geo_key(const KEY *key_info);
int mrn_add_column_param(MRN_SHARE *share, Field *field, int i);
int mrn_parse_column_param(MRN_SHARE *share, TABLE *table);
MRN_SHARE *mrn_get_share(const char *table_name, TABLE *table, int *error);
int mrn_free_share_alloc(MRN_SHARE *share);
int mrn_free_share(MRN_SHARE *share);
MRN_LONG_TERM_SHARE *mrn_get_long_term_share(const char *table_name,
                                             uint table_name_length,
                                             int *error);
void mrn_free_long_term_share(MRN_LONG_TERM_SHARE *long_term_share);
TABLE_SHARE *mrn_get_table_share(TABLE_LIST *table_list, int *error);
TABLE_SHARE *mrn_create_tmp_table_share(TABLE_LIST *table_list,
                                        const char *path,
#ifdef MRN_OPEN_TABLE_DEF_USE_TABLE_DEFINITION
                                        const dd::Table *table_def,
#endif
                                        int *error);
void mrn_free_tmp_table_share(TABLE_SHARE *table_share);
#ifdef MRN_ENABLE_WRAPPER_MODE
KEY *mrn_create_key_info_for_table(MRN_SHARE *share, TABLE *table, int *error);
#endif
void mrn_set_bitmap_by_key(MY_BITMAP *map, KEY *key_info);
mrn::SlotData *mrn_get_slot_data(THD *thd, bool can_create);
