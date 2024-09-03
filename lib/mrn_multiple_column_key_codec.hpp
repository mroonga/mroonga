/* -*- c-basic-offset: 2 -*- */
/*
  Copyright (C) 2012-2024  Sutou Kouhe <kou@clear-code.com>

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
  class MultipleColumnKeyCodec {
  public:
    MultipleColumnKeyCodec(grn_ctx *ctx, THD *thread, KEY *key_info);
    ~MultipleColumnKeyCodec();

    int encode(const uchar *mysql_key, uint mysql_key_length,
               uchar *grn_key, uint *grn_key_length);
    int decode(const uchar *grn_key, uint grn_key_length,
               uchar *mysql_key, uint *mysql_key_length);
    uint size();

  private:
    enum DataType {
      TYPE_UNKNOWN,
      TYPE_LONG_LONG_NUMBER,
      TYPE_NUMBER,
      TYPE_FLOAT,
      TYPE_DOUBLE,
      TYPE_DATETIME,
#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
      TYPE_DATETIME2,
#endif
      TYPE_BYTE_SEQUENCE,
      TYPE_BYTE_REVERSE,
      TYPE_BYTE_BLOB
    };

    grn_ctx *ctx_;
    THD *thread_;
    KEY *key_info_;

    bool is_reverse_sort(KEY_PART_INFO *key_part) {
      return key_part->key_part_flag & HA_REVERSE_SORT;
    }

    void get_key_info(KEY_PART_INFO *key_part,
                      DataType *data_type, uint *data_size);

    void encode_number(KEY_PART_INFO *key_part,
                       const uchar *mysql_key,
                       uint mysql_key_size,
                       bool is_signed,
                       uchar *grn_key);
    void decode_number(KEY_PART_INFO *key_part,
                       const uchar *grn_key,
                       uint grn_key_size,
                       bool is_signed,
                       uchar *mysql_key);
    void encode_long_long_int(KEY_PART_INFO *key_part,
                              volatile long long int value,
                              uchar *grn_key);
    void decode_long_long_int(KEY_PART_INFO *key_part,
                              const uchar *grn_key,
                              long long int *value);
    void encode_float(KEY_PART_INFO *key_part,
                      volatile float value,
                      uint value_size,
                      uchar *grn_key);
    void decode_float(KEY_PART_INFO *key_part,
                      const uchar *grn_key,
                      uint grn_key_size,
                      uchar *mysql_key);
    void encode_double(KEY_PART_INFO *key_part,
                       volatile double value,
                       uint value_size,
                       uchar *grn_key);
    void decode_double(KEY_PART_INFO *key_part,
                       const uchar *grn_key,
                       uint grn_key_size,
                       uchar *mysql_key);
    void encode_sequence(KEY_PART_INFO *key_part,
                        const uchar *mysql_key,
                        uint mysql_key_size,
                        uchar *grn_key);
    void decode_sequence(KEY_PART_INFO *key_part,
                         const uchar *grn_key,
                         uint grn_key_size,
                         uchar *mysql_key);
    void encode_reverse(KEY_PART_INFO *key_part,
                        const uchar *mysql_key,
                        uint mysql_key_size,
                        uchar *grn_key);
    void decode_reverse(KEY_PART_INFO *key_part,
                        const uchar *grn_key,
                        uint grn_key_size,
                        uchar *mysql_key);
    void encode_blob(KEY_PART_INFO *key_part,
                     const uchar *mysql_key,
                     uint *mysql_key_size,
                     Field *field,
                     uchar *grn_key);
    void decode_blob(KEY_PART_INFO *key_part,
                     const uchar *grn_key,
                     uint grn_key_size,
                     uchar *mysql_key);
  };
}

