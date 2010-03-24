#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "driver.h"
#include "config.h"

grn_hash *mrn_system_hash;
grn_obj *mrn_system_db;
pthread_mutex_t *mrn_lock, *mrn_lock_hash;
const char *mrn_logfile_name=MRN_LOG_FILE_NAME;
FILE *mrn_logfile = NULL;

uint mrn_hash_counter=0;

grn_logger_info mrn_logger_info = {
  GRN_LOG_DUMP,
  GRN_LOG_TIME|GRN_LOG_MESSAGE,
  mrn_logger_func,
  NULL
};

const char *mrn_log_level_str[] =
{
  "NONE",
  "EMERG",
  "ALERT",
  "CRIT",
  "ERROR",
  "WARNING",
  "NOTICE",
  "INFO",
  "DEBUG",
  "DUMP"
};

void mrn_logger_mysql(int level, const char *time, const char *title,
                      const char *msg, const char *location, void *func_arg)
{
  fprintf(stderr, "%s mroonga [%s] %s\n",
          time, *(mrn_log_level_str + level), msg);
}


void mrn_logger_func(int level, const char *time, const char *title,
                     const char *msg, const char *location, void *func_arg)
{
  const char slev[] = " EACewnid-";
  if ((mrn_logfile)) {
    fprintf(mrn_logfile, "%s|%c|%u|%s\n", time,
	    *(slev + level), (uint)pthread_self(), msg);
    fflush(mrn_logfile);
  }
}

int mrn_init(int in_mysql)
{
  grn_ctx ctx;

  // init groonga
  if (grn_init() != GRN_SUCCESS) {
    goto err;
  }

  grn_ctx_init(&ctx,0);

  grn_logger_info_set(&ctx, &mrn_logger_info);
  if (in_mysql) {
    mrn_logger_info.max_level = GRN_LOG_DEBUG;
    mrn_logger_info.func = mrn_logger_mysql;
  } else {
    if (!(mrn_logfile = fopen(mrn_logfile_name, "a"))) {
      goto err;
    }
  }
  GRN_LOG(&ctx, GRN_LOG_NOTICE, "%s start", PACKAGE_STRING);

  // init meta-info database
  if (!(mrn_system_db = grn_db_create(&ctx, NULL, NULL))) {
    GRN_LOG(&ctx, GRN_LOG_ERROR, "cannot create system database, exiting");
    goto err;
  }
  grn_ctx_use(&ctx, mrn_system_db);

  // init hash
  if (!(mrn_system_hash = grn_hash_create(&ctx,NULL,
                                   MRN_MAX_KEY_LEN,sizeof(size_t),
                                   GRN_OBJ_KEY_VAR_SIZE))) {
    GRN_LOG(&ctx, GRN_LOG_ERROR, "cannot init hash, exiting");
    goto err;
  }

  // init lock
  mrn_lock = malloc(sizeof(pthread_mutex_t));
  if ((mrn_lock == NULL) || (pthread_mutex_init(mrn_lock, NULL) != 0)) {
    goto err;
  }
  mrn_lock_hash = malloc(sizeof(pthread_mutex_t));
  if ((mrn_lock_hash == NULL) || (pthread_mutex_init(mrn_lock_hash, NULL) != 0)) {
    goto err;
  }

  grn_ctx_fin(&ctx);
  return 0;

err:
  // TODO: report more detail error to mysql
  grn_ctx_fin(&ctx);
  return -1;
}

int mrn_deinit()
{
  grn_ctx ctx;
  grn_ctx_init(&ctx,0);
  grn_ctx_use(&ctx, mrn_system_db);
  GRN_LOG(&ctx, GRN_LOG_NOTICE, "shutdown");


  pthread_mutex_destroy(mrn_lock);
  free(mrn_lock);
  mrn_lock = NULL;
  pthread_mutex_destroy(mrn_lock_hash);
  free(mrn_lock_hash);
  mrn_lock_hash = NULL;

  grn_hash_close(&ctx, mrn_system_hash);
  mrn_system_hash = NULL;

  grn_obj_unlink(&ctx, mrn_system_db);
  mrn_system_db = NULL;

  if (mrn_logfile) {
    fclose(mrn_logfile);
  }
  mrn_logfile = NULL;

  grn_ctx_fin(&ctx);
  grn_fin();

  return 0;
}

/**
 *   0 success
 *  -1 duplicated
 */
