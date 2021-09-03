/* -*- c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
  Copyright(C) 2015-2020  Sutou Kouhei <kou@clear-code.com>

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

#include <mrn_mysql.h>
#include <mrn_mysql_compat.h>
#include <mrn_err.h>
#include <mrn_encoding.hpp>
#include <mrn_windows.hpp>
#include <mrn_table.hpp>
#include <mrn_macro.hpp>
#include <mrn_database_manager.hpp>
#include <mrn_context_pool.hpp>
#include <mrn_variables.hpp>
#include <mrn_query_parser.hpp>
#include <mrn_current_thread.hpp>
#include <mrn_index_table_name.hpp>
#include <mrn_smart_grn_obj.hpp>

#include <string>

MRN_BEGIN_DECLS

extern mrn::DatabaseManager *mrn_db_manager;
extern mrn::ContextPool *mrn_context_pool;

typedef struct st_mrn_snippet_html_info
{
  grn_ctx *ctx;
  grn_obj result;
  unsigned int mysql_table_name_index;
  unsigned int mysql_index_name_index;
  unsigned int mysql_lexicon_name_index;
  grn_obj *db;
  bool use_shared_db;
  grn_obj *snippet;
  grn_obj *query_table;
  grn_obj *query_default_column;
} mrn_snippet_html_info;

static mrn_bool mrn_snippet_html_prepare(mrn_snippet_html_info *info,
                                        UDF_ARGS *args,
                                        char *message,
                                        grn_obj **snippet)
{
  MRN_DBUG_ENTER_FUNCTION();

  grn_ctx *ctx = info->ctx;
  int flags = GRN_SNIP_SKIP_LEADING_SPACES;
  unsigned int width = 200;
  unsigned int max_n_results = 3;
  const char *open_tag = "<span class=\"keyword\">";
  const char *close_tag = "</span>";
  grn_snip_mapping *mapping = GRN_SNIP_MAPPING_HTML_ESCAPE;

  *snippet = NULL;

  mrn::encoding::set_raw(ctx, system_charset_info);

  *snippet = grn_snip_open(ctx, flags,
                           width, max_n_results,
                           open_tag, strlen(open_tag),
                           close_tag, strlen(close_tag),
                           mapping);
  if (ctx->rc != GRN_SUCCESS) {
    if (message) {
      snprintf(message, MYSQL_ERRMSG_SIZE,
               "mroonga_snippet_html(): failed to open grn_snip: <%s>",
               ctx->errbuf);
    }
    goto error;
  }

  if (info->mysql_table_name_index == 0 &&
      info->mysql_index_name_index == 0 &&
      info->mysql_lexicon_name_index == 0) {
    if (!(system_charset_info->state & (MY_CS_BINSORT | MY_CS_CSSORT))) {
      grn_snip_set_normalizer(ctx, *snippet, GRN_NORMALIZER_AUTO);
    }
  } else {
    std::string lexicon_name;
    if (info->mysql_lexicon_name_index > 0) {
      const char *mysql_lexicon_name =
        args->args[info->mysql_lexicon_name_index];
      const size_t mysql_lexicon_name_length =
        args->lengths[info->mysql_lexicon_name_index];
      if (mysql_lexicon_name_length == 0) {
        if (message) {
          snprintf(message, MYSQL_ERRMSG_SIZE,
                   "mroonga_snippet_html(): lexicon_name is empty");
        }
        goto error;
      }
      lexicon_name = std::string(mysql_lexicon_name,
                                 mysql_lexicon_name_length);
    } else {
      if (info->mysql_table_name_index == 0) {
        if (message) {
          snprintf(message, MYSQL_ERRMSG_SIZE,
                   "mroonga_snippet_html(): table_name is missing");
        }
        goto error;
      }
      const char *mysql_table_name = args->args[info->mysql_table_name_index];
      const size_t mysql_table_name_length =
        args->lengths[info->mysql_table_name_index];
      if (mysql_table_name_length == 0) {
        if (message) {
          snprintf(message, MYSQL_ERRMSG_SIZE,
                   "mroonga_snippet_html(): table_name is empty");
        }
        goto error;
      }
      if (info->mysql_index_name_index == 0) {
        if (message) {
          snprintf(message, MYSQL_ERRMSG_SIZE,
                   "mroonga_snippet_html(): index_name is missing");
        }
        goto error;
      }
      const char *mysql_index_name = args->args[info->mysql_index_name_index];
      const size_t mysql_index_name_length =
        args->lengths[info->mysql_index_name_index];
      if (mysql_index_name_length == 0) {
        if (message) {
          snprintf(message, MYSQL_ERRMSG_SIZE,
                   "mroonga_snippet_html(): index_name is empty");
        }
        goto error;
      }
      mrn::IndexTableName index_table_name(mysql_table_name,
                                           mysql_table_name_length,
                                           mysql_index_name,
                                           mysql_index_name_length);
      lexicon_name = std::string(index_table_name.c_str(),
                                 index_table_name.length());
    }
    mrn::SmartGrnObj lexicon(ctx,
                             lexicon_name.c_str(),
                             lexicon_name.length());
    if (!lexicon.get()) {
      if (message) {
        snprintf(message, MYSQL_ERRMSG_SIZE,
                 "mroonga_snippet_html(): nonexistent index: <%.*s>",
                 static_cast<int>(lexicon_name.length()),
                 lexicon_name.c_str());
      }
      goto error;
    }
    grn_snip_set_normalizer(ctx, *snippet, lexicon.get());
  }

  for (unsigned int i = 1; i < args->arg_count; ++i) {
    if (!args->args[i]) {
      continue;
    }
    grn_raw_string arg_name = {
      args->attributes[i],
      args->attribute_lengths[i],
    };
    if (GRN_RAW_STRING_EQUAL_CSTRING(arg_name, "table_name") ||
        GRN_RAW_STRING_EQUAL_CSTRING(arg_name, "index_name") ||
        GRN_RAW_STRING_EQUAL_CSTRING(arg_name, "lexicon_name")) {
      // Do nothing
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(arg_name, "query")) {
      if (!info->query_table) {
        grn_obj *short_text;
        short_text = grn_ctx_at(info->ctx, GRN_DB_SHORT_TEXT);
        info->query_table = grn_table_create(info->ctx,
                                             NULL, 0, NULL,
                                             GRN_TABLE_HASH_KEY,
                                             short_text,
                                             NULL);
      }
      if (!info->query_default_column) {
        info->query_default_column =
          grn_obj_column(info->ctx,
                         info->query_table,
                         GRN_COLUMN_NAME_KEY,
                         GRN_COLUMN_NAME_KEY_LEN);
      }

      grn_obj *expr = NULL;
      grn_obj *record = NULL;
      GRN_EXPR_CREATE_FOR_QUERY(info->ctx, info->query_table, expr, record);
      if (!expr) {
        if (message) {
          snprintf(message, MYSQL_ERRMSG_SIZE,
                   "mroonga_snippet_html(): "
                   "failed to create expression: <%s>",
                   ctx->errbuf);
        }
        goto error;
      }

      mrn::SmartGrnObj smart_expr(ctx, expr);
      mrn::QueryParser query_parser(info->ctx,
                                    current_thd,
                                    expr,
                                    info->query_default_column,
                                    0,
                                    NULL);
      grn_rc rc = query_parser.parse(args->args[i], args->lengths[i]);
      if (rc != GRN_SUCCESS) {
        if (message) {
          snprintf(message, MYSQL_ERRMSG_SIZE,
                   "mroonga_snippet_html(): "
                   "failed to parse query: "
                   "<%.*s>: "
                   "<%s>",
                   static_cast<int>(args->lengths[i]),
                   args->args[i],
                   ctx->errbuf);
        }
        goto error;
      }

      rc = grn_expr_snip_add_conditions(info->ctx,
                                        expr,
                                        *snippet,
                                        0,
                                        NULL, NULL,
                                        NULL, NULL);
      if (rc != GRN_SUCCESS) {
        if (message) {
          snprintf(message, MYSQL_ERRMSG_SIZE,
                   "mroonga_snippet_html(): "
                   "failed to add conditions: <%s>",
                   ctx->errbuf);
        }
        goto error;
      }
    } else {
      grn_rc rc = grn_snip_add_cond(ctx, *snippet,
                                    args->args[i], args->lengths[i],
                                    NULL, 0,
                                    NULL, 0);
      if (rc != GRN_SUCCESS) {
        if (message) {
          snprintf(message, MYSQL_ERRMSG_SIZE,
                   "mroonga_snippet_html(): "
                   "failed to add a condition to grn_snip: <%s>",
                   ctx->errbuf);
        }
        goto error;
      }
    }
  }

  DBUG_RETURN(false);

error:
  if (*snippet) {
    grn_obj_close(ctx, *snippet);
  }
  DBUG_RETURN(true);
}

MRN_API mrn_bool mroonga_snippet_html_init(UDF_INIT *init,
                                           UDF_ARGS *args,
                                           char *message)
{
  MRN_DBUG_ENTER_FUNCTION();

  mrn_snippet_html_info *info = NULL;

  init->ptr = NULL;

  if (args->arg_count < 1) {
    snprintf(message, MYSQL_ERRMSG_SIZE,
             "mroonga_snippet_html(): wrong number of arguments: %u for 1+",
             args->arg_count);
    goto error;
  }


  for (unsigned int i = 0; i < args->arg_count; ++i) {
    switch (args->arg_type[i]) {
    case STRING_RESULT:
      /* OK */
      break;
    case REAL_RESULT:
      snprintf(message, MYSQL_ERRMSG_SIZE,
               "mroonga_snippet_html(): all arguments must be string: "
               "<%u>=<%g>",
               i, *((double *)(args->args[i])));
      goto error;
      break;
    case INT_RESULT:
      snprintf(message, MYSQL_ERRMSG_SIZE,
               "mroonga_snippet_html(): all arguments must be string: "
               "<%u>=<%lld>",
               i, *((longlong *)(args->args[i])));
      goto error;
      break;
    default:
      snprintf(message, MYSQL_ERRMSG_SIZE,
               "mroonga_snippet_html(): all arguments must be string: <%u>",
               i);
      goto error;
      break;
    }
  }

  init->maybe_null = 1;

  info = (mrn_snippet_html_info *)mrn_my_malloc(sizeof(mrn_snippet_html_info),
                                                MYF(MY_WME | MY_ZEROFILL));
  if (!info) {
    snprintf(message, MYSQL_ERRMSG_SIZE,
             "mroonga_snippet_html(): failed to allocate memory");
    goto error;
  }

  info->ctx = mrn_context_pool->pull();
  {
    const char *current_db_path = MRN_THD_DB_PATH(current_thd);
    const char *action;
    if (current_db_path) {
      action = "open database";
      mrn::Database *db;
      int error = mrn_db_manager->open(current_db_path, &db);
      if (error == 0) {
        info->db = db->get();
        grn_ctx_use(info->ctx, info->db);
        info->use_shared_db = true;
      }
    } else {
      action = "create anonymous database";
      info->db = grn_db_create(info->ctx, NULL, NULL);
      info->use_shared_db = false;
    }
    if (!info->db) {
      sprintf(message,
              "mroonga_snippet_html(): failed to %s: %s",
              action,
              info->ctx->errbuf);
      goto error;
    }
  }
  GRN_TEXT_INIT(&(info->result), 0);
  info->mysql_table_name_index = 0;
  info->mysql_index_name_index = 0;
  info->mysql_lexicon_name_index = 0;

  info->query_table = NULL;
  info->query_default_column = NULL;
  for (unsigned int i = 1; i < args->arg_count; ++i) {
    grn_raw_string arg_name = {
      args->attributes[i],
      args->attribute_lengths[i],
    };
    if (GRN_RAW_STRING_EQUAL_CSTRING(arg_name, "query") ||
        GRN_RAW_STRING_EQUAL_CSTRING(arg_name, "keyword")) {
      // Do nothing
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(arg_name, "table_name")) {
      if (info->mysql_table_name_index != 0) {
        sprintf(message,
                "mroonga_snippet_html(): "
                "can't specify table_name multiple times");
        goto error;
      }
      if (info->mysql_lexicon_name_index != 0) {
        sprintf(message,
                "mroonga_snippet_html(): "
                "can't specify table_name with lexicon_name");
        goto error;
      }
      info->mysql_table_name_index = i;
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(arg_name, "index_name")) {
      if (info->mysql_index_name_index != 0) {
        sprintf(message,
                "mroonga_snippet_html(): "
                "can't specify index_name multiple times");
        goto error;
      }
      if (info->mysql_lexicon_name_index != 0) {
        sprintf(message,
                "mroonga_snippet_html(): "
                "can't specify index_name with lexicon_name");
        goto error;
      }
      info->mysql_index_name_index = i;
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(arg_name, "lexicon_name")) {
      if (info->mysql_lexicon_name_index != 0) {
        sprintf(message,
                "mroonga_snippet_html(): "
                "can't specify lexicon_name multiple times");
        goto error;
      }
      if (info->mysql_table_name_index != 0) {
        sprintf(message,
                "mroonga_snippet_html(): "
                "can't specify lexicon_name with table_name");
        goto error;
      }
      if (info->mysql_index_name_index != 0) {
        sprintf(message,
                "mroonga_snippet_html(): "
                "can't specify lexicon_name with index_name");
        goto error;
      }
      info->mysql_lexicon_name_index = i;
    } else {
      if (arg_name.length > 0 && arg_name.value[0] == '\'') {
        // No "AS XXX"
      } else {
        sprintf(message,
                "mroonga_snippet_html(): unknown named argument: %.*s",
                static_cast<int>(arg_name.length),
                arg_name.value);
        goto error;
      }
    }
  }

  {
    bool all_arguments_are_constant = true;
    for (unsigned int i = 1; i < args->arg_count; ++i) {
      if (!args->args[i]) {
        all_arguments_are_constant = false;
        break;
      }
    }

    if (all_arguments_are_constant) {
      if (mrn_snippet_html_prepare(info, args, message, &(info->snippet))) {
        goto error;
      }
    } else {
      info->snippet = NULL;
    }
  }

  init->ptr = (char *)info;

  DBUG_RETURN(false);

