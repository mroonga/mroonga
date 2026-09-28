/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2014 Kouhei Sutou <kou@clear-code.com>

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

    // grn_ctx_get() that accepts std::string_view.
    inline grn_obj* ctx_get(grn_ctx* ctx, std::string_view name)
    {
      return grn_ctx_get(ctx, name.data(), static_cast<int>(name.size()));
    }
  } // namespace grn
} // namespace mrn

#endif // MRN_GRN_HPP_
