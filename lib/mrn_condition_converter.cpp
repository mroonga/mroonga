/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2013-2021  Sutou Kouhei <kou@clear-code.com>
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

#include "mrn_condition_converter.hpp"
#include "mrn_encoding.hpp"
#include "mrn_query_parser.hpp"
#include "mrn_time_converter.hpp"

// for debug
#define MRN_CLASS_NAME "mrn::ConditionConverter"

#ifdef MRN_ITEM_HAVE_ITEM_NAME
#  define MRN_ITEM_FIELD_GET_NAME(item)        ((item)->item_name.ptr())
#  define MRN_ITEM_FIELD_GET_NAME_LENGTH(item)  \
  (static_cast<int>(item->item_name.length()))
#else
#  ifdef MRN_ITEM_ITEM_NAME_IS_LEX_STRING
#    define MRN_ITEM_FIELD_GET_NAME(item)        ((item)->name.str)
#    define MRN_ITEM_FIELD_GET_NAME_LENGTH(item)        \
  (static_cast<int>((item)->name.length))
#  else
#    define MRN_ITEM_FIELD_GET_NAME(item)        ((item)->name)
#    define MRN_ITEM_FIELD_GET_NAME_LENGTH(item)        \
  (static_cast<int>(strlen((item)->name)))
#  endif
#endif

namespace mrn {
  ConditionConverter::ConditionConverter(THD *thread,
                                         grn_ctx *ctx,
                                         grn_obj *table,
                                         grn_obj **index_columns,
                                         KEY *key_infos,
                                         bool is_storage_mode)
    : thread_(thread),
      ctx_(ctx),
      table_(table),
      index_columns_(index_columns),
      key_infos_(key_infos),
      is_storage_mode_(is_storage_mode) {
    GRN_TEXT_INIT(&column_name_, 0);
    GRN_VOID_INIT(&value_);
  }

  ConditionConverter::~ConditionConverter() {
    grn_obj_unlink(ctx_, &column_name_);
    grn_obj_unlink(ctx_, &value_);
  }

  bool ConditionConverter::is_convertable(const Item *item,
                                          grn_operator logical_operator) {
    MRN_DBUG_ENTER_METHOD();

    if (!item) {
      DBUG_RETURN(false);
    }

    std::vector<grn_encoding> encodings;
    bool convertable = is_convertable(item,
                                      logical_operator,
                                      encodings);
    DBUG_RETURN(convertable);
  }

