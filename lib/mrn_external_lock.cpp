/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2012 Kentoku SHIBA

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

#include "mrn_external_lock.hpp"

namespace mrn {
  ExternalLock::ExternalLock(THD *thd, handler *handler, int lock_type)
    : thd_(thd),
    handler_(handler),
    lock_type_(lock_type) {
    if (lock_type_ != F_UNLCK) {
      error_ = handler_->ha_external_lock(thd_, lock_type);
    } else {
      error_ = 0;
    }
  }

  ExternalLock::~ExternalLock() {
    if (lock_type_ != F_UNLCK) {
      handler_->ha_external_lock(thd_, F_UNLCK);
    }
  }

  int ExternalLock::error() {
    return error_;
  }
}
