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

#ifndef MRN_CONDITION_HPP_
#define MRN_CONDITION_HPP_

#include <groonga.h>
#include <mrn_mysql_compat.h>

namespace mrn {
  class Condition {
  public:
    Condition(grn_ctx *ctx, grn_obj *table, bool is_storage_mode);
    bool is_convertable(const Item *item);
    const Item_func *find_match_against(const Item *item);

  private:
    grn_ctx *ctx_;
    grn_obj *table_;
    bool is_storage_mode_;

    bool is_convertable(const Item_cond *cond_item);
    bool is_convertable(const Item_func *func_item);
    bool is_convertable_string(const Item_field *field_item,
                               const Item *string_item);
  };
}

#endif /* MRN_CONDITION_HPP_ */
