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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "mrn_condition_converter.hpp"

// for debug
#define MRN_CLASS_NAME "mrn::ConditionConverter"

#ifdef MRN_ITEM_HAVE_ITEM_NAME
#  define MRN_ITEM_FIELD_GET_NAME(item)        ((item)->item_name.ptr())
#  define MRN_ITEM_FIELD_GET_NAME_LENGTH(item) ((item)->item_name.length())
#else
#  define MRN_ITEM_FIELD_GET_NAME(item)        ((item)->name)
#  define MRN_ITEM_FIELD_GET_NAME_LENGTH(item) (strlen((item)->name))
#endif

namespace mrn {
  ConditionConverter::ConditionConverter(grn_ctx *ctx, grn_obj *table,
                                         bool is_storage_mode)
    : ctx_(ctx),
      table_(table),
      is_storage_mode_(is_storage_mode) {
    GRN_TEXT_INIT(&column_name_, 0);
    GRN_VOID_INIT(&value_);
  }

  ConditionConverter::~ConditionConverter() {
    grn_obj_unlink(ctx_, &column_name_);
    grn_obj_unlink(ctx_, &value_);
  }

  bool ConditionConverter::is_convertable(const Item *item) {
    MRN_DBUG_ENTER_METHOD();

    if (!item) {
      DBUG_RETURN(false);
    }

    switch (item->type()) {
    case Item::COND_ITEM:
      {
        const Item_cond *cond_item = reinterpret_cast<const Item_cond *>(item);
        bool convertable = is_convertable(cond_item);
        DBUG_RETURN(convertable);
      }
      break;
    case Item::FUNC_ITEM:
      {
        const Item_func *func_item = reinterpret_cast<const Item_func *>(item);
        bool convertable = is_convertable(func_item);
        DBUG_RETURN(convertable);
      }
      break;
    default:
      DBUG_RETURN(false);
      break;
    }

    DBUG_RETURN(false);
  }

  bool ConditionConverter::is_convertable(const Item_cond *cond_item) {
    MRN_DBUG_ENTER_METHOD();

    if (!is_storage_mode_) {
      DBUG_RETURN(false);
    }

    if (cond_item->functype() != Item_func::COND_AND_FUNC) {
      DBUG_RETURN(false);
    }

    List<Item> *argument_list =
      const_cast<Item_cond *>(cond_item)->argument_list();
    List_iterator<Item> iterator(*argument_list);
    const Item *sub_item;
    while ((sub_item = iterator++)) {
      if (!is_convertable(sub_item)) {
        DBUG_RETURN(false);
      }
    }

    DBUG_RETURN(true);
  }

  bool ConditionConverter::is_convertable(const Item_func *func_item) {
    MRN_DBUG_ENTER_METHOD();

    switch (func_item->functype()) {
    case Item_func::EQ_FUNC:
      if (!is_storage_mode_) {
        DBUG_RETURN(false);
      }
      {
        Item **arguments = func_item->arguments();
        Item *left_item = arguments[0];
        Item *right_item = arguments[1];
        if (left_item->type() != Item::FIELD_ITEM) {
          DBUG_RETURN(false);
        }
        if (!right_item->basic_const_item()) {
          DBUG_RETURN(false);
        }

        bool convertable;
        switch (right_item->type()) {
        case Item::STRING_ITEM:
          {
            Item_field *field_item = static_cast<Item_field *>(left_item);
            convertable = is_convertable_string(field_item, right_item);
          }
          break;
        case Item::INT_ITEM:
          convertable = true;
          break;
        default:
          convertable = false;
          break;
        }
        DBUG_RETURN(convertable);
      }
      break;
    case Item_func::FT_FUNC:
      DBUG_RETURN(true);
      break;
    default:
      DBUG_RETURN(false);
      break;
    }

    DBUG_RETURN(true);
  }

  bool ConditionConverter::is_convertable_string(const Item_field *field_item,
                                                 const Item *string_item) {
    MRN_DBUG_ENTER_METHOD();

    grn_obj *column;
    column = grn_obj_column(ctx_, table_,
                            MRN_ITEM_FIELD_GET_NAME(field_item),
                            MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item));
    if (!column) {
      DBUG_RETURN(false);
    }

