/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2013  Kouhei Sutou <kou@clear-code.com>

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

#include "mrn_normalizer_finder.hpp"

// for debug
#define MRN_CLASS_NAME "mrn::NormalizerFinder"

namespace mrn {
  NormalizerFinder::NormalizerFinder(grn_ctx *ctx, THD *thread)
    : ctx_(ctx),
      thread_(thread) {
  }

  NormalizerFinder::~NormalizerFinder() {
  }

  grn_obj *NormalizerFinder::find(Field *field) {
    MRN_DBUG_ENTER_METHOD();

    const CHARSET_INFO *charset_info = field->charset();
    const char *normalizer_name = NULL;
    const char *default_normalizer_name = "NormalizerAuto";
    if ((strcmp(charset_info->name, "utf8_general_ci") == 0) ||
        (strcmp(charset_info->name, "utf8mb4_general_ci") == 0)) {
      normalizer_name = "NormalizerMySQLGeneralCI";
    } else if ((strcmp(charset_info->name, "utf8_unicode_ci") == 0) ||
               (strcmp(charset_info->name, "utf8mb4_unicode_ci") == 0)) {
      normalizer_name = "NormalizerMySQLUnicodeCI";
    }

    grn_obj *normalizer = NULL;
    if (normalizer_name) {
      normalizer = grn_ctx_get(ctx_, normalizer_name, -1);
      if (!normalizer) {
        char error_message[MRN_MESSAGE_BUFFER_SIZE];
        snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
                 "%s normalizer isn't found for %s. "
                 "Install groonga-normalizer-mysql normalizer. "
                 "%s is used as fallback.",
                 normalizer_name,
                 charset_info->name,
                 default_normalizer_name);
        push_warning(thread_, Sql_condition::WARN_LEVEL_WARN,
                     HA_ERR_UNSUPPORTED, error_message);
      }
    }

    if (!normalizer) {
      normalizer = grn_ctx_get(ctx_, default_normalizer_name, -1);
    }

    DBUG_RETURN(normalizer);
  }
}
