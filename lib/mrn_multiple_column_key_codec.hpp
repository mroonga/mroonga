/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2012-2013 Kouhei Sutou <kou@clear-code.com>

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

#ifndef MRN_MULTIPLE_COLUMN_KEY_CODEC_HPP_
#define MRN_MULTIPLE_COLUMN_KEY_CODEC_HPP_

#include <mrn_mysql.h>
#include <mrn_mysql_compat.h>

namespace mrn {
  class MultipleColumnKeyCodec {
  public:
    MultipleColumnKeyCodec(KEY *key_info);
    ~MultipleColumnKeyCodec();

    int encode(const uchar *mysql_key, uint mysql_key_length,
               uchar *encoded, uint *encoded_length,
               bool decode);
    uint size();

  private:
    enum DataType {
      TYPE_UNKNOWN,
      TYPE_LONG_LONG_NUMBER,
      TYPE_NUMBER,
      TYPE_FLOAT,
      TYPE_DOUBLE,
      TYPE_BYTE_SEQUENCE,
      TYPE_BYTE_REVERSE,
      TYPE_BYTE_BLOB
    };

    KEY *key_info_;

    void get_key_info(KEY_PART_INFO *key_part,
                      DataType *data_type, uint *data_size);

    void encode_float(volatile float value, uint data_size,
                      uchar *encoded, bool decode);
    void encode_double(volatile double value, uint data_size,
                       uchar *encoded, bool decode);
    void encode_reverse(const uchar *mysql_key, uint data_size, uchar *encoded);
  };
}

#endif // MRN_MULTIPLE_COLUMN_KEY_CODEC_HPP_
