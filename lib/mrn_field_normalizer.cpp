/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2013-2018  Kouhei Sutou <kou@clear-code.com>
  Copyright(C) 2021  Horimoto Yasuhiro <horimoto@clear-code.com>

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

#include "mrn_field_normalizer.hpp"
#include "mrn_encoding.hpp"

#include <field.h>

// for debug
#define MRN_CLASS_NAME "mrn::FieldNormalizer"

namespace mrn {
  FieldNormalizer::FieldNormalizer(grn_ctx *ctx, THD *thread, Field *field)
    : ctx_(ctx),
      thread_(thread),
      field_(field),
      lexicon_(NULL) {
  }

  FieldNormalizer::~FieldNormalizer() {
    if (lexicon_) {
      grn_obj_close(ctx_, lexicon_);
    }
  }

  bool FieldNormalizer::should_normalize() {
    MRN_DBUG_ENTER_METHOD();

    DBUG_PRINT("info",
               ("mroonga: result_type = %u", field_->result_type()));
    DBUG_PRINT("info",
               ("mroonga: charset->name = %s", MRN_CHARSET_NAME(field_->charset())));
    DBUG_PRINT("info",
               ("mroonga: charset->csname = %s", MRN_CHARSET_CSNAME(field_->charset())));
    DBUG_PRINT("info",
               ("mroonga: charset->state = %u", field_->charset()->state));
    bool need_normalize_p;
    if (field_->charset()->state & (MY_CS_BINSORT | MY_CS_CSSORT)) {
      need_normalize_p = false;
      DBUG_PRINT("info",
                 ("mroonga: should_normalize: false: sort is required"));
    } else {
      if (is_text_type()) {
        need_normalize_p = true;
        DBUG_PRINT("info", ("mroonga: should_normalize: true: text type"));
      } else {
        need_normalize_p = false;
        DBUG_PRINT("info", ("mroonga: should_normalize: false: no text type"));
      }
    }

    DBUG_RETURN(need_normalize_p);
  }

  bool FieldNormalizer::is_text_type() {
    MRN_DBUG_ENTER_METHOD();
    bool text_type_p;
    switch (field_->type()) {
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_VAR_STRING:
      text_type_p = true;
      break;
    case MYSQL_TYPE_STRING:
      switch (field_->real_type()) {
      case MYSQL_TYPE_ENUM:
      case MYSQL_TYPE_SET:
        text_type_p = false;
        break;
      default:
        text_type_p = true;
        break;
      }
      break;
    default:
      text_type_p = false;
      break;
    }
    DBUG_RETURN(text_type_p);
  }

  grn_obj *FieldNormalizer::normalize(const char *string,
                                      unsigned int string_length) {
    MRN_DBUG_ENTER_METHOD();
    grn_obj normalizer;
    GRN_TEXT_INIT(&normalizer, 0);
    find_grn_normalizer(&normalizer);
    int flags = 0;
    grn_encoding original_encoding = GRN_CTX_GET_ENCODING(ctx_);
    encoding::set_raw(ctx_, field_->charset());
    grn_obj *grn_string;
    if (GRN_TEXT_VALUE(&normalizer)[GRN_TEXT_LEN(&normalizer) - 1] == ')') {
      if (!lexicon_) {
        lexicon_ = grn_table_create(ctx_,
                                    NULL, 0,
                                    NULL,
                                    GRN_OBJ_TABLE_PAT_KEY,
                                    grn_ctx_at(ctx_, GRN_DB_SHORT_TEXT),
                                    NULL);
      }
      grn_obj_set_info(ctx_, lexicon_, GRN_INFO_NORMALIZER, &normalizer);
      grn_string = grn_string_open(ctx_,
                                   string,
                                   string_length,
                                   lexicon_,
                                   flags);
    } else {
      grn_string = grn_string_open(ctx_,
                                   string,
                                   string_length,
                                   grn_ctx_get(ctx_,
                                               GRN_TEXT_VALUE(&normalizer),
                                               GRN_TEXT_LEN(&normalizer)),
                                   flags);
    }
    GRN_OBJ_FIN(ctx_, &normalizer);
    GRN_CTX_SET_ENCODING(ctx_, original_encoding);
    DBUG_RETURN(grn_string);
  }

  void FieldNormalizer::find_grn_normalizer(grn_obj *normalizer) {
    MRN_DBUG_ENTER_METHOD();

    const CHARSET_INFO *charset_info = field_->charset();
    const char *normalizer_name = NULL;
    const char *normalizer_spec = NULL;
    const char *default_normalizer_name = "NormalizerAuto";
    if ((strcmp(MRN_CHARSET_NAME(charset_info), "utf8_general_ci") == 0) ||
        (strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb3_general_ci") == 0) ||
        (strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb4_general_ci") == 0)) {
      normalizer_name = normalizer_spec = "NormalizerMySQLGeneralCI";
    } else if ((strcmp(MRN_CHARSET_NAME(charset_info), "utf8_unicode_ci") == 0) ||
               (strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb3_unicode_ci") == 0) ||
               (strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb4_unicode_ci") == 0)) {
      normalizer_name = normalizer_spec = "NormalizerMySQLUnicodeCI";
    } else if ((strcmp(MRN_CHARSET_NAME(charset_info), "utf8_unicode_520_ci") == 0) ||
               (strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb3_unicode_520_ci") == 0) ||
               (strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb4_unicode_520_ci") == 0)) {
      normalizer_name = normalizer_spec = "NormalizerMySQLUnicode520CI";
    } else if ((strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb4_0900_ai_ci") == 0)) {
      normalizer_name = "NormalizerMySQLUnicode900";
      normalizer_spec = "NormalizerMySQLUnicode900('weight_level', 1)";
    } else if ((strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb4_0900_as_ci") == 0)) {
      normalizer_name = "NormalizerMySQLUnicode900";
      normalizer_spec = "NormalizerMySQLUnicode900('weight_level', 2)";
    } else if ((strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb4_0900_as_cs") == 0)) {
      normalizer_name = "NormalizerMySQLUnicode900";
      normalizer_spec = "NormalizerMySQLUnicode900('weight_level', 3)";
    } else if ((strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb4_ja_0900_as_cs") == 0)) {
      normalizer_name = "NormalizerMySQLUnicode900";
      normalizer_spec =
        "NormalizerMySQLUnicode900('locale', 'ja', 'weight_level', 3)";
    } else if ((strcmp(MRN_CHARSET_NAME(charset_info), "utf8mb4_ja_0900_as_cs_ks") == 0)) {
      normalizer_name = "NormalizerMySQLUnicode900";
      normalizer_spec =
        "NormalizerMySQLUnicode900('locale', 'ja', 'weight_level', 4)";
    }

    if (normalizer_name) {
      if (!grn_ctx_get(ctx_, normalizer_name, -1)) {
        char error_message[MRN_MESSAGE_BUFFER_SIZE];
        snprintf(error_message, MRN_MESSAGE_BUFFER_SIZE,
                 "%s normalizer isn't found for %s. "
                 "Install groonga-normalizer-mysql normalizer. "
                 "%s is used as fallback.",
                 normalizer_name,
                 MRN_CHARSET_NAME(charset_info),
                 default_normalizer_name);
        push_warning(thread_, MRN_SEVERITY_WARNING,
                     HA_ERR_UNSUPPORTED, error_message);
        normalizer_name = NULL;
      }
    }
    if (!normalizer_name) {
      normalizer_name = normalizer_spec = default_normalizer_name;
    }
    GRN_TEXT_PUTS(ctx_, normalizer, normalizer_spec);

    DBUG_VOID_RETURN;
  }
}
