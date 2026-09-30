/* -*- c-basic-offset: 2 -*- */
/*
  Copyright (C) 2014-2026  Sutou Kouhei <kou@clear-code.com>

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

#ifndef MRN_GRN_HPP_
#define MRN_GRN_HPP_

#include <groonga.h>

#include <cstdint>
#include <string_view>

namespace mrn {
  namespace grn {
    inline bool is_table(grn_obj* obj)
    {
      grn_id type = obj->header.type;
      return GRN_TABLE_HASH_KEY <= type && obj->header.type <= GRN_DB;
    }

    inline bool is_vector_column(grn_obj* column)
    {
      int column_type = (column->header.flags & GRN_OBJ_COLUMN_TYPE_MASK);
      return column_type == GRN_OBJ_COLUMN_VECTOR;
    }

    inline grn_obj* ctx_get(grn_ctx* ctx, std::string_view name)
    {
      return grn_ctx_get(ctx, name.data(), static_cast<int>(name.size()));
    }

    inline grn_obj* column_create(grn_ctx* ctx,
                                  grn_obj* table,
                                  std::string_view name,
                                  const char* path,
                                  grn_column_flags flags,
                                  grn_obj* type)
    {
      return grn_column_create(ctx,
                               table,
                               name.data(),
                               static_cast<unsigned int>(name.size()),
                               path,
                               flags,
                               type);
    }

    inline grn_obj* table_create(grn_ctx* ctx,
                                 std::string_view name,
                                 const char* path,
                                 grn_table_flags flags,
                                 grn_obj* key_type,
                                 grn_obj* value_type)
    {
      return grn_table_create(ctx,
                              name.data(),
                              static_cast<unsigned int>(name.size()),
                              path,
                              flags,
                              key_type,
                              value_type);
    }

    inline grn_id
    table_add(grn_ctx* ctx, grn_obj* table, std::string_view key, int* added)
    {
      return grn_table_add(ctx,
                           table,
                           key.data(),
                           static_cast<unsigned int>(key.size()),
                           added);
    }

    inline grn_id
    table_add(grn_ctx* ctx, grn_obj* table, grn_obj* key, int* added)
    {
      return table_add(
        ctx,
        table,
        std::string_view(GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key)),
        added);
    }

    inline grn_id table_get(grn_ctx* ctx, grn_obj* table, std::string_view key)
    {
      return grn_table_get(ctx,
                           table,
                           key.data(),
                           static_cast<unsigned int>(key.size()));
    }

    inline grn_id table_get(grn_ctx* ctx, grn_obj* table, grn_obj* key)
    {
      return table_get(
        ctx,
        table,
        std::string_view(GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key)));
    }

    inline grn_id
    hash_get(grn_ctx* ctx, grn_hash* hash, std::string_view key, void** value)
    {
      return grn_hash_get(ctx,
                          hash,
                          key.data(),
                          static_cast<unsigned int>(key.size()),
                          value);
    }

    inline grn_id hash_add(grn_ctx* ctx,
                           grn_hash* hash,
                           std::string_view key,
                           void** value,
                           int* added)
    {
      return grn_hash_add(ctx,
                          hash,
                          key.data(),
                          static_cast<unsigned int>(key.size()),
                          value,
                          added);
    }

    inline grn_obj* snip_open(grn_ctx* ctx,
                              int flags,
                              unsigned int width,
                              unsigned int max_results,
                              std::string_view default_open_tag,
                              std::string_view default_close_tag,
                              grn_snip_mapping* mapping)
    {
      return grn_snip_open(ctx,
                           flags,
                           width,
                           max_results,
                           default_open_tag.data(),
                           static_cast<unsigned int>(default_open_tag.size()),
                           default_close_tag.data(),
                           static_cast<unsigned int>(default_close_tag.size()),
                           mapping);
    }

    inline grn_rc
    table_rename(grn_ctx* ctx, grn_obj* table, std::string_view name)
    {
      return grn_table_rename(ctx,
                              table,
                              name.data(),
                              static_cast<unsigned int>(name.size()));
    }

    inline grn_rc
    column_rename(grn_ctx* ctx, grn_obj* column, std::string_view name)
    {
      return grn_column_rename(ctx,
                               column,
                               name.data(),
                               static_cast<unsigned int>(name.size()));
    }

    inline grn_obj*
    obj_column(grn_ctx* ctx, grn_obj* table, std::string_view name)
    {
      return grn_obj_column(ctx,
                            table,
                            name.data(),
                            static_cast<uint32_t>(name.size()));
    }

    inline grn_rc obj_remove_force(grn_ctx* ctx, std::string_view name)
    {
      return grn_obj_remove_force(ctx,
                                  name.data(),
                                  static_cast<int>(name.size()));
    }
  } // namespace grn
} // namespace mrn

#endif // MRN_GRN_HPP_
