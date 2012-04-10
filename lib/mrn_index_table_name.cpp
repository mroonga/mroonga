/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2011 Kentoku SHIBA
  Copyright(C) 2011-2012 Kouhei Sutou <kou@clear-code.com>

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

#include "mrn_index_table_name.hpp"

namespace mrn {
  IndexTableName::IndexTableName(const char *table_name,
                                 const char *mysql_index_name)
    : table_name_(table_name),
      mysql_index_name_(mysql_index_name) {
    char encoded_mysql_index_name[MRN_MAX_KEY_SIZE];
    encode(encoded_mysql_index_name,
           encoded_mysql_index_name + MRN_MAX_KEY_SIZE,
           mysql_index_name_, mysql_index_name_ + strlen(mysql_index_name_));
    snprintf(name_, MRN_MAX_KEY_SIZE,
             "%s-%s", table_name_, encoded_mysql_index_name);
    length_ = strlen(name_);
  }

  const char *IndexTableName::c_str() {
    return name_;
  }

  size_t IndexTableName::length() {
    return length_;
  }

  uint IndexTableName::encode(char *encoded_start,
                              char *encoded_end,
                              const char *mysql_string_start,
                              const char *mysql_string_end) {
    MRN_DBUG_ENTER_METHOD();
    int res1, res2;
    my_wc_t wc;
    my_charset_conv_mb_wc mb_wc = system_charset_info->cset->mb_wc;
    my_charset_conv_wc_mb wc_mb = my_charset_filename.cset->wc_mb;
    DBUG_PRINT("info", ("mroonga: in=%s", mysql_string_start));
    encoded_end--;
    char *encoded;
    const char *mysql_string;
    for (encoded = encoded_start, mysql_string = mysql_string_start;
         mysql_string < mysql_string_end && encoded < encoded_end;
         mysql_string += res1, encoded += res2)
    {
      if ((res1 = (*mb_wc)(NULL, &wc,
                           (uchar *)mysql_string,
                           (uchar *)mysql_string_end)) > 0)
      {
        if ((res2 = (*wc_mb)(NULL, wc,
                             (uchar *)encoded,
                             (uchar *)encoded_end)) <= 0)
        {
          break;
        }
      } else if (res1 == MY_CS_ILSEQ)
      {
        *encoded = *mysql_string;
        res1 = 1;
        res2 = 1;
      } else {
        break;
      }
    }
    *encoded = '\0';
    DBUG_PRINT("info", ("mroonga: out=%s", encoded_start));
    DBUG_RETURN(encoded - encoded_start);
  }
}