error:
  if (info) {
    if (info->db) {
      GRN_OBJ_FIN(info->ctx, &(info->result));
    }
    if (!info->use_shared_db) {
      grn_obj_close(info->ctx, info->db);
    }
    mrn_context_pool->release(info->ctx);
    my_free(info);
  }
  DBUG_RETURN(true);
}

MRN_API char *mroonga_snippet_html(UDF_INIT *init,
                                   UDF_ARGS *args,
                                   char *result,
                                   unsigned long *length,
                                   char *is_null,
                                   char *error)
{
  MRN_DBUG_ENTER_FUNCTION();

  mrn_snippet_html_info *info =
    reinterpret_cast<mrn_snippet_html_info *>(init->ptr);

  grn_ctx *ctx = info->ctx;
  grn_obj *snippet = info->snippet;
  grn_obj *result_buffer = &(info->result);

  if (!args->args[0]) {
    *is_null = 1;
    DBUG_RETURN(NULL);
  }

  if (!snippet) {
    if (mrn_snippet_html_prepare(info, args, NULL, &snippet)) {
      goto error;
    }
  }

  {
    char *target = args->args[0];
    unsigned int target_length = args->lengths[0];

    unsigned int n_results, max_tagged_length;
    {
      grn_rc rc = grn_snip_exec(ctx, snippet, target, target_length,
                                &n_results, &max_tagged_length);
      if (rc != GRN_SUCCESS) {
        my_printf_error(ER_MRN_ERROR_FROM_GROONGA_NUM,
                        ER_MRN_ERROR_FROM_GROONGA_STR, MYF(0), ctx->errbuf);
        goto error;
      }
    }

    *is_null = 0;
    GRN_BULK_REWIND(result_buffer);

    {
      const char *start_tag = "<div class=\"snippet\">";
      const char *end_tag = "</div>";
      size_t start_tag_length = strlen(start_tag);
      size_t end_tag_length = strlen(end_tag);
      for (unsigned int i = 0; i < n_results; ++i) {
        GRN_TEXT_PUT(ctx, result_buffer, start_tag, start_tag_length);

        grn_bulk_reserve(ctx, result_buffer, max_tagged_length);
        unsigned int result_length;
        grn_rc rc =
          grn_snip_get_result(ctx, snippet, i,
                              GRN_BULK_CURR(result_buffer),
                              &result_length);
        if (rc) {
          my_printf_error(ER_MRN_ERROR_FROM_GROONGA_NUM,
                          ER_MRN_ERROR_FROM_GROONGA_STR, MYF(0), ctx->errbuf);
          goto error;
        }
        grn_bulk_space(ctx, result_buffer, result_length);

        GRN_TEXT_PUT(ctx, result_buffer, end_tag, end_tag_length);
      }
    }

    if (!info->snippet) {
      grn_rc rc = grn_obj_close(ctx, snippet);
      if (rc != GRN_SUCCESS) {
        my_printf_error(ER_MRN_ERROR_FROM_GROONGA_NUM,
                        ER_MRN_ERROR_FROM_GROONGA_STR, MYF(0), ctx->errbuf);
        goto error;
      }
    }
  }

  *length = GRN_TEXT_LEN(result_buffer);
  DBUG_RETURN(GRN_TEXT_VALUE(result_buffer));

error:
  if (!info->snippet && snippet) {
    grn_obj_close(ctx, snippet);
  }

  *is_null = 1;
  *error = 1;

  DBUG_RETURN(NULL);
}

MRN_API void mroonga_snippet_html_deinit(UDF_INIT *init)
{
  MRN_DBUG_ENTER_FUNCTION();

  mrn_snippet_html_info *info =
    reinterpret_cast<mrn_snippet_html_info *>(init->ptr);
  if (!info) {
    DBUG_VOID_RETURN;
  }

  if (info->snippet) {
    grn_obj_close(info->ctx, info->snippet);
  }
  if (info->query_default_column) {
    grn_obj_close(info->ctx, info->query_default_column);
  }
  if (info->query_table) {
    grn_obj_close(info->ctx, info->query_table);
  }
  GRN_OBJ_FIN(info->ctx, &(info->result));
  if (!info->use_shared_db) {
    grn_obj_close(info->ctx, info->db);
  }
  mrn_context_pool->release(info->ctx);
  my_free(info);

  DBUG_VOID_RETURN;
}

MRN_END_DECLS
