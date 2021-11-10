/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2017-2021  Sutou Kouhei <kou@clear-code.com>

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

#include "mrn_table_data_switcher.hpp"

#include <table.h>
#include <field.h>

namespace mrn {
  TableDataSwitcher::TableDataSwitcher(TABLE *from_table,
                                       TABLE *to_table)
    : from_table_(from_table),
      to_table_(to_table),
      diff_(to_table_->record[0] - from_table_->record[0]) {
    uint n_columns = from_table_->s->fields;
    for (uint i = 0; i < n_columns; ++i) {
      Field *field = from_table_->field[i];
      field->move_field_offset(diff_);
      field->table = to_table_;
    }
  }

  TableDataSwitcher::~TableDataSwitcher() {
    uint n_columns = from_table_->s->fields;
    for (uint i = 0; i < n_columns; ++i) {
      Field *field = from_table_->field[i];
      field->move_field_offset(-diff_);
      field->table = from_table_;
    }
  }
}
