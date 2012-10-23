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

#include <mrn_mysql.h>

#include "mrn_multiple_column_key_codec.hpp"

// for debug
#define MRN_CLASS_NAME "mrn::MultipleColumnKeyCodec"

#ifdef WORDS_BIGENDIAN
#define mrn_byte_order_host_to_network(buf, key, size)  \
{                                                       \
  uint32 size_ = (uint32)(size);                        \
  uint8 *buf_ = (uint8 *)(buf);                         \
  uint8 *key_ = (uint8 *)(key);                         \
  while (size_--) { *buf_++ = *key_++; }                \
}
#else /* WORDS_BIGENDIAN */
#define mrn_byte_order_host_to_network(buf, key, size)  \
{                                                       \
  uint32 size_ = (uint32)(size);                        \
  uint8 *buf_ = (uint8 *)(buf);                         \
  uint8 *key_ = (uint8 *)(key) + size_;                 \
  while (size_--) { *buf_++ = *(--key_); }              \
}
#endif /* WORDS_BIGENDIAN */

namespace mrn {
  MultipleColumnKeyCodec::MultipleColumnKeyCodec(KEY *key_info)
    : key_info_(key_info) {
  }

  MultipleColumnKeyCodec::~MultipleColumnKeyCodec() {
  }

  int MultipleColumnKeyCodec::encode(const uchar *key, uint key_length,
                                     uchar *buffer, uint *encoded_length,
                                     bool decode) {
    MRN_DBUG_ENTER_METHOD();
    int error = 0;
    const uchar *current_key = key;
    const uchar *key_end = key + key_length;
    uchar *current_buffer = buffer;

    int n_key_parts = key_info_->key_parts;
    DBUG_PRINT("info", ("mroonga: n_key_parts=%d", n_key_parts));
    *encoded_length = 0;
    for (int i = 0; i < n_key_parts && current_key < key_end; i++) {
      KEY_PART_INFO key_part = key_info_->key_part[i];
      Field *field = key_part.field;
      DBUG_PRINT("info", ("mroonga: key_part.length=%u", key_part.length));

      if (field->null_bit) {
        DBUG_PRINT("info", ("mroonga: field has null bit"));
        *current_buffer = *current_key;
        current_key += 1;
        current_buffer += 1;
        (*encoded_length)++;
      }

      DataType data_type = TYPE_UNKNOWN;
      uint data_size = 0;
      long long int long_long_value = 0;
      switch (field->real_type()) {
      case MYSQL_TYPE_DECIMAL:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_DECIMAL"));
        data_type = TYPE_BYTE_SEQUENCE;
        data_size = key_part.length;
        break;
      case MYSQL_TYPE_TINY:
      case MYSQL_TYPE_YEAR:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_TINY"));
        data_type = TYPE_NUMBER;
        data_size = 1;
        break;
      case MYSQL_TYPE_SHORT:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_SHORT"));
        data_type = TYPE_NUMBER;
        data_size = 2;
        break;
      case MYSQL_TYPE_LONG:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_LONG"));
        data_type = TYPE_NUMBER;
        data_size = 4;
        break;
      case MYSQL_TYPE_FLOAT:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_FLOAT"));
        data_type = TYPE_FLOAT;
        data_size = 4;
        {
          float value;
          float4get(value, current_key);
          encode_float(value, data_size, current_buffer, decode);
        }
        break;
      case MYSQL_TYPE_DOUBLE:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_DOUBLE"));
        data_type = TYPE_DOUBLE;
        data_size = 8;
        {
          double value;
          float8get(value, current_key);
          encode_double(value, data_size, current_buffer, decode);
        }
        break;
      case MYSQL_TYPE_NULL:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_NULL"));
        data_type = TYPE_NUMBER;
        data_size = 1;
        break;
      case MYSQL_TYPE_TIMESTAMP:
      case MYSQL_TYPE_DATE:
      case MYSQL_TYPE_DATETIME:
      case MYSQL_TYPE_NEWDATE:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_DATETIME"));
        data_type = TYPE_BYTE_REVERSE;
        data_size = key_part.length;
        encode_reverse(current_key, data_size, current_buffer);
        break;
      case MYSQL_TYPE_LONGLONG:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_LONGLONG"));
        data_type = TYPE_NUMBER;
        data_size = 8;
        break;
      case MYSQL_TYPE_INT24:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_INT24"));
        data_type = TYPE_NUMBER;
        data_size = 3;
        break;
      case MYSQL_TYPE_TIME:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_TIME"));
        data_type = TYPE_LONG_LONG_NUMBER;
        long_long_value = (long long int)sint3korr(current_key);
        data_size = 3;
        break;
      case MYSQL_TYPE_VARCHAR:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_VARCHAR"));
        data_type = TYPE_BYTE_BLOB;
        data_size = key_part.length;
        break;
      case MYSQL_TYPE_BIT:
        // TODO
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_BIT"));
        data_type = TYPE_NUMBER;
        data_size = 1;
        break;
#ifdef MRN_HAVE_MYSQL_TYPE_TIMESTAMP2
      case MYSQL_TYPE_TIMESTAMP2:
        // TODO
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_TIMESTAMP2"));
        data_type = TYPE_LONG_LONG_NUMBER;
        long_long_value = (long long int)sint8korr(current_key);
        data_size = 8;
        break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
      case MYSQL_TYPE_DATETIME2:
        // TODO
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_DATETIME2"));
        data_type = TYPE_LONG_LONG_NUMBER;
        long_long_value = (long long int)sint8korr(current_key);
        data_size = 8;
        break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
      case MYSQL_TYPE_TIME2:
        // TODO
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_TIME2"));
        data_type = TYPE_LONG_LONG_NUMBER;
        long_long_value = (long long int)sint8korr(current_key);
        data_size = 8;
        break;
#endif
      case MYSQL_TYPE_NEWDECIMAL:
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_NEWDECIMAL"));
        data_type = TYPE_BYTE_SEQUENCE;
        data_size = key_part.length;
        break;
      case MYSQL_TYPE_ENUM:
        // TODO
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_ENUM"));
        data_type = TYPE_NUMBER;
        data_size = 1;
        break;
      case MYSQL_TYPE_SET:
        // TODO
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_SET"));
        data_type = TYPE_NUMBER;
        data_size = 1;
        break;
      case MYSQL_TYPE_TINY_BLOB:
      case MYSQL_TYPE_MEDIUM_BLOB:
      case MYSQL_TYPE_LONG_BLOB:
      case MYSQL_TYPE_BLOB:
        // TODO
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_BLOB"));
        data_type = TYPE_BYTE_BLOB;
        data_size = key_part.length;
        break;
      case MYSQL_TYPE_VAR_STRING:
      case MYSQL_TYPE_STRING:
        // TODO
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_STRING"));
        data_type = TYPE_BYTE_SEQUENCE;
        data_size = key_part.length;
        break;
      case MYSQL_TYPE_GEOMETRY:
        // TODO
        DBUG_PRINT("info", ("mroonga: MYSQL_TYPE_GEOMETRY"));
        data_type = TYPE_BYTE_SEQUENCE;
        data_size = key_part.length;
        break;
      }

      switch (data_type) {
      case TYPE_UNKNOWN:
        // TODO: This will not be happen. This is just for
        // suppressing warnings by gcc -O2. :<
        error = HA_ERR_UNSUPPORTED;
        break;
      case TYPE_LONG_LONG_NUMBER:
        if (decode)
          *((uint8 *)(&long_long_value)) ^= 0x80;
        mrn_byte_order_host_to_network(current_buffer, &long_long_value,
                                       data_size);
        if (!decode)
          *((uint8 *)(current_buffer)) ^= 0x80;
        break;
      case TYPE_NUMBER:
        if (decode)
        {
          Field_num *number_field = (Field_num *)field;
          if (!number_field->unsigned_flag) {
            *((uint8 *)(current_key)) ^= 0x80;
          }
        }
        mrn_byte_order_host_to_network(current_buffer, current_key, data_size);
        if (!decode)
        {
          Field_num *number_field = (Field_num *)field;
          if (!number_field->unsigned_flag) {
            *((uint8 *)(current_buffer)) ^= 0x80;
          }
        }
        break;
      case TYPE_FLOAT:
        break;
      case TYPE_DOUBLE:
        break;
      case TYPE_BYTE_SEQUENCE:
        memcpy(current_buffer, current_key, data_size);
        break;
      case TYPE_BYTE_REVERSE:
        break;
      case TYPE_BYTE_BLOB:
        if (decode) {
          memcpy(current_buffer, current_key + data_size, HA_KEY_BLOB_LENGTH);
          memcpy(current_buffer + HA_KEY_BLOB_LENGTH, current_key, data_size);
        } else {
          memcpy(current_buffer + data_size, current_key, HA_KEY_BLOB_LENGTH);
          memcpy(current_buffer, current_key + HA_KEY_BLOB_LENGTH, data_size);
        }
        data_size += HA_KEY_BLOB_LENGTH;
        break;
      }

      if (error) {
        break;
      }

      current_key += data_size;
      current_buffer += data_size;
      *encoded_length += data_size;
    }

    DBUG_RETURN(error);
  }

