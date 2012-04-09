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

#include "mrn_path_encoder.hpp"

namespace mrn {
  PathEncoder::PathEncoder(const char *name)
    : name_(name) {
    path_[0] = '\0';
  }

  const char *PathEncoder::path() {
    if (path_[0] != '\0') {
      return path_;
    }

    encode(path_, path_ + MRN_MAX_PATH_SIZE,
           name_, name_ + strlen(name_));
    return path_;
  }

  uint PathEncoder::encode(char *buf_st, char *buf_ed, const char *st, const char *ed) {
    int res1, res2;
    char *buf = buf_st;
    my_wc_t wc;
    my_charset_conv_mb_wc mb_wc = system_charset_info->cset->mb_wc;
    my_charset_conv_wc_mb wc_mb = my_charset_filename.cset->wc_mb;
    DBUG_ENTER("mrn_encode");
    DBUG_PRINT("info", ("mroonga: in=%s", st));
    buf_ed--;
    for (; st < ed && buf < buf_ed; st += res1, buf += res2)
    {
      if ((res1 = (*mb_wc)(NULL, &wc, (uchar *) st, (uchar *) ed)) > 0)
      {
        if ((res2 = (*wc_mb)(NULL, wc, (uchar *) buf, (uchar *) buf_ed)) <= 0)
        {
          break;
        }
      } else if (res1 == MY_CS_ILSEQ)
      {
        *buf = *st;
        res1 = 1;
        res2 = 1;
      } else {
        break;
      }
    }
    *buf = '\0';
    DBUG_PRINT("info", ("mroonga: out=%s", buf_st));
    DBUG_RETURN(buf - buf_st);
  }
}
