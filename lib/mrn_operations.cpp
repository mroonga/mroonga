/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2015 Kouhei Sutou <kou@clear-code.com>

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

#include "mrn_operations.hpp"

// for debug
#define MRN_CLASS_NAME "mrn::Operations"

#define TABLE_NAME "mrn_operations"
#define COLUMN_TYPE_NAME   "type"
#define COLUMN_TABLE_NAME  "table"
#define COLUMN_RECORD_NAME "record"

namespace mrn {
  Operations::Operations(grn_ctx *ctx)
    : ctx_(ctx) {
    MRN_DBUG_ENTER_METHOD();

    GRN_TEXT_INIT(&text_buffer_, GRN_OBJ_DO_SHALLOW_COPY);
    GRN_UINT32_INIT(&id_buffer_, 0);

    table_ = grn_ctx_get(ctx_, TABLE_NAME, -1);
    if (!table_) {
      table_ = grn_table_create(ctx_,
                                TABLE_NAME, strlen(TABLE_NAME),
                                NULL,
                                GRN_OBJ_TABLE_NO_KEY | GRN_OBJ_PERSISTENT,
                                NULL, NULL);
      columns_.type_ =
        grn_column_create(ctx_, table_,
                          COLUMN_TYPE_NAME, strlen(COLUMN_TYPE_NAME),
                          NULL,
                          GRN_OBJ_COLUMN_SCALAR | GRN_OBJ_PERSISTENT,
                          grn_ctx_at(ctx_, GRN_DB_SHORT_TEXT));
      columns_.table_ =
        grn_column_create(ctx_, table_,
                          COLUMN_TABLE_NAME, strlen(COLUMN_TABLE_NAME),
                          NULL,
                          GRN_OBJ_COLUMN_SCALAR | GRN_OBJ_PERSISTENT,
                          grn_ctx_at(ctx_, GRN_DB_SHORT_TEXT));
      columns_.record_ =
        grn_column_create(ctx_, table_,
                          COLUMN_RECORD_NAME, strlen(COLUMN_RECORD_NAME),
                          NULL,
                          GRN_OBJ_COLUMN_SCALAR | GRN_OBJ_PERSISTENT,
                          grn_ctx_at(ctx_, GRN_DB_UINT32));
    } else {
      columns_.type_   = grn_ctx_get(ctx_, TABLE_NAME "." COLUMN_TYPE_NAME, -1);
      columns_.table_  = grn_ctx_get(ctx_, TABLE_NAME "." COLUMN_TABLE_NAME, -1);
      columns_.record_ = grn_ctx_get(ctx_, TABLE_NAME "." COLUMN_RECORD_NAME, -1);
    }

    DBUG_VOID_RETURN;
  }

  Operations::~Operations() {
    MRN_DBUG_ENTER_METHOD();

    grn_obj_unlink(ctx_, columns_.record_);
    grn_obj_unlink(ctx_, columns_.table_);
    grn_obj_unlink(ctx_, columns_.type_);
    grn_obj_unlink(ctx_, table_);
    GRN_OBJ_FIN(ctx_, &id_buffer_);
    GRN_OBJ_FIN(ctx_, &text_buffer_);

    DBUG_VOID_RETURN;
  }

  grn_id Operations::start(const char *type,
                           const char *table_name, size_t table_name_size) {
    MRN_DBUG_ENTER_METHOD();

    grn_id id = grn_table_add(ctx_, table_, NULL, 0, NULL);

    GRN_TEXT_SETS(ctx_, &text_buffer_, type);
    grn_obj_set_value(ctx_, columns_.type_,  id, &text_buffer_, GRN_OBJ_SET);

    GRN_TEXT_SET(ctx_, &text_buffer_, table_name, table_name_size);
    grn_obj_set_value(ctx_, columns_.table_, id, &text_buffer_, GRN_OBJ_SET);

    DBUG_RETURN(id);
  }

  void Operations::record_target(grn_id id, grn_id record_id) {
    MRN_DBUG_ENTER_METHOD();

    GRN_UINT32_SET(ctx_, &id_buffer_, record_id);
    grn_obj_set_value(ctx_, columns_.record_,  id, &id_buffer_, GRN_OBJ_SET);

    DBUG_VOID_RETURN;
  }

  void Operations::finish(grn_id id) {
    MRN_DBUG_ENTER_METHOD();

    grn_table_delete_by_id(ctx_, table_, id);

    DBUG_VOID_RETURN;
  }
}