  void MultipleColumnKeyCodec::encode_float(volatile float value, uint data_size,
                                            uchar *buffer, bool decode) {
    MRN_DBUG_ENTER_METHOD();
    int n_bits = (data_size * 8 - 1);
    volatile int *int_value_pointer = (int *)(&value);
    int int_value = *int_value_pointer;
    if (!decode)
      int_value ^= ((int_value >> n_bits) | (1 << n_bits));
    mrn_byte_order_host_to_network(buffer, &int_value, data_size);
    if (decode) {
      int_value = *((int *)buffer);
      *((int *)buffer) = int_value ^ (((int_value ^ (1 << n_bits)) >> n_bits) |
                                      (1 << n_bits));
    }
    DBUG_VOID_RETURN;
  }

  void MultipleColumnKeyCodec::encode_double(volatile double value, uint data_size,
                                             uchar *buffer, bool decode) {
    MRN_DBUG_ENTER_METHOD();
    int n_bits = (data_size * 8 - 1);
    volatile long long int *long_long_value_pointer = (long long int *)(&value);
    volatile long long int long_long_value = *long_long_value_pointer;
    if (!decode)
      long_long_value ^= ((long_long_value >> n_bits) | (1LL << n_bits));
    mrn_byte_order_host_to_network(buffer, &long_long_value, data_size);
    if (decode) {
      long_long_value = *((long long int *)buffer);
      *((long long int *)buffer) =
        long_long_value ^ (((long_long_value ^ (1LL << n_bits)) >> n_bits) |
                           (1LL << n_bits));
    }
    DBUG_VOID_RETURN;
  }

  void MultipleColumnKeyCodec::encode_reverse(const uchar *key, uint data_size,
                                              uchar *buffer) {
    MRN_DBUG_ENTER_METHOD();
    for (uint i = 0; i < data_size; i++) {
      buffer[i] = key[data_size - i - 1];
    }
    DBUG_VOID_RETURN;
  }
}
