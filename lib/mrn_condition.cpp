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

#include "mrn_condition.hpp"

// for debug
#define MRN_CLASS_NAME "mrn::Condition"

namespace mrn {
  Condition::Condition(grn_ctx *ctx, grn_obj *table, bool is_storage_mode)
    : ctx_(ctx),
      table_(table),
      is_storage_mode_(is_storage_mode) {
  }

  bool Condition::is_convertable(const Item *item) {
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

  bool Condition::is_convertable(const Item_cond *cond_item) {
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

  bool Condition::is_convertable(const Item_func *func_item) {
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

  bool Condition::is_convertable_string(const Item_field *field_item,
                                        const Item *string_item) {
    MRN_DBUG_ENTER_METHOD();

    grn_obj *column;
#ifdef MRN_ITEM_HAVE_ITEM_NAME
    Item_field *field_item = static_cast<Item_field *>(left_item);
    Item_name_string *name = &(field_item->item_name);
    column = grn_obj_column(ctx_, table_, name->ptr(), name->length());
#else
    column = grn_obj_column(ctx_, table_,
                            field_item->name, strlen(field_item->name));
#endif
    if (!column) {
      DBUG_RETURN(false);
    }

    int n_indexes = grn_column_index(ctx_, column, GRN_OP_EQUAL, NULL, 0, NULL);
    bool convertable = (n_indexes > 0);
    grn_obj_unlink(ctx_, column);

    DBUG_RETURN(convertable);
  }

  const Item_func *Condition::find_match_against(const Item *item) {
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
}
