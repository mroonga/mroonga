/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2013  Kouhei Sutou <kou@clear-code.com>

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

#include "mrn_field.hpp"
#include "mrn_encoding.hpp"

// for debug
#define MRN_CLASS_NAME "mrn::Field"

namespace mrn {
  Field::Field(grn_ctx *ctx, THD *thread, ::Field *field)
    : ctx_(ctx),
      thread_(thread),
      field_(field) {
  }

  Field::~Field() {
  }

  bool Field::is_need_normalize() {
    MRN_DBUG_ENTER_METHOD();

    DBUG_PRINT("info",
               ("mroonga: result_type = %u", field_->result_type()));
    DBUG_PRINT("info",
               ("mroonga: charset->name = %s", field_->charset()->name));
    DBUG_PRINT("info",
               ("mroonga: charset->csname = %s", field_->charset()->csname));
    DBUG_PRINT("info",
               ("mroonga: charset->state = %u", field_->charset()->state));
    bool need_normalize_p;
    if (field_->charset()->state & (MY_CS_BINSORT | MY_CS_CSSORT)) {
      need_normalize_p = false;
      DBUG_PRINT("info",
                 ("mroonga: is_need_normalize: false: sort is required"));
    } else {
      if (is_text_type()) {
        need_normalize_p = true;
        DBUG_PRINT("info", ("mroonga: is_need_normalize: true: text type"));
      } else {
        need_normalize_p = false;
        DBUG_PRINT("info", ("mroonga: is_need_normalize: false: no text type"));
      }
    }

    DBUG_RETURN(need_normalize_p);
  }

  bool Field::is_text_type() {
    MRN_DBUG_ENTER_METHOD();
    bool text_type_p;
    switch (field_->type()) {
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_VAR_STRING:
      text_type_p = true;
      break;
    case MYSQL_TYPE_STRING:
      switch (field_->real_type()) {
      case MYSQL_TYPE_ENUM:
      case MYSQL_TYPE_SET:
        text_type_p = false;
        break;
      default:
        text_type_p = true;
        break;
      }
      break;
    default:
      text_type_p = false;
      break;
    }
    DBUG_RETURN(text_type_p);
  }
}
