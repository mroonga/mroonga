/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2019  Sutou Kouhei <kou@clear-code.com>

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

#include <mrn_mysql.h>
#include <mrn_mysql_compat.h>

#include <groonga.h>

namespace mrn {
  template <typename TIMESTAMP_FIELD>
  class TimestampFieldValueConverter {
  public:
    TimestampFieldValueConverter(TIMESTAMP_FIELD *field)
      : field_(field) {
    }

    ~TimestampFieldValueConverter() {
    }

    int64_t convert() const {
      int64_t grn_time = 0;
#ifdef MRN_TIMESTAMP_USE_TIMEVAL
      int warnings = 0;
      struct timeval time_value;
      if (field_->get_timestamp(&time_value, &warnings)) {
        // XXX: Should we report warnings or MySQL does?
      } else {
        grn_time = GRN_TIME_PACK(time_value.tv_sec, time_value.tv_usec);
      }
#elif defined(MRN_TIMESTAMP_USE_MY_TIME_T_AND_POS)
      unsigned long int micro_seconds;
      my_time_t seconds = field_->get_timestamp(field_->ptr, &micro_seconds);
      grn_time = GRN_TIME_PACK(seconds, micro_seconds);
#elif defined(MRN_TIMESTAMP_USE_MY_TIME_T)
      unsigned long int micro_seconds;
      my_time_t seconds = field_->get_timestamp(&micro_seconds);
      grn_time = GRN_TIME_PACK(seconds, micro_seconds);
#else
      mrn_bool is_null_value;
      long seconds = field_->get_timestamp(&is_null_value);
      grn_time = GRN_TIME_PACK(seconds, 0);
#endif
      return grn_time;
    }

  private:
    TIMESTAMP_FIELD *field_;
  };
}