    int n_indexes = grn_column_index(ctx_, column, GRN_OP_EQUAL, NULL, 0, NULL);
    bool convertable = (n_indexes > 0);
    grn_obj_unlink(ctx_, column);

    DBUG_RETURN(convertable);
  }

  const Item_func *ConditionConverter::find_match_against(const Item *item) {
    MRN_DBUG_ENTER_METHOD();

    if (!item) {
      DBUG_RETURN(NULL);
    }

    switch (item->type()) {
    case Item::COND_ITEM:
      if (is_storage_mode_) {
        Item_cond *cond_item = (Item_cond *)item;
        if (cond_item->functype() == Item_func::COND_AND_FUNC) {
          List_iterator<Item> iterator(*((cond_item)->argument_list()));
          const Item *sub_item;
          while ((sub_item = iterator++)) {
            const Item_func *match_against = find_match_against(sub_item);
            if (match_against) {
              DBUG_RETURN(match_against);
            }
          }
        }
      }
      break;
    case Item::FUNC_ITEM:
      {
        const Item_func *func_item = (const Item_func *)item;
        switch (func_item->functype()) {
        case Item_func::FT_FUNC:
          DBUG_RETURN(func_item);
          break;
        default:
          break;
        }
      }
      break;
    default:
      break;
    }

    DBUG_RETURN(NULL);
  }

  void ConditionConverter::convert(const Item *where, grn_obj *expression) {
    MRN_DBUG_ENTER_METHOD();

    if (!where || where->type() != Item::COND_ITEM) {
      DBUG_VOID_RETURN;
    }

    Item_cond *cond_item = (Item_cond *)where;
    List_iterator<Item> iterator(*((cond_item)->argument_list()));
    const Item *sub_item;
    while ((sub_item = iterator++)) {
      switch (sub_item->type()) {
      case Item::FUNC_ITEM:
        {
          const Item_func *func_item = (const Item_func *)sub_item;
          switch (func_item->functype()) {
          case Item_func::EQ_FUNC:
            convert_equal(func_item, expression);
            break;
          default:
            break;
          }
        }
        break;
      default:
        break;
      }
    }

    DBUG_VOID_RETURN;
  }

  void ConditionConverter::convert_equal(const Item_func *func_item,
                                         grn_obj *expression) {
    Item **arguments = func_item->arguments();
    Item *left_item = arguments[0];
    Item *right_item = arguments[1];
    if (left_item->type() == Item::FIELD_ITEM) {
      const Item_field *field_item = static_cast<const Item_field *>(left_item);
      append_field_value(field_item, expression);
      append_const_item(right_item, expression);
      grn_expr_append_op(ctx_, expression, GRN_OP_EQUAL, 2);
      grn_expr_append_op(ctx_, expression, GRN_OP_AND, 2);
    }
  }

  void ConditionConverter::append_field_value(const Item_field *field_item,
                                              grn_obj *expression) {
    MRN_DBUG_ENTER_METHOD();

    GRN_BULK_REWIND(&column_name_);
    GRN_TEXT_PUT(ctx_, &column_name_,
                 MRN_ITEM_FIELD_GET_NAME(field_item),
                 MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item));
    grn_expr_append_const(ctx_, expression, &column_name_,
                          GRN_OP_PUSH, 1);
    grn_expr_append_op(ctx_, expression, GRN_OP_GET_VALUE, 1);

    DBUG_VOID_RETURN;
  }

  void ConditionConverter::append_const_item(Item *const_item,
                                             grn_obj *expression) {
    MRN_DBUG_ENTER_METHOD();

    switch (const_item->type()) {
    case Item::STRING_ITEM:
      grn_obj_reinit(ctx_, &value_, GRN_DB_TEXT, 0);
      {
        String *string;
        string = const_item->val_str(NULL);
        GRN_TEXT_SET(ctx_, &value_, string->ptr(), string->length());
      }
      break;
    case Item::INT_ITEM:
      grn_obj_reinit(ctx_, &value_, GRN_DB_INT64, 0);
      GRN_INT64_SET(ctx_, &value_, const_item->val_int());
      break;
    default:
      break;
    }
    grn_expr_append_const(ctx_, expression, &value_, GRN_OP_PUSH, 1);

    DBUG_VOID_RETURN;
  }
}