  bool ConditionConverter::is_convertable(const Item *item,
                                          grn_operator logical_operator,
                                          std::vector<grn_encoding> &encodings) {
    MRN_DBUG_ENTER_METHOD();

    switch (item->type()) {
    case Item::COND_ITEM:
      {
        const Item_cond *cond_item = reinterpret_cast<const Item_cond *>(item);
        bool convertable = is_convertable(cond_item,
                                          logical_operator,
                                          encodings);
        if (convertable) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][true] "
                  "convertable conditions");
        }
        DBUG_RETURN(convertable);
      }
      break;
    case Item::FUNC_ITEM:
      {
        const Item_func *func_item = reinterpret_cast<const Item_func *>(item);
        bool convertable = is_convertable(func_item,
                                          logical_operator,
                                          encodings);
        if (convertable) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][true] "
                  "convertable function condition: %u",
                  func_item->functype());
        }
        DBUG_RETURN(convertable);
      }
      break;
    default:
      GRN_LOG(ctx_, GRN_LOG_DEBUG,
              "[mroonga][condition-push-down][false] "
              "unsupported top level conditionnot only one item: %u",
              item->type());
      DBUG_RETURN(false);
      break;
    }

    DBUG_RETURN(false);
  }

  bool ConditionConverter::is_convertable(const Item_cond *cond_item,
                                          grn_operator logical_operator,
                                          std::vector<grn_encoding> &encodings) {
    MRN_DBUG_ENTER_METHOD();

    if (!is_storage_mode_) {
      DBUG_RETURN(false);
    }

    grn_operator sub_logical_operator;
    if (cond_item->functype() == Item_func::COND_AND_FUNC) {
      sub_logical_operator = GRN_OP_AND;
    } else if (cond_item->functype() == Item_func::COND_OR_FUNC) {
      sub_logical_operator = GRN_OP_OR;
    } else {
      GRN_LOG(ctx_, GRN_LOG_DEBUG,
              "[mroonga][condition-push-down][false] "
              "not AND nor OR conditions: %u",
              cond_item->functype());
      DBUG_RETURN(false);
    }

    List<Item> *argument_list =
      const_cast<Item_cond *>(cond_item)->argument_list();
    List_iterator<Item> iterator(*argument_list);
    const Item *sub_item;
    while ((sub_item = iterator++)) {
      if (!is_convertable(sub_item, sub_logical_operator, encodings)) {
        DBUG_RETURN(false);
      }
    }

    DBUG_RETURN(true);
  }

  bool ConditionConverter::is_convertable(const Item_func *func_item,
                                          grn_operator logical_operator,
                                          std::vector<grn_encoding> &encodings) {
    MRN_DBUG_ENTER_METHOD();

    switch (func_item->functype()) {
    case Item_func::EQ_FUNC:
    case Item_func::LT_FUNC:
    case Item_func::LE_FUNC:
    case Item_func::GE_FUNC:
    case Item_func::GT_FUNC:
      if (!is_storage_mode_) {
        DBUG_RETURN(false);
      }
      {
        Item **arguments = func_item->arguments();
        Item *left_item = arguments[0];
        Item *right_item = arguments[1];
        if (left_item->type() != Item::FIELD_ITEM) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "left item of binary operation isn't field: %u",
                  left_item->type());
          DBUG_RETURN(false);
        }
        if (!right_item->basic_const_item()) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "right item of binary operation isn't constant: %u",
                  right_item->type());
          DBUG_RETURN(false);
        }

        bool convertable =
          is_convertable_binary_operation(static_cast<Item_field *>(left_item),
                                          right_item,
                                          func_item->functype(),
                                          encodings);
        DBUG_RETURN(convertable);
      }
      break;
    case Item_func::FT_FUNC:
      {
        const Item_func_match *match_item =
          static_cast<const Item_func_match *>(func_item);
        KEY *key_info = &(key_infos_[match_item->key]);
        uint n_key_parts = KEY_N_KEY_PARTS(key_info);
        for (uint i = 0; i < n_key_parts; ++i) {
          Field *field = key_info->key_part[i].field;
          grn_encoding encoding = encoding::convert(field->charset());
          if (!encodings.empty() && encodings[0] != encoding) {
            DBUG_RETURN(false);
          }
          encodings.push_back(encoding);
        }
      }
      DBUG_RETURN(true);
      break;
    case Item_func::BETWEEN:
      if (!is_storage_mode_) {
        DBUG_RETURN(false);
      }
      {
        Item **arguments = func_item->arguments();
        Item *target_item = arguments[0];
        Item *min_item = arguments[1];
        Item *max_item = arguments[2];
        if (target_item->type() != Item::FIELD_ITEM) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "target of BETWEEN isn't field: %u",
                  target_item->type());
          DBUG_RETURN(false);
        }
        if (!min_item->basic_const_item()) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "minimum value of BETWEEN isn't constant: %u",
                  min_item->type());
          DBUG_RETURN(false);
        }
        if (!max_item->basic_const_item()) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "maximum value of BETWEEN isn't constant: %u",
                  max_item->type());
          DBUG_RETURN(false);
        }

        bool convertable =
          is_convertable_between(static_cast<Item_field *>(target_item),
                                 min_item,
                                 max_item);
        DBUG_RETURN(convertable);
      }
    case Item_func::IN_FUNC:
      if (!is_storage_mode_) {
        DBUG_RETURN(false);
      }
      {
        const Item_func_in *in_item =
          static_cast<const Item_func_in *>(func_item);
        if (in_item->negated && logical_operator == GRN_OP_OR) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] OR NOT IN");
          DBUG_RETURN(false);
        }

        Item **arguments = func_item->arguments();
        Item *target_item = arguments[0];
        if (target_item->type() != Item::FIELD_ITEM) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "target of IN isn't field: %u",
                  target_item->type());
          DBUG_RETURN(false);
        }
        Item_field *field_item = static_cast<Item_field *>(target_item);
        uint n_arguments = func_item->argument_count();
        for (uint i = 1; i < n_arguments; ++i) {
          Item *value_item = arguments[i];
          if (!value_item->basic_const_item()) {
            GRN_LOG(ctx_, GRN_LOG_DEBUG,
                    "[mroonga][condition-push-down][false] "
                    "%uth value of IN isn't constant: %.*s: %u",
                    i - 1,
                    MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                    MRN_ITEM_FIELD_GET_NAME(field_item),
                    value_item->type());
            DBUG_RETURN(false);
          }
        }

        bool convertable = is_convertable_in(field_item,
                                             arguments + 1,
                                             n_arguments - 1);
        DBUG_RETURN(convertable);
      }
    // TODO
    // case Item_func::NOT_FUNC:
    //   DBUG_RETURN(true);
    //   break;
    default:
      DBUG_RETURN(false);
      break;
    }

    DBUG_RETURN(true);
  }

  bool ConditionConverter::is_convertable_binary_operation(
    const Item_field *field_item,
    Item *value_item,
    Item_func::Functype func_type,
    std::vector<grn_encoding> &encodings) {
    MRN_DBUG_ENTER_METHOD();

    enum_field_types field_type = field_item->field->real_type();
    NormalizedType normalized_type = normalize_field_type(field_type);
    switch (normalized_type) {
    case STRING_TYPE:
      if (func_type == Item_func::EQ_FUNC) {
        grn_encoding encoding = encoding::convert(field_item->field->charset());
        if (!encodings.empty() && encodings[0] != encoding) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "string equal operation for "
                  "mixed charset isn't supported: <%.*s>: <%s>",
                  MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                  MRN_ITEM_FIELD_GET_NAME(field_item),
                  MRN_CHARSET_CSNAME(field_item->field->charset()));
          DBUG_RETURN(false);
        }
        encodings.push_back(encoding);
        if (!have_index(field_item, GRN_OP_EQUAL)) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "index for string equal operation doesn't exist: <%.*s>",
                  MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                  MRN_ITEM_FIELD_GET_NAME(field_item));
          DBUG_RETURN(false);
        }
      }
      break;
    case INT_TYPE:
      if (field_type == MYSQL_TYPE_ENUM) {
        if (!MRN_ITEM_IS_STRING_TYPE(value_item) &&
            !MRN_ITEM_IS_INT_TYPE(value_item)) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "constant value of enum binary operation "
                  "isn't string nor integer: <%.*s>: <%u>",
                  MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                  MRN_ITEM_FIELD_GET_NAME(field_item),
                  value_item->type());
          DBUG_RETURN(false);
        }
      }
      break;
    case TIME_TYPE:
      if (!is_valid_time_value(field_item, value_item)) {
        GRN_LOG(ctx_, GRN_LOG_DEBUG,
                "[mroonga][condition-push-down][false] "
                "constant value of time binary operation "
                "is invalid: <%.*s>: <%u>",
                MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                MRN_ITEM_FIELD_GET_NAME(field_item),
                value_item->type());
        DBUG_RETURN(false);
      }
      break;
    case UNSUPPORTED_TYPE:
      GRN_LOG(ctx_, GRN_LOG_DEBUG,
              "[mroonga][condition-push-down][false] "
              "unsupported value of binary operation: <%.*s>: <%u>",
              MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
              MRN_ITEM_FIELD_GET_NAME(field_item),
              field_type);
      DBUG_RETURN(false);
      break;
    }

    DBUG_RETURN(true);
  }

  bool ConditionConverter::is_convertable_between(const Item_field *field_item,
                                                  Item *min_item,
                                                  Item *max_item) {
    MRN_DBUG_ENTER_METHOD();

    enum_field_types field_type = field_item->field->type();
    NormalizedType normalized_type = normalize_field_type(field_type);
    switch (normalized_type) {
    case STRING_TYPE:
      if (!have_index(field_item, GRN_OP_LESS)) {
        GRN_LOG(ctx_, GRN_LOG_DEBUG,
                "[mroonga][condition-push-down][false] "
                "index for string BETWEEN operation doesn't exist: %.*s",
                MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                MRN_ITEM_FIELD_GET_NAME(field_item));
        DBUG_RETURN(false);
      }
      break;
    case INT_TYPE:
      if (field_type == MYSQL_TYPE_ENUM) {
        if (!MRN_ITEM_IS_STRING_TYPE(min_item) &&
            !MRN_ITEM_IS_INT_TYPE(min_item)) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "minimum value of enum BETWEEN operation "
                  "isn't string nor integer: %.*s: %u",
                  MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                  MRN_ITEM_FIELD_GET_NAME(field_item),
                  min_item->type());
          DBUG_RETURN(false);
        }
        if (!MRN_ITEM_IS_STRING_TYPE(max_item) &&
            !MRN_ITEM_IS_INT_TYPE(max_item)) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "maximum value of enum BETWEEN operation "
                  "isn't string nor integer: %.*s: %u",
                  MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                  MRN_ITEM_FIELD_GET_NAME(field_item),
                  max_item->type());
          DBUG_RETURN(false);
        }
      }
      break;
    case TIME_TYPE:
      if (!is_valid_time_value(field_item, min_item)) {
        GRN_LOG(ctx_, GRN_LOG_DEBUG,
                "[mroonga][condition-push-down][false] "
                "minimum value of time BETWEEN operation is invalid: "
                "%.*s: %u",
                MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                MRN_ITEM_FIELD_GET_NAME(field_item),
                min_item->type());
        DBUG_RETURN(false);
      }
      if (!is_valid_time_value(field_item, max_item)) {
        GRN_LOG(ctx_, GRN_LOG_DEBUG,
                "[mroonga][condition-push-down][false] "
                "maximum value of time BETWEEN operation is invalid: "
                "%.*s: %u",
                MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                MRN_ITEM_FIELD_GET_NAME(field_item),
                max_item->type());
        DBUG_RETURN(false);
      }
      break;
    case UNSUPPORTED_TYPE:
      GRN_LOG(ctx_, GRN_LOG_DEBUG,
              "[mroonga][condition-push-down][false] "
              "unsupported type of BETWEEN operation: %.*s: %u",
              MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
              MRN_ITEM_FIELD_GET_NAME(field_item),
              field_type);
      DBUG_RETURN(false);
      break;
    }

    DBUG_RETURN(true);
  }

  bool ConditionConverter::is_convertable_in(const Item_field *field_item,
                                             Item **value_items,
                                             uint n_value_items) {
    MRN_DBUG_ENTER_METHOD();


    enum_field_types field_type = field_item->field->type();
    NormalizedType normalized_type = normalize_field_type(field_type);

    if (normalized_type == UNSUPPORTED_TYPE) {
      GRN_LOG(ctx_, GRN_LOG_DEBUG,
              "[mroonga][condition-push-down][false] "
              "unsupported type of IN operation: %.*s: %u",
              MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
              MRN_ITEM_FIELD_GET_NAME(field_item),
              field_type);
      DBUG_RETURN(false);
    }

    for (uint i = 0; i < n_value_items; ++i) {
      Item *value_item = value_items[i];
      switch (normalized_type) {
      case STRING_TYPE:
        if (!have_index(field_item, GRN_OP_EQUAL)) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "index for string IN operation doesn't exist: %.*s",
                  MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                  MRN_ITEM_FIELD_GET_NAME(field_item));
        }
        break;
      case INT_TYPE:
        if (field_type == MYSQL_TYPE_ENUM) {
          if (!MRN_ITEM_IS_STRING_TYPE(value_item) &&
              !MRN_ITEM_IS_INT_TYPE(value_item)) {
            GRN_LOG(ctx_, GRN_LOG_DEBUG,
                    "[mroonga][condition-push-down][false] "
                    "constant value of enum IN operation "
                    "isn't string nor integer: %.*s: %u",
                    MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                    MRN_ITEM_FIELD_GET_NAME(field_item),
                    value_item->type());
            DBUG_RETURN(false);
          }
        }
        break;
      case TIME_TYPE:
        if (!is_valid_time_value(field_item, value_item)) {
          GRN_LOG(ctx_, GRN_LOG_DEBUG,
                  "[mroonga][condition-push-down][false] "
                  "%uth value of time IN operation is invalid: "
                  "%.*s: %u",
                  i,
                  MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item),
                  MRN_ITEM_FIELD_GET_NAME(field_item),
                  value_item->type());
          DBUG_RETURN(false);
        }
        break;
      default:
        break;
      }
    }

    DBUG_RETURN(true);
  }

  bool ConditionConverter::is_valid_time_value(const Item_field *field_item,
                                               Item *value_item) {
    MRN_DBUG_ENTER_METHOD();

    MYSQL_TIME mysql_time;
    bool error = get_time_value(field_item, value_item, &mysql_time);

    DBUG_RETURN(!error);
  }

  bool ConditionConverter::get_time_value(const Item_field *field_item,
                                          Item *value_item,
                                          MYSQL_TIME *mysql_time) {
    MRN_DBUG_ENTER_METHOD();

    bool error;
    Item *real_value_item = value_item->real_item();
    switch (field_item->field->type()) {
    case MYSQL_TYPE_TIME:
      error = MRN_ITEM_GET_TIME(real_value_item, mysql_time, thread_);
      break;
    case MYSQL_TYPE_YEAR:
      mysql_time->year        = static_cast<int>(value_item->val_int());
      mysql_time->month       = 1;
      mysql_time->day         = 1;
      mysql_time->hour        = 0;
      mysql_time->hour        = 0;
      mysql_time->minute      = 0;
      mysql_time->second_part = 0;
      mysql_time->neg         = false;
      mysql_time->time_type   = MYSQL_TIMESTAMP_DATE;
      error = false;
      break;
    default:
      error = MRN_ITEM_GET_DATE_FUZZY(real_value_item, mysql_time, thread_);
      break;
    }

    DBUG_RETURN(error);
  }

  ConditionConverter::NormalizedType
  ConditionConverter::normalize_field_type(enum_field_types field_type) {
    MRN_DBUG_ENTER_METHOD();

    NormalizedType type = UNSUPPORTED_TYPE;

    switch (field_type) {
    case MYSQL_TYPE_DECIMAL:
      type = STRING_TYPE;
      break;
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
      type = INT_TYPE;
      break;
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      type = UNSUPPORTED_TYPE;
      break;
    case MYSQL_TYPE_NULL:
      type = UNSUPPORTED_TYPE;
      break;
    case MYSQL_TYPE_TIMESTAMP:
      type = TIME_TYPE;
      break;
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_INT24:
      type = INT_TYPE;
      break;
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_NEWDATE:
      type = TIME_TYPE;
      break;
    case MYSQL_TYPE_VARCHAR:
#ifdef MRN_HAVE_MYSQL_TYPE_VARCHAR_COMPRESSED
    case MYSQL_TYPE_VARCHAR_COMPRESSED:
#endif
      type = STRING_TYPE;
      break;
    case MYSQL_TYPE_BIT:
      type = INT_TYPE;
      break;
#ifdef MRN_HAVE_MYSQL_TYPE_TIMESTAMP2
    case MYSQL_TYPE_TIMESTAMP2:
      type = TIME_TYPE;
      break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_DATETIME2
    case MYSQL_TYPE_DATETIME2:
      type = TIME_TYPE;
      break;
#endif
#ifdef MRN_HAVE_MYSQL_TYPE_TIME2
    case MYSQL_TYPE_TIME2:
      type = TIME_TYPE;
      break;
#endif
    case MYSQL_TYPE_NEWDECIMAL:
      type = STRING_TYPE;
      break;
    case MYSQL_TYPE_ENUM:
      type = INT_TYPE;
      break;
    case MYSQL_TYPE_SET:
      type = INT_TYPE;
      break;
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
#ifdef MRN_HAVE_MYSQL_TYPE_BLOB_COMPRESSED
    case MYSQL_TYPE_BLOB_COMPRESSED:
#endif
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
      type = STRING_TYPE;
      break;
    case MYSQL_TYPE_GEOMETRY:
      type = UNSUPPORTED_TYPE;
      break;
#ifdef MRN_HAVE_MYSQL_TYPE_JSON
    case MYSQL_TYPE_JSON:
      type = STRING_TYPE;
      break;
#endif
    }

    DBUG_RETURN(type);
  }

  bool ConditionConverter::have_index(const Item_field *field_item,
                                      grn_operator _operator) {
    MRN_DBUG_ENTER_METHOD();

    grn_obj *column;
    column = grn_obj_column(ctx_, table_,
                            MRN_ITEM_FIELD_GET_NAME(field_item),
                            MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item));
    if (!column) {
      DBUG_RETURN(false);
    }
    mrn::SmartGrnObj smart_column(ctx_, column);

    int n_indexes = grn_column_index(ctx_, column, _operator, NULL, 0, NULL);
    bool convertable = (n_indexes > 0);

    DBUG_RETURN(convertable);
  }

  bool ConditionConverter::have_index(const Item_field *field_item,
                                      Item_func::Functype func_type) {
    MRN_DBUG_ENTER_METHOD();

    bool have = false;
    switch (func_type) {
    case Item_func::EQ_FUNC:
      have = have_index(field_item, GRN_OP_EQUAL);
      break;
    case Item_func::LT_FUNC:
      have = have_index(field_item, GRN_OP_LESS);
      break;
    case Item_func::LE_FUNC:
      have = have_index(field_item, GRN_OP_LESS_EQUAL);
      break;
    case Item_func::GE_FUNC:
      have = have_index(field_item, GRN_OP_GREATER_EQUAL);
      break;
    case Item_func::GT_FUNC:
      have = have_index(field_item, GRN_OP_GREATER);
      break;
    default:
      break;
    }

    DBUG_RETURN(have);
  }

  unsigned int ConditionConverter::count_match_against(const Item *item) {
    MRN_DBUG_ENTER_METHOD();

    if (!item) {
      DBUG_RETURN(0);
    }

    switch (item->type()) {
    case Item::COND_ITEM:
      if (is_storage_mode_) {
        Item_cond *cond_item = (Item_cond *)item;
        if (cond_item->functype() == Item_func::COND_AND_FUNC ||
            cond_item->functype() == Item_func::COND_OR_FUNC) {
          unsigned int n_match_againsts = 0;
          List_iterator<Item> iterator(*((cond_item)->argument_list()));
          const Item *sub_item;
          while ((sub_item = iterator++)) {
            n_match_againsts += count_match_against(sub_item);
          }
          DBUG_RETURN(n_match_againsts);
        }
      }
      break;
    case Item::FUNC_ITEM:
      {
        const Item_func *func_item = (const Item_func *)item;
        switch (func_item->functype()) {
        case Item_func::FT_FUNC:
          DBUG_RETURN(1);
          break;
        default:
          break;
        }
      }
      break;
    default:
      break;
    }

    DBUG_RETURN(0);
  }

  grn_encoding ConditionConverter::convert(
    const Item *where,
    grn_obj *expression,
    std::list<SmartGrnObj> &match_columns_list) {
    MRN_DBUG_ENTER_METHOD();

    if (!where) {
      DBUG_RETURN(GRN_ENC_NONE);
    }

    std::vector<grn_encoding> encodings;
    switch (where->type()) {
    case Item::COND_ITEM:
      convert(static_cast<const Item_cond *>(where),
              expression,
              match_columns_list,
              encodings);
      break;
    case Item::FUNC_ITEM:
      convert(static_cast<const Item_func *>(where),
              expression,
              match_columns_list,
              encodings,
              false);
      break;
    default:
      break;
    }

    grn_encoding encoding = GRN_ENC_NONE;
    if (!encodings.empty()) {
      encoding = encodings[0];
    }
    DBUG_RETURN(encoding);
  }

  bool ConditionConverter::convert(
    const Item_cond *cond_item,
    grn_obj *expression,
    std::list<SmartGrnObj> &match_columns_list,
    std::vector<grn_encoding> &encodings) {
    MRN_DBUG_ENTER_METHOD();

    grn_operator logical_operator = GRN_OP_AND;
    if (cond_item->functype() == Item_func::COND_OR_FUNC) {
      logical_operator = GRN_OP_OR;
    }

    List<Item> *sub_item_list =
      const_cast<Item_cond *>(cond_item)->argument_list();
    List_iterator<Item> iterator(*sub_item_list);
    const Item *sub_item;
    int n_conditions = 0;
    while ((sub_item = iterator++)) {
      switch (sub_item->type()) {
      case Item::COND_ITEM:
        if (convert(static_cast<const Item_cond *>(sub_item),
                    expression,
                    match_columns_list,
                    encodings)) {
          if (n_conditions > 0) {
            grn_expr_append_op(ctx_, expression, logical_operator, 2);
          }
          ++n_conditions;
        }
        break;
      case Item::FUNC_ITEM:
        {
          const Item_func *sub_func_item =
            static_cast<const Item_func *>(sub_item);
          if (convert(sub_func_item,
                      expression,
                      match_columns_list,
                      encodings,
                      n_conditions > 0)) {
            if (n_conditions > 0) {
              grn_operator sub_logical_operator = logical_operator;
              if (sub_logical_operator == GRN_OP_AND &&
                  sub_func_item->functype() == Item_func::IN_FUNC) {
                const Item_func_in *sub_in_item =
                  static_cast<const Item_func_in *>(sub_func_item);
                if (sub_in_item->negated) {
                  sub_logical_operator = GRN_OP_AND_NOT;
                }
              }
              grn_expr_append_op(ctx_, expression, sub_logical_operator, 2);
            }
            ++n_conditions;
          }
        }
        break;
      default:
        break;
      }
    }

    DBUG_RETURN(n_conditions > 0);
  }

  bool ConditionConverter::convert(
    const Item_func *func_item,
    grn_obj *expression,
    std::list<SmartGrnObj> &match_columns_list,
    std::vector<grn_encoding> &encodings,
    bool have_condition) {
    MRN_DBUG_ENTER_METHOD();

    bool added = false;

    switch (func_item->functype()) {
    case Item_func::EQ_FUNC:
      added = convert_binary_operation(func_item, expression, GRN_OP_EQUAL);
      break;
    case Item_func::LT_FUNC:
      added = convert_binary_operation(func_item, expression, GRN_OP_LESS);
      break;
    case Item_func::LE_FUNC:
      added = convert_binary_operation(func_item, expression, GRN_OP_LESS_EQUAL);
      break;
    case Item_func::GE_FUNC:
      added = convert_binary_operation(func_item, expression,
                                       GRN_OP_GREATER_EQUAL);
      break;
    case Item_func::GT_FUNC:
      added = convert_binary_operation(func_item, expression, GRN_OP_GREATER);
      break;
    case Item_func::BETWEEN:
      added = convert_between(func_item, expression);
      break;
    case Item_func::IN_FUNC:
      added = convert_in(func_item, expression, have_condition);
      break;
    case Item_func::FT_FUNC:
      added = convert_full_text_search(func_item,
                                       expression,
                                       match_columns_list,
                                       encodings);
    default:
      break;
    }

    DBUG_RETURN(added);
  }

  bool ConditionConverter::convert_binary_operation(const Item_func *func_item,
                                                    grn_obj *expression,
                                                    grn_operator _operator) {
    MRN_DBUG_ENTER_METHOD();

    Item **arguments = func_item->arguments();
    Item *left_item = arguments[0];
    Item *right_item = arguments[1];
    if (left_item->type() == Item::FIELD_ITEM) {
      const Item_field *field_item = static_cast<const Item_field *>(left_item);
      append_field_value(field_item, expression);
      append_const_item(field_item, right_item, expression);
      grn_expr_append_op(ctx_, expression, _operator, 2);
      DBUG_RETURN(true);
    } else {
      DBUG_RETURN(false);
    }
  }

  bool ConditionConverter::convert_between(const Item_func *func_item,
                                           grn_obj *expression) {
    MRN_DBUG_ENTER_METHOD();

    Item **arguments = func_item->arguments();
    Item *target_item = arguments[0];
    Item *min_item = arguments[1];
    Item *max_item = arguments[2];

    grn_obj *between_func = grn_ctx_get(ctx_, "between", strlen("between"));
    grn_expr_append_obj(ctx_, expression, between_func, GRN_OP_PUSH, 1);

    const Item_field *field_item = static_cast<const Item_field *>(target_item);
    append_field_value(field_item, expression);

    grn_obj include;
    mrn::SmartGrnObj smart_include(ctx_, &include);
    GRN_TEXT_INIT(&include, 0);
    GRN_TEXT_PUTS(ctx_, &include, "include");
    append_const_item(field_item, min_item, expression);
    grn_expr_append_const(ctx_, expression, &include, GRN_OP_PUSH, 1);
    append_const_item(field_item, max_item, expression);
    grn_expr_append_const(ctx_, expression, &include, GRN_OP_PUSH, 1);

    grn_expr_append_op(ctx_, expression, GRN_OP_CALL, 5);

    DBUG_RETURN(true);
  }

  bool ConditionConverter::convert_in(const Item_func *func_item,
                                      grn_obj *expression,
                                      bool have_condition) {
    MRN_DBUG_ENTER_METHOD();

    const Item_func_in *in_item = static_cast<const Item_func_in *>(func_item);

    if (!have_condition && in_item->negated) {
      grn_expr_append_const_bool(ctx_, expression, GRN_TRUE, GRN_OP_PUSH, 1);
    }

    Item **arguments = func_item->arguments();
    Item *target_item = arguments[0];

    grn_obj *in_values_func = grn_ctx_get(ctx_, "in_values", -1);
    grn_expr_append_obj(ctx_, expression, in_values_func, GRN_OP_PUSH, 1);

    const Item_field *field_item = static_cast<const Item_field *>(target_item);
    append_field_value(field_item, expression);

    uint n_arguments = func_item->argument_count();
    for (uint i = 1; i < n_arguments; ++i) {
      Item *value_item = arguments[i];
      append_const_item(field_item, value_item, expression);
    }

    grn_expr_append_op(ctx_, expression, GRN_OP_CALL, n_arguments);

    if (!have_condition && in_item->negated) {
      grn_expr_append_op(ctx_, expression, GRN_OP_AND_NOT, 2);
    }

    DBUG_RETURN(true);
  }

  bool ConditionConverter::convert_full_text_search(
    const Item_func *func_item,
    grn_obj *expression,
    std::list<SmartGrnObj> &match_columns_list,
    std::vector<grn_encoding> &encodings) {
    MRN_DBUG_ENTER_METHOD();

    const Item_func_match *match_item =
      static_cast<const Item_func_match *>(func_item);

    KEY *key_info = &(key_infos_[match_item->key]);
    uint n_key_parts = KEY_N_KEY_PARTS(key_info);
    for (uint i = 0; i < n_key_parts; ++i) {
      Field *field = key_info->key_part[i].field;
      encodings.push_back(encoding::convert(field->charset()));
    }

    GRN_CTX_SET_ENCODING(ctx_, encodings[0]);
    grn_obj *index_column = index_columns_[match_item->key];
    String query_buffer;
    String converted_query_buffer;
    String *query;
    query = match_item->key_item()->val_str(&query_buffer);
    DTCollation cmp_collation = match_item->cmp_collation;
    if (query->charset() != cmp_collation.collation) {
      uint errors;
      converted_query_buffer.copy(query->ptr(),
                                  query->length(),
                                  query->charset(),
                                  cmp_collation.collation,
                                  &errors);
      query = &converted_query_buffer;
    }
    if (MRN_MATCH_ITEM_FLAGS(match_item) & FT_BOOL) {
      grn_obj *match_columns;
      grn_obj *match_columns_variable;
      GRN_EXPR_CREATE_FOR_QUERY(ctx_,
                                table_,
                                match_columns,
                                match_columns_variable);
      // TODO: Use std::vector, emplace_back() and move semantics when
      // we drop support for MariaDB 10.3.
      match_columns_list.push_front(SmartGrnObj(ctx_,
                                                  static_cast<grn_obj *>(NULL)));
      match_columns_list.front().reset(match_columns);
      grn_obj source_ids;
      GRN_RECORD_INIT(&source_ids, GRN_OBJ_VECTOR, GRN_ID_NIL);
      grn_obj_get_info(ctx_, index_column, GRN_INFO_SOURCE, &source_ids);
      QueryParser query_parser(ctx_,
                               thread_,
                               expression,
                               index_column,
                               GRN_RECORD_VECTOR_SIZE(&source_ids),
                               match_columns);
      query_parser.parse(query->ptr(), query->length());
      GRN_OBJ_FIN(ctx_, &source_ids);
    } else {
      grn_expr_append_obj(ctx_,
                          expression,
                          index_column,
                          GRN_OP_PUSH,
                          1);
      grn_expr_append_const_str(ctx_,
                                expression,
                                query->ptr(),
                                query->length(),
                                GRN_OP_PUSH,
                                1);
      grn_expr_append_op(ctx_, expression, GRN_OP_SIMILAR, 2);
    }

    DBUG_RETURN(true);
  }

  void ConditionConverter::append_field_value(const Item_field *field_item,
                                              grn_obj *expression) {
    MRN_DBUG_ENTER_METHOD();

    GRN_BULK_REWIND(&column_name_);
    GRN_TEXT_PUT(ctx_, &column_name_,
                 MRN_ITEM_FIELD_GET_NAME(field_item),
                 MRN_ITEM_FIELD_GET_NAME_LENGTH(field_item));
    grn_expr_append_const(ctx_, expression, &column_name_,
                          GRN_OP_PUSH, 1);
    grn_expr_append_op(ctx_, expression, GRN_OP_GET_VALUE, 1);

    DBUG_VOID_RETURN;
  }

  void ConditionConverter::append_const_item(const Item_field *field_item,
                                             Item *const_item,
                                             grn_obj *expression) {
    MRN_DBUG_ENTER_METHOD();

    enum_field_types field_type = field_item->field->real_type();
    NormalizedType normalized_type = normalize_field_type(field_type);

    switch (normalized_type) {
    case STRING_TYPE:
      grn_obj_reinit(ctx_, &value_, GRN_DB_TEXT, 0);
      {
        String buffer;
        String *string;
        string = const_item->val_str(&buffer);
        GRN_TEXT_SET(ctx_, &value_, string->ptr(), string->length());
      }
      break;
    case INT_TYPE:
      grn_obj_reinit(ctx_, &value_, GRN_DB_INT64, 0);
      if (field_type == MYSQL_TYPE_ENUM) {
        if (MRN_ITEM_IS_STRING_TYPE(const_item)) {
          String *string;
          string = const_item->val_str(NULL);
          Field_enum *enum_field = static_cast<Field_enum *>(field_item->field);
          int enum_value = find_type(string->c_ptr(),
                                     enum_field->typelib,
                                     FIND_TYPE_BASIC);
          GRN_INT64_SET(ctx_, &value_, enum_value);
        } else {
          GRN_INT64_SET(ctx_, &value_, const_item->val_int());
        }
      } else {
        GRN_INT64_SET(ctx_, &value_, const_item->val_int());
      }
      break;
    case TIME_TYPE:
      grn_obj_reinit(ctx_, &value_, GRN_DB_TIME, 0);
      {
        MYSQL_TIME mysql_time;
        get_time_value(field_item, const_item, &mysql_time);
        bool truncated = false;
        TimeConverter time_converter;
        long long int time =
          time_converter.mysql_time_to_grn_time(&mysql_time, &truncated);
        GRN_TIME_SET(ctx_, &value_, time);
      }
      break;
    case UNSUPPORTED_TYPE:
      // Should not be occurred.
      DBUG_PRINT("error",
                 ("mroonga: append_const_item: unsupported type: <%d> "
                  "This case should not be occurred.",
                  field_type));
      grn_obj_reinit(ctx_, &value_, GRN_DB_VOID, 0);
      break;
    }
    grn_expr_append_const(ctx_, expression, &value_, GRN_OP_PUSH, 1);

    DBUG_VOID_RETURN;
  }
}