int mrn_hash_put(grn_ctx *ctx, const char *key, void *value)
{
  int added, res=0;
  void *buf;
  pthread_mutex_lock(mrn_lock_hash);
  grn_hash_add(ctx, mrn_system_hash, (const char*) key, strlen(key), &buf, &added);
  // duplicate check
  if (added == 0) {
    GRN_LOG(ctx, GRN_LOG_WARNING, "hash put duplicated (key=%s)", key);
    res = -1;
  } else {
    // store address of value
    memcpy(buf, &value, sizeof(buf));
    mrn_hash_counter++;
    GRN_LOG(ctx, GRN_LOG_DEBUG, "hash put (key=%s)", key);
  }
  pthread_mutex_unlock(mrn_lock_hash);
  return res;
}

/**
 *   0 success
 *  -1 not found
 */
int mrn_hash_get(grn_ctx *ctx, const char *key, void **value)
{
  int res = 0;
  grn_id id;
  void *buf;
  pthread_mutex_lock(mrn_lock_hash);
  id = grn_hash_get(ctx, mrn_system_hash, (const char*) key, strlen(key), &buf);
  // key not found
  if (id == GRN_ID_NIL) {
    GRN_LOG(ctx, GRN_LOG_DEBUG, "hash get not found (key=%s)", key);
    res = -1;
  } else {
    // restore address of value
    memcpy(value, buf, sizeof(buf));
  }
  pthread_mutex_unlock(mrn_lock_hash);
  return res;
}

/**
 *   0 success
 *  -1 error
 */
int mrn_hash_remove(grn_ctx *ctx, const char *key)
{
  int res = 0;
  grn_rc rc;
  grn_id id;
  pthread_mutex_lock(mrn_lock_hash);
  id = grn_hash_get(ctx, mrn_system_hash, (const char*) key, strlen(key), NULL);
  if (id == GRN_ID_NIL) {
    GRN_LOG(ctx, GRN_LOG_WARNING, "hash remove not found (key=%s)", key);
    res = -1;
  } else {
    rc = grn_hash_delete_by_id(ctx, mrn_system_hash, id, NULL);
    if (rc != GRN_SUCCESS) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "hash remove error (key=%s)", key);
      res = -1;
    } else {
      GRN_LOG(ctx, GRN_LOG_DEBUG, "hash remove (key=%s)", key);
      mrn_hash_counter--;
    }
  }
  pthread_mutex_unlock(mrn_lock_hash);
  return res;
}

mrn_info *mrn_init_obj_info(grn_ctx *ctx, uint n_columns)
{
  int i, alloc_size = 0;
  void *ptr;
  mrn_info *info;
  alloc_size = sizeof(mrn_info) + sizeof(mrn_db_info) +
    MRN_MAX_PATH_SIZE + sizeof(mrn_table_info) +
    (sizeof(void*) + sizeof(mrn_column_info)) * n_columns;
  ptr = malloc(alloc_size);
  if (ptr == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "malloc error mrn_init_obj_info size=%d",alloc_size);
    return NULL;
  }
  info = (mrn_info*) ptr;
  info->res = NULL;
  info->cursor = NULL;

  ptr += sizeof(mrn_info);
  info->db = ptr;
  info->db->name = NULL;
  info->db->name_size = 0;
  ptr += sizeof(mrn_db_info);
  info->db->path = ptr;

  ptr += MRN_MAX_PATH_SIZE;
  info->table = ptr;
  info->table->name = NULL;
  info->table->name_size = 0;
  info->table->flags = GRN_OBJ_PERSISTENT;
  info->table->key_type = NULL;

  ptr += sizeof(mrn_table_info);
  info->columns = (mrn_column_info**) ptr;

  ptr += sizeof(void*) * n_columns;
  for (i = 0; i < n_columns; i++) {
    info->columns[i] = ptr + sizeof(mrn_column_info) * i;
    info->columns[i]->name = NULL;
    info->columns[i]->name_size = 0;
    info->columns[i]->flags = GRN_OBJ_PERSISTENT;
    info->columns[i]->type = NULL;
  }

  info->n_columns = n_columns;
  return info;
}

int mrn_deinit_obj_info(grn_ctx *ctx, mrn_info *info)
{
  free(info);
  return 0;
}

int mrn_create(grn_ctx *ctx, mrn_info *info, mrn_object *obj)
{
  int i;
  mrn_table_info *table;
  mrn_column_info *column;

  table = info->table;
  obj->table = grn_table_create(ctx, table->name, table->name_size,
                                NULL, table->flags,
                                table->key_type, 0);
  if (obj->table == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create table: name=%s, name_size=%d, "
            "flags=%d, key_type=%p, value_size=%d", table->name, table->name_size,
            table->flags, table->key_type, NULL);
    return -1;
  }

  obj->columns = malloc(sizeof(grn_obj*) * info->n_columns);
  for (i=0; i < info->n_columns; i++) {
    column = info->columns[i];
    obj->columns[i] = grn_column_create(ctx, obj->table, column->name,
                                        column->name_size, NULL,
                                        column->flags, column->type);
    if (obj->columns[i] == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create column: table=%p, name=%s, name_size=%d, "
              "flags=%d, type=%p", obj->table, column->name,
              column->name_size, column->flags, column->type);
      goto auto_drop;
    }
  }
  return 0;

