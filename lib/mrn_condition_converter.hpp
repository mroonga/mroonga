/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2013 Kouhei Sutou <kou@clear-code.com>

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

#ifndef MRN_CONDITION_CONVERTER_HPP_
#define MRN_CONDITION_CONVERTER_HPP_

#include <groonga.h>
#include <mrn_mysql_compat.h>

namespace mrn {
  class ConditionConverter {
  public:
    ConditionConverter(grn_ctx *ctx, grn_obj *table, bool is_storage_mode);
    ~ConditionConverter();

    bool is_convertable(const Item *item);
    const Item_func *find_match_against(const Item *item);
    // caller must check "where" can be convertable by
    // is_convertable(). This method doesn't validate "where".
    void convert(const Item *where, grn_obj *expression);

  private:
    grn_ctx *ctx_;
    grn_obj *table_;
    bool is_storage_mode_;
    grn_obj column_name_;
    grn_obj value_;

    bool is_convertable(const Item_cond *cond_item);
    bool is_convertable(const Item_func *func_item);
    bool is_convertable_string(const Item_field *field_item,
                               const Item *string_item);

    void convert_equal(const Item_func *func_item, grn_obj *expression);
    void append_field_value(const Item_field *field_item,
                            grn_obj *expression);
    void append_const_item(Item *const_item, grn_obj *expression);
  };
}

#endif /* MRN_CONDITION_CONVERTER_HPP_ */
