/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2013-2021  Sutou Kouhei <kou@clear-code.com>

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

#include <mrn_mysql_compat.h>

#include "mrn_smart_grn_obj.hpp"

#include <item_cmpfunc.h>

#include <list>
#include <vector>

#include <groonga.h>

namespace mrn {
  class ConditionConverter {
  public:
    ConditionConverter(THD *thrad,
                       grn_ctx *ctx,
                       grn_obj *table,
                       grn_obj **index_columns,
                       KEY *key_infos,
                       bool is_storage_mode);
    ~ConditionConverter();

    bool is_convertable(const Item *item,
                        grn_operator logical_operator=GRN_OP_AND);
    unsigned int count_match_against(const Item *item);
    // caller must check "where" can be convertable by
    // is_convertable(). This method doesn't validate "where".
    grn_encoding convert(const Item *where,
                         grn_obj *expression,
                         std::list<SmartGrnObj> &match_columns_list);

  private:
    enum NormalizedType {
      STRING_TYPE,
      INT_TYPE,
      TIME_TYPE,
      UNSUPPORTED_TYPE,
    };

    THD *thread_;
    grn_ctx *ctx_;
    grn_obj *table_;
    grn_obj **index_columns_;
    KEY *key_infos_;
    bool is_storage_mode_;
    grn_obj column_name_;
    grn_obj value_;

    bool is_convertable(const Item *item,
                        grn_operator logical_operator,
                        std::vector<grn_encoding> &encodings);
    bool is_convertable(const Item_cond *cond_item,
                        grn_operator logical_operator,
                        std::vector<grn_encoding> &encodings);
    bool is_convertable(const Item_func *func_item,
                        grn_operator logical_operator,
                        std::vector<grn_encoding> &encodings);
    bool is_convertable_binary_operation(const Item_field *field_item,
                                         Item *value_item,
                                         Item_func::Functype func_type,
                                         std::vector<grn_encoding> &encodings);
    bool is_convertable_between(const Item_field *field_item,
                                Item *min_item,
                                Item *max_item);
    bool is_convertable_in(const Item_field *field_item,
                           Item **value_items,
                           uint n_value_items);
    bool is_valid_time_value(const Item_field *field_item,
                             Item *value_item);
    bool get_time_value(const Item_field *field_item,
                        Item *value_item,
                        MYSQL_TIME *mysql_time);
    bool have_index(const Item_field *field_item, grn_operator _operator);
    bool have_index(const Item_field *field_item, Item_func::Functype func_type);

    NormalizedType normalize_field_type(enum_field_types field_type);

    bool convert(const Item_cond *cond_item,
                 grn_obj *expression,
                 std::list<SmartGrnObj> &match_columns_list,
                 std::vector<grn_encoding> &encodings);
    bool convert(const Item_func *func_item,
                 grn_obj *expression,
                 std::list<SmartGrnObj> &match_columns_list,
                 std::vector<grn_encoding> &encodings,
                 bool have_condition);

    bool convert_binary_operation(const Item_func *func_item,
                                  grn_obj *expression,
                                  grn_operator _operator);
    bool convert_between(const Item_func *func_item, grn_obj *expression);
    bool convert_in(const Item_func *func_item,
                    grn_obj *expression,
                    bool have_condition);
    bool convert_full_text_search(
      const Item_func *func_item,
      grn_obj *expression,
      std::list<SmartGrnObj> &match_columns_list,
      std::vector<grn_encoding> &encodings);
    void append_field_value(const Item_field *field_item,
                            grn_obj *expression);
    void append_const_item(const Item_field *field_item,
                           Item *const_item,
                           grn_obj *expression);
  };
}
