/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2011-2013  Kentoku SHIBA
  Copyright(C) 2013-2021  Sutou Kouhei <kou@clear-code.com>

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

#include <mrn_err.h>
#include "mrn_encoding.hpp"

namespace mrn {
  namespace encoding {
    CHARSET_INFO *mrn_charset_utf8 = NULL;
    CHARSET_INFO *mrn_charset_utf8mb3 = NULL;
    CHARSET_INFO *mrn_charset_utf8mb4 = NULL;
    CHARSET_INFO *mrn_charset_binary = NULL;
    CHARSET_INFO *mrn_charset_ascii = NULL;
    CHARSET_INFO *mrn_charset_latin1_1 = NULL;
    CHARSET_INFO *mrn_charset_latin1_2 = NULL;
    CHARSET_INFO *mrn_charset_cp932 = NULL;
    CHARSET_INFO *mrn_charset_sjis = NULL;
    CHARSET_INFO *mrn_charset_eucjpms = NULL;
    CHARSET_INFO *mrn_charset_ujis = NULL;
    CHARSET_INFO *mrn_charset_koi8r = NULL;

    void init(void) {
      CHARSET_INFO **cs;
      MRN_DBUG_ENTER_FUNCTION();
      for (cs = all_charsets; cs < all_charsets + MY_ALL_CHARSETS_SIZE; cs++)
      {
        if (!cs[0])
          continue;
        /*
           The utf8mb3 charset is deprecated in MySQL and will be removed in a future release.
           Currently, the utf8mb3 is an alias for the utf8, but the utf8 is expected to
           become a reference to the utf8mb4 at some point.
         */
        if ((!strcmp(MRN_CHARSET_CSNAME(cs[0]), "utf8"))
            || (!strcmp(MRN_CHARSET_CSNAME(cs[0]), "utf8mb3")))
        {
          DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                              MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
          if (!mrn_charset_utf8)
            mrn_charset_utf8 = cs[0];
          else if (mrn_charset_utf8->cset != cs[0]->cset)
            assert(0);
          continue;
        }
        if (!strcmp(MRN_CHARSET_CSNAME(cs[0]), "utf8mb4"))
        {
          DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                              MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
          if (!mrn_charset_utf8mb4)
            mrn_charset_utf8mb4 = cs[0];
          else if (mrn_charset_utf8mb4->cset != cs[0]->cset)
            assert(0);
          continue;
        }
        if (!strcmp(MRN_CHARSET_CSNAME(cs[0]), "binary"))
        {
          DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                              MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
          if (!mrn_charset_binary)
            mrn_charset_binary = cs[0];
          else if (mrn_charset_binary->cset != cs[0]->cset)
            assert(0);
          continue;
        }
        if (!strcmp(MRN_CHARSET_CSNAME(cs[0]), "ascii"))
        {
          DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                              MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
          if (!mrn_charset_ascii)
            mrn_charset_ascii = cs[0];
          else if (mrn_charset_ascii->cset != cs[0]->cset)
            assert(0);
          continue;
        }
        if (!strcmp(MRN_CHARSET_CSNAME(cs[0]), "latin1"))
        {
          DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                              MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
          if (!mrn_charset_latin1_1)
            mrn_charset_latin1_1 = cs[0];
          else if (mrn_charset_latin1_1->cset != cs[0]->cset)
          {
            if (!mrn_charset_latin1_2)
              mrn_charset_latin1_2 = cs[0];
            else if (mrn_charset_latin1_2->cset != cs[0]->cset)
              assert(0);
          }
          continue;
        }
        if (!strcmp(MRN_CHARSET_CSNAME(cs[0]), "cp932"))
        {
          DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                              MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
          if (!mrn_charset_cp932)
            mrn_charset_cp932 = cs[0];
          else if (mrn_charset_cp932->cset != cs[0]->cset)
            assert(0);
          continue;
        }
        if (!strcmp(MRN_CHARSET_CSNAME(cs[0]), "sjis"))
        {
          DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                              MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
          if (!mrn_charset_sjis)
            mrn_charset_sjis = cs[0];
          else if (mrn_charset_sjis->cset != cs[0]->cset)
            assert(0);
          continue;
        }
        if (!strcmp(MRN_CHARSET_CSNAME(cs[0]), "eucjpms"))
        {
          DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                              MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
          if (!mrn_charset_eucjpms)
            mrn_charset_eucjpms = cs[0];
          else if (mrn_charset_eucjpms->cset != cs[0]->cset)
            assert(0);
          continue;
        }
        if (!strcmp(MRN_CHARSET_CSNAME(cs[0]), "ujis"))
        {
          DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                              MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
          if (!mrn_charset_ujis)
            mrn_charset_ujis = cs[0];
          else if (mrn_charset_ujis->cset != cs[0]->cset)
            assert(0);
          continue;
        }
        if (!strcmp(MRN_CHARSET_CSNAME(cs[0]), "koi8r"))
        {
          DBUG_PRINT("info", ("mroonga: %s is %s [%p]",
                              MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
          if (!mrn_charset_koi8r)
            mrn_charset_koi8r = cs[0];
          else if (mrn_charset_koi8r->cset != cs[0]->cset)
            assert(0);
          continue;
        }
        DBUG_PRINT("info", ("mroonga: %s[%s][%p] is not supported",
                            MRN_CHARSET_NAME(cs[0]), MRN_CHARSET_CSNAME(cs[0]), cs[0]->cset));
      }
      DBUG_VOID_RETURN;
    }

    int set(grn_ctx *ctx, const CHARSET_INFO *charset) {
      MRN_DBUG_ENTER_FUNCTION();
      int error = 0;

      if (!set_raw(ctx, charset)) {
        const char *name = "<null>";
        const char *csname = "<null>";
        if (charset) {
          name = MRN_CHARSET_NAME(charset);
          csname = MRN_CHARSET_CSNAME(charset);
        }
        error = ER_MRN_CHARSET_NOT_SUPPORT_NUM;
        my_printf_error(error,
                        ER_MRN_CHARSET_NOT_SUPPORT_STR,
                        MYF(0), name, csname);
      }

      DBUG_RETURN(error);
    }

    bool set_raw(grn_ctx *ctx, const CHARSET_INFO *charset) {
      MRN_DBUG_ENTER_FUNCTION();
      if (!charset) {
        GRN_CTX_SET_ENCODING(ctx, GRN_ENC_NONE);
        DBUG_RETURN(true);
      }
      grn_encoding encoding = convert(charset);
      GRN_CTX_SET_ENCODING(ctx, encoding);
      if (encoding != GRN_ENC_NONE) {
        DBUG_RETURN(true);
      }
      DBUG_RETURN(charset->cset == mrn_charset_binary->cset);
    }

    grn_encoding convert(const CHARSET_INFO *charset) {
      MRN_DBUG_ENTER_FUNCTION();
      if (!charset) {
        DBUG_RETURN(GRN_ENC_NONE);
      }
      if (charset->cset == mrn_charset_utf8->cset) {
        DBUG_RETURN(GRN_ENC_UTF8);
      }
      if (mrn_charset_utf8mb4 && charset->cset == mrn_charset_utf8mb4->cset) {
        DBUG_RETURN(GRN_ENC_UTF8);
      }
      if (charset->cset == mrn_charset_cp932->cset) {
        DBUG_RETURN(GRN_ENC_SJIS);
      }
      if (charset->cset == mrn_charset_eucjpms->cset) {
        DBUG_RETURN(GRN_ENC_EUC_JP);
      }
      if (charset->cset == mrn_charset_latin1_1->cset) {
        DBUG_RETURN(GRN_ENC_LATIN1);
      }
      if (charset->cset == mrn_charset_latin1_2->cset) {
        DBUG_RETURN(GRN_ENC_LATIN1);
      }
      if (charset->cset == mrn_charset_koi8r->cset) {
        DBUG_RETURN(GRN_ENC_KOI8R);
      }
      if (charset->cset == mrn_charset_binary->cset) {
        DBUG_RETURN(GRN_ENC_NONE);
      }
      if (charset->cset == mrn_charset_ascii->cset) {
        DBUG_RETURN(GRN_ENC_UTF8);
      }
      if (charset->cset == mrn_charset_sjis->cset) {
        DBUG_RETURN(GRN_ENC_SJIS);
      }
      if (charset->cset == mrn_charset_ujis->cset) {
        DBUG_RETURN(GRN_ENC_EUC_JP);
      }
      DBUG_RETURN(GRN_ENC_NONE);
    }
  }
}
