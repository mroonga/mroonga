/* -*- c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
  Copyright(C) 2013 Kouhei Sutou <kou@clear-code.com>

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
#include <mrn_mysql_compat.h>
#include <mrn_path_mapper.hpp>
#include <mrn_windows.hpp>
#include <mrn_macro.hpp>

MRN_BEGIN_DECLS

struct EscapeInfo
{
  grn_ctx ctx;
  grn_obj target_characters;
  grn_obj escaped_query;
};

MRN_API my_bool mroonga_escape_init(UDF_INIT *initid, UDF_ARGS *args,
                                    char *message)
{
  EscapeInfo *info = NULL;

  initid->ptr = NULL;
  if (!(1 <= args->arg_count && args->arg_count <= 2)) {
    sprintf(message,
            "mroonga_escape(): Incorrect number of arguments: %u for 1..2",
            args->arg_count);
    goto error;
  }
  if (args->arg_type[0] != STRING_RESULT) {
    strcpy(message,
           "mroonga_escape(): The 1st argument must be query as string");
    goto error;
  }
  if (args->arg_count == 2) {
    if (args->arg_type[1] != STRING_RESULT) {
      strcpy(message,
             "mroonga_escape(): "
             "The 2st argument must be escape target characters as string");
      goto error;
    }
  }

  initid->maybe_null = 1;
  initid->const_item = 1;

  info = (EscapeInfo *)my_malloc(sizeof(EscapeInfo),
                                  MYF(MY_WME | MY_ZEROFILL));
  if (!info) {
    strcpy(message, "mroonga_escape(): out of memory");
    goto error;
  }

  grn_ctx_init(&(info->ctx), 0);
  GRN_TEXT_INIT(&(info->target_characters), 0);
  GRN_TEXT_INIT(&(info->escaped_query), 0);

  initid->ptr = (char *)info;

  return FALSE;

error:
  if (info) {
    grn_ctx_fin(&(info->ctx));
    my_free(info, MYF(0));
  }
  return TRUE;
}

MRN_API char *mroonga_escape(UDF_INIT *initid, UDF_ARGS *args, char *result,
                             unsigned long *length, char *is_null, char *error)
{
  EscapeInfo *info = (EscapeInfo *)initid->ptr;
  grn_ctx *ctx = &(info->ctx);
  char *query;
  unsigned int query_length;
  char *target_characters;
  unsigned int target_characters_length;

  if (!args->args[0]) {
    *is_null = 1;
    return NULL;
  }

  *is_null = 0;
  query = args->args[0];
  query_length = args->lengths[0];

  if (args->arg_count == 2) {
    target_characters = args->args[1];
    target_characters_length = args->lengths[1];
    GRN_TEXT_PUT(ctx, &(info->target_characters),
                 target_characters,
                 target_characters_length);
    GRN_TEXT_PUTC(ctx, &(info->target_characters), '\0');
    grn_expr_syntax_escape(ctx, query, query_length,
                           GRN_TEXT_VALUE(&(info->target_characters)),
                           GRN_QUERY_ESCAPE,
                           &(info->escaped_query));
  } else {
    grn_expr_syntax_escape_query(ctx, query, query_length,
                                 &(info->escaped_query));
  }

  if (ctx->rc) {
    my_message(ER_ERROR_ON_WRITE, ctx->errbuf, MYF(0));
    goto error;
  }

  *length = GRN_TEXT_LEN(&(info->escaped_query));
  return (char *)(GRN_TEXT_VALUE(&(info->escaped_query)));

error:
  *error = 1;
  return NULL;
}

MRN_API void mroonga_escape_deinit(UDF_INIT *initid)
{
  EscapeInfo *info = (EscapeInfo *)initid->ptr;
  if (info) {
    grn_obj_unlink(&(info->ctx), &(info->target_characters));
    grn_obj_unlink(&(info->ctx), &(info->escaped_query));
    grn_ctx_fin(&(info->ctx));
    my_free(info, MYF(0));
  }
}

MRN_END_DECLS