auto_drop:
  GRN_LOG(ctx, GRN_LOG_ERROR, "auto-drop table/columns");
  grn_obj_remove(ctx, obj->table);
  obj->table = NULL;
  for (i=0; i < info->n_columns; i++) {
    obj->columns[i] = NULL;
  }
  return -1;
}

int mrn_open(grn_ctx *ctx, mrn_info *info, mrn_object *obj)
{
  int i;
  mrn_table_info *table;
  mrn_column_info *column;

  table = info->table;
  obj->table = grn_ctx_get(ctx, table->name, table->name_size);
  if (obj->table == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open table: name=%s", table->name);
    return -1;
  }
  obj->columns = malloc(sizeof(grn_obj*) * info->n_columns);
  for (i=0; i < info->n_columns; i++) {
    column = info->columns[i];
    obj->columns[i] = grn_obj_column(ctx, obj->table, column->name, column->name_size);
    if (obj->columns[i] == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open column: table=%s, column=%s",
              table->name, column->name);
      goto auto_close;
    }
  }
  return 0;

auto_close:
  GRN_LOG(ctx, GRN_LOG_ERROR, "auto-closing table/columns");
  grn_obj_unlink(ctx, obj->table);
  obj->table = NULL;
  for (i=0; i < info->n_columns; i++) {
    obj->columns[i] = NULL;
  }
  return -1;
}

int mrn_close(grn_ctx *ctx, mrn_info *info, mrn_object *obj)
{
  int i;
  grn_obj_unlink(ctx, obj->table);
  for (i=0; i < info->n_columns; i++) {
    obj->columns[i] = NULL;
  }
  free(obj->columns);
  obj->table = NULL;
  return 0;
}

int mrn_drop(grn_ctx *ctx, char *db_path, char *table_name)
{
  grn_obj *db, *table;
  db = grn_db_open(ctx, db_path);
  if (db == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "grn_db_open failed while mrn_drop (%s)",
            db_path);
    return -1;
  }
  grn_ctx_use(ctx, db);
  table = grn_ctx_get(ctx, table_name, strlen(table_name));
  if (table == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "grn_ctx_get failed while mrn_drop (%s)",
            table_name);
    return -1;
  }
  return grn_obj_remove(ctx, table);
}

int mrn_write_row(grn_ctx *ctx, mrn_record *record, mrn_object *obj)
{
  grn_obj *table, *column;
  grn_id gid;
  int added, i, j;
  table = obj->table;
  gid = grn_table_add(ctx, table, record->key, record->key_size, &added);
  if (added == 0) {
    goto duplicated_key_err;
  }
  for (i=0, j=0; i < record->info->n_columns; i++) {
    column = obj->columns[i];
    if (MRN_IS_BIT(record->bitmap, i)) {
      if (grn_obj_set_value(ctx, column, gid, record->value[j], GRN_OBJ_SET)
          != GRN_SUCCESS) {
        goto obj_set_err;
      }
      j++;
    }
  }
  return 0;

duplicated_key_err:
  GRN_LOG(ctx, GRN_LOG_INFO, "duplicated key error for table=%s",
          record->info->table->name);
  return -2;
obj_set_err:
  return -1;
}

mrn_record* mrn_init_record(grn_ctx *ctx, mrn_info *info, uchar *bitmap, int size)
{
  mrn_record *record;
  int i, j, alloc_size, offset;
  void *p;
  alloc_size = sizeof(mrn_record) + (sizeof(grn_obj*) + sizeof(grn_obj)) * size;
  p = malloc(alloc_size);
  record = (mrn_record*) p;
  p += sizeof(mrn_record);
  record->info = info;
  record->bitmap = bitmap;
  record->value = (grn_obj**) p;
  p += sizeof(grn_obj*) * size;
  for (i=0,j=0,offset=0; i < info->n_columns; i++) {
    // column pruning
    if (MRN_IS_BIT(bitmap, i)) {
      record->value[j] = (grn_obj*) (p + offset);
      grn_builtin_type gtype = info->columns[i]->gtype;
      switch (gtype) {
      case GRN_DB_INT32:
        GRN_INT32_INIT(record->value[j], 0);
        break;
      case GRN_DB_TEXT:
        GRN_TEXT_INIT(record->value[j], 0);
        break;
      default:
        GRN_TEXT_INIT(record->value[j], 0);
      }
      offset += sizeof(grn_obj);
      j++;
    }
  }
  record->actual_size = size;
  record->n_columns = info->n_columns;
  return record;
}


