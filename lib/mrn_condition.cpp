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
  Condition::Condition(bool is_storage_mode)
    : is_storage_mode_(is_storage_mode) {
  }

  bool Condition::is_convertable(const Item *item) {
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
            if (!is_convertable(sub_item)) {
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
                (right_item->type() == Item::STRING_ITEM ||
                 right_item->type() == Item::INT_ITEM)) {
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
