/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2015-2021 Sutou Kouhei <kou@clear-code.com>

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

#include "mrn_value_decoder.hpp"
#include <mrn_mysql_compat.h>

#ifdef MRN_HAVE_MY_BYTEORDER_H
#  include <my_byteorder.h>
#endif

namespace mrn {
  namespace {
#if MYSQL_VERSION_ID >= 80019 && !defined(MRN_MARIADB_P)
    inline uint16 mrn_ushortget(const uchar *source) {
      return ushortget(source);
    }

    inline float mrn_float4get(const uchar *source) {
      return float4get(source);
    }

    inline double mrn_float8get(const uchar *source) {
      return float8get(source);
    }

    inline long long int mrn_longlongget(const uchar *source) {
      return longlongget(source);
    }
#elif !defined(MRN_MARIADB_P)
    inline uint16 mrn_ushortget(const uchar *source) {
      uint16 value;
      ushortget(&value, source);
      return value;
    }

    inline float mrn_float4get(const uchar *source) {
      float value;
      float4get(&value, source);
      return value;
    }

    inline double mrn_float8get(const uchar *source) {
      double value;
      float8get(&value, source);
      return value;
    }

    inline long long int mrn_longlongget(const uchar *source) {
      long long int value;
      longlongget(&value, source);
      return value;
    }
#else
    inline uint16 mrn_ushortget(const uchar *source) {
      uint16 value;
      ushortget(value, source);
      return value;
    }

    inline float mrn_float4get(const uchar *source) {
      float value;
      float4get(value, source);
      return value;
    }

    inline double mrn_float8get(const uchar *source) {
      double value;
      float8get(value, source);
      return value;
    }

    inline long long int mrn_longlongget(const uchar *source) {
      long long int value;
      longlongget(value, source);
      return value;
    }
#endif
  }

  namespace value_decoder {
    void decode(uint16 *dest, const uchar *source) {
      MRN_DBUG_ENTER_FUNCTION();
      *dest = mrn_ushortget(source);
      DBUG_VOID_RETURN;
    };

    void decode(float *dest, const uchar *source) {
      MRN_DBUG_ENTER_FUNCTION();
      *dest = mrn_float4get(source);
      DBUG_VOID_RETURN;
    };

    void decode(double *dest, const uchar *source) {
      MRN_DBUG_ENTER_FUNCTION();
      *dest = mrn_float8get(source);
      DBUG_VOID_RETURN;
    }
    void decode(long long int *dest, const uchar *source) {
      MRN_DBUG_ENTER_FUNCTION();
      *dest = mrn_longlongget(source);
      DBUG_VOID_RETURN;
    }
  }
}