int mrn_deinit_record(grn_ctx *ctx, mrn_record *record)
{
  int i;
  for (i=0; i < record->actual_size; i++) {
    grn_obj_unlink(ctx, record->value[i]);
  }
  free(record);
  return 0;
}

int mrn_rewind_record(grn_ctx *ctx, mrn_record *record)
{
  int i;
  for (i=0; i < record->actual_size; i++) {
    GRN_BULK_REWIND(record->value[i]);
  }
  return 0;
}

int mrn_rnd_init(grn_ctx *ctx, mrn_info *info, mrn_expr *expr, mrn_object *obj)
{
  if (expr) {
    grn_obj *v, intbuf, textbuf;
    GRN_INT32_INIT(&intbuf, 0);
    GRN_TEXT_INIT(&textbuf, 0);
    grn_obj *gexpr = grn_expr_create(ctx, NULL, 0);
    v = grn_expr_add_var(ctx, gexpr, NULL, 0);
    GRN_RECORD_INIT(v, 0, grn_obj_id(ctx, obj->table));

    mrn_expr *cur = expr;
    while (cur) {
      switch (cur->type) {
      case MRN_EXPR_UNKNOWN:
        break;
      case MRN_EXPR_COLUMN:
        GRN_BULK_REWIND(&textbuf);
        GRN_TEXT_SETS(ctx, &textbuf, cur->val_string);
        grn_expr_append_const(ctx, gexpr, &textbuf, GRN_OP_GET_VALUE, 1);
        break;
      case MRN_EXPR_AND:
        grn_expr_append_op(ctx, gexpr, GRN_OP_AND, 2);
        break;
      case MRN_EXPR_OR:
        grn_expr_append_op(ctx, gexpr, GRN_OP_OR, 2);
        break;
      case MRN_EXPR_EQ:
        grn_expr_append_op(ctx, gexpr, GRN_OP_EQUAL, 2);
        break;
      case MRN_EXPR_NOT_EQ:
        grn_expr_append_op(ctx, gexpr, GRN_OP_NOT_EQUAL, 2);
        break;
      case MRN_EXPR_GT:
        grn_expr_append_op(ctx, gexpr, GRN_OP_GREATER, 2);
        break;
      case MRN_EXPR_GT_EQ:
        grn_expr_append_op(ctx, gexpr, GRN_OP_GREATER_EQUAL, 2);
        break;
      case MRN_EXPR_LESS:
        grn_expr_append_op(ctx, gexpr, GRN_OP_LESS, 2);
        break;
      case MRN_EXPR_LESS_EQ:
        grn_expr_append_op(ctx, gexpr, GRN_OP_LESS_EQUAL, 2);
        break;
      case MRN_EXPR_INT:
        GRN_BULK_REWIND(&intbuf);
        GRN_INT32_SET(ctx, &intbuf, cur->val_int);
        grn_expr_append_const(ctx, gexpr, &intbuf, GRN_OP_PUSH, 1);
        break;
      case MRN_EXPR_TEXT:
        GRN_BULK_REWIND(&textbuf);
        GRN_TEXT_SETS(ctx, &textbuf, cur->val_string);
        grn_expr_append_const(ctx, gexpr, &textbuf, GRN_OP_PUSH, 1);
        break;
      case MRN_EXPR_NEGATIVE:
        break;
      }
      cur = cur->next;
    }
    grn_expr_compile(ctx, gexpr);
    info->res = grn_table_create(ctx, NULL, 0, NULL,
                                 GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC,
                                 obj->table, NULL);
    grn_table_select(ctx, obj->table, gexpr, info->res, GRN_OP_OR);
    info->cursor = grn_table_cursor_open(ctx, info->res, NULL, 0, NULL,
                                         0, 0, -1, 0);
    grn_obj_unlink(ctx, &intbuf);
    grn_obj_unlink(ctx, &textbuf);
  } else {
    info->cursor = grn_table_cursor_open(ctx, obj->table, NULL, 0, NULL, 0, 0, -1, 0);
    if (info->cursor == NULL) {
      GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open cursor: %s", info->table->name);
      return -1;
    }
  }
  return 0;
}

