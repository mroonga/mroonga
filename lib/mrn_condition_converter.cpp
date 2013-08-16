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

namespace mrn {
  ConditionConverter::ConditionConverter(bool is_storage_mode,
                                         const Item *where)
    : is_storage_mode_(is_storage_mode),
      where_(where) {
  }

  void ConditionConverter::convert(grn_ctx *ctx, grn_obj *expression) {
    MRN_DBUG_ENTER_METHOD();

    if (!where_ || where_->type() != Item::COND_ITEM) {
      DBUG_VOID_RETURN;
    }

    grn_obj column_name, value;
    GRN_TEXT_INIT(&column_name, 0);
    GRN_VOID_INIT(&value);
    Item_cond *cond_item = (Item_cond *)where_;
    List_iterator<Item> iterator(*((cond_item)->argument_list()));
    const Item *sub_item;
    while ((sub_item = iterator++)) {
      switch (sub_item->type()) {
      case Item::FUNC_ITEM:
        {
          const Item_func *func_item = (const Item_func *)sub_item;
          switch (func_item->functype()) {
          case Item_func::EQ_FUNC:
            {
              Item **arguments = func_item->arguments();
              Item *left_item = arguments[0];
              Item *right_item = arguments[1];
              if (left_item->type() == Item::FIELD_ITEM) {
                GRN_BULK_REWIND(&column_name);
#ifdef MRN_ITEM_HAVE_ITEM_NAME
                Item_name_string *name = &(left_item->item_name);
                GRN_TEXT_PUT(ctx, &column_name,
                             name->ptr(), name->length());
#else
                GRN_TEXT_PUTS(ctx, &column_name, left_item->name);
#endif
                grn_expr_append_const(ctx, expression, &column_name,
                                      GRN_OP_PUSH, 1);
                grn_expr_append_op(ctx, expression, GRN_OP_GET_VALUE, 1);
                grn_obj_reinit(ctx, &value, GRN_DB_INT64, 0);
                GRN_INT64_SET(ctx, &value, right_item->val_int());
                grn_expr_append_const(ctx, expression, &value,
                                      GRN_OP_PUSH, 1);
                grn_expr_append_op(ctx, expression, GRN_OP_EQUAL, 2);
                grn_expr_append_op(ctx, expression, GRN_OP_AND, 2);
              }
            }
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
    grn_obj_unlink(ctx, &column_name);
    grn_obj_unlink(ctx, &value);

    DBUG_VOID_RETURN;
  }

  bool ConditionConverter::is_convertable() {
    MRN_DBUG_ENTER_METHOD();
    bool convertable = is_convertable_recursive(where_);
    DBUG_RETURN(convertable);
  }

  bool ConditionConverter::is_convertable_recursive(const Item *item) {
    MRN_DBUG_ENTER_METHOD();

    if (!item) {
      DBUG_RETURN(false);
    }

    bool convertable = false;
    switch (item->type()) {
    case Item::COND_ITEM:
      if (is_storage_mode_) {
        Item_cond *cond_item = (Item_cond *)item;
        if (cond_item->functype() == Item_func::COND_AND_FUNC) {
          List_iterator<Item> iterator(*((cond_item)->argument_list()));
          const Item *sub_item;
          convertable = true;
          while ((sub_item = iterator++)) {
            if (!is_convertable_recursive(sub_item)) {
              convertable = false;
              break;
            }
          }
        }
      }
      break;
    case Item::FUNC_ITEM:
      {
        const Item_func *func_item = (const Item_func *)item;
        switch (func_item->functype()) {
        case Item_func::EQ_FUNC:
          if (is_storage_mode_) {
            Item **arguments = func_item->arguments();
            Item *left_item = arguments[0];
            Item *right_item = arguments[1];
              if (left_item->type() == Item::FIELD_ITEM &&
                  right_item->basic_const_item() &&
                  right_item->type() == Item::INT_ITEM) {
                convertable = true;
              }
          }
          break;
        case Item_func::FT_FUNC:
          convertable = true;
          break;
        default:
          break;
        }
      }
      break;
    default:
      break;
    }

    DBUG_RETURN(convertable);
  }

  const Item_func *ConditionConverter::find_match_against() {
    MRN_DBUG_ENTER_METHOD();
    const Item_func *match_against = find_match_against_recursive(where_);
    DBUG_RETURN(match_against);
  }

  const Item_func *ConditionConverter::find_match_against_recursive(const Item *item) {
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
            const Item_func *match_against =
              find_match_against_recursive(sub_item);
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
}
