/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2012 Kouhei Sutou <kou@clear-code.com>

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

#include "mrn_auto_increment_value_lock.hpp"

namespace mrn {
  AutoIncrementValueLock::AutoIncrementValueLock(TABLE_SHARE *table_share)
    : table_share_(table_share),
      need_lock_(table_share_->tmp_table == NO_TMP_TABLE) {
    if (need_lock_) {
      mysql_mutex_lock(&table_share_->LOCK_ha_data);
    }
  }

  AutoIncrementValueLock::~AutoIncrementValueLock() {
    if (need_lock_) {
      mysql_mutex_unlock(&table_share_->LOCK_ha_data);
    }
  }
}