int mrn_rnd_next(grn_ctx *ctx, mrn_record *record, mrn_object *obj)
{
  int i,j;
  grn_table_cursor *cursor = record->info->cursor;
  record->id = grn_table_cursor_next(ctx, cursor);
  if (record->id == GRN_ID_NIL) {
    grn_table_cursor_close(ctx, cursor);
    record->info->cursor = NULL;
    return 1; // EOF
  } else {
    for (i=0,j=0; i < record->n_columns; i++) {
      grn_obj *res;
      // column pruning
      if (MRN_IS_BIT(record->bitmap, i)) {
        if (grn_obj_get_value(ctx, obj->columns[i],
                              record->id, record->value[j]) != NULL) {
          j++;
        } else {
          GRN_LOG(ctx, GRN_LOG_ERROR, "error while fetching cursor:[%s,%d,%d]",
                  record->info->table->name, i, record->id);
          goto err;
        }
      }
    }
  }
  return 0;

err:
  return -1;
}

uint mrn_table_size(grn_ctx *ctx, mrn_object *obj)
{
  return grn_table_size(ctx, obj->table);
}

void mrn_free_expr(mrn_expr *expr)
{
  if (expr && expr->next) {
    mrn_free_expr(expr->next);
  }
  free(expr);
}

void mrn_dump_expr(mrn_expr *expr)
{
  mrn_expr *cur = expr;
  printf("\nmrn_expr (");
  while (cur) {
    switch (cur->type) {
    case MRN_EXPR_UNKNOWN:
      printf("unknown ");
      break;
    case MRN_EXPR_COLUMN:
      printf("%s ", cur->val_string);
      break;
    case MRN_EXPR_AND:
      printf("and ");
      break;
    case MRN_EXPR_OR:
      printf("or ");
      break;
    case MRN_EXPR_EQ:
      printf("= ");
      break;
    case MRN_EXPR_NOT_EQ:
      printf("!= ");
      break;
    case MRN_EXPR_GT:
      printf("> ");
      break;
    case MRN_EXPR_GT_EQ:
      printf(">= ");
      break;
    case MRN_EXPR_LESS:
      printf("< ");
      break;
    case MRN_EXPR_LESS_EQ:
      printf("<= ");
      break;
    case MRN_EXPR_INT:
      printf("%d ", cur->val_int);
      break;
    case MRN_EXPR_TEXT:
      printf("'%s' ", cur->val_string);
      break;
    case MRN_EXPR_NEGATIVE:
      break;
    default:
      printf("?? ");
    }
    cur = cur->next;
  }
  printf(")\n");
}

void mrn_dump_buffer(uchar *buf, int size)
{
  int i;
  const char c[] = "0123456789ABCDEF";
  for (i=0; i < size; i++) {
    uchar cur = buf[i];
    uchar low_bit = cur & 0xF;
    uchar high_bit = cur >> 4;
    printf("%c%c ",c[high_bit],c[low_bit]);
  }
  printf("\n");
}

int mrn_db_open_or_create(grn_ctx *ctx, mrn_info *info, mrn_object *obj)
{
  mrn_db_info *db = info->db;
  pthread_mutex_lock(mrn_lock);
  if (mrn_hash_get(ctx, db->name, (void**) &(obj->db)) != 0) {
    struct stat dummy;
    if (stat(db->path, &dummy)) {
      GRN_LOG(ctx, GRN_LOG_INFO, "database not found. creating...(%s)", db->path);
      obj->db = grn_db_create(ctx, db->path, NULL);
      if (obj->db == NULL) {
        GRN_LOG(ctx, GRN_LOG_ERROR, "cannot create database (%s)", db->path);
        return -1;
      }
    } else {
      obj->db = grn_db_open(ctx, db->path);
      if (obj->db == NULL) {
        GRN_LOG(ctx, GRN_LOG_ERROR, "cannot open database (%s)", db->path);
        return -1;
      }
    }
    mrn_hash_put(ctx, db->name, obj->db);
  }
  pthread_mutex_unlock(mrn_lock);
  return 0;
}

int mrn_db_drop(grn_ctx *ctx, char *path)
{
  grn_obj *db = grn_db_open(ctx, path);
  if (db == NULL) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot drop database (%s)", path);
    return -1;
  }
  if (grn_obj_remove(ctx, db)) {
    GRN_LOG(ctx, GRN_LOG_ERROR, "cannot drop database (%s)", path);
    return -1;
  }
  /* workaround code (from) */
  char f[32];
  strncpy(f, path, 32);
  strncat(f, ".0000000", 32);
  unlink(f);
  /* workaround code (to) */
  return 0;
}
