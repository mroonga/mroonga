/*
  Copyright(C) 2011 Kentoku SHIBA
  Copyright(C) 2014-2021 Sutou Kouhei <kou@clear-code.com>

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

#pragma once

#define ER_MRN_INVALID_TABLE_PARAM_NUM 9501
#define ER_MRN_INVALID_TABLE_PARAM_STR "The table parameter '%-.64s' is invalid"
#define ER_MRN_CHARSET_NOT_SUPPORT_NUM 9502
#define ER_MRN_CHARSET_NOT_SUPPORT_STR "The character set '%s[%s]' is not supported by Groonga"
#define ER_MRN_GEOMETRY_NOT_SUPPORT_NUM 9503
#define ER_MRN_GEOMETRY_NOT_SUPPORT_STR "This geometry type is not supported. Groonga is supported point only"
#define ER_MRN_ERROR_FROM_GROONGA_NUM 9504
#define ER_MRN_ERROR_FROM_GROONGA_STR "Error from Groonga [%s]"
#define ER_MRN_INVALID_NULL_VALUE_NUM 9505
#define ER_MRN_INVALID_NULL_VALUE_STR "NULL value can't be used for %s"
#define ER_MRN_UNSUPPORTED_COLUMN_FLAG_NUM 9506
#define ER_MRN_UNSUPPORTED_COLUMN_FLAG_STR \
  "The column flag '%-.64s' is unsupported. It is ignored"
#define ER_MRN_INVALID_COLUMN_FLAG_NUM 9507
#define ER_MRN_INVALID_COLUMN_FLAG_STR \
  "The column flag '%-.64s' is invalid. It is ignored"
#define ER_MRN_INVALID_INDEX_FLAG_NUM 9508
#define ER_MRN_INVALID_INDEX_FLAG_STR \
  "The index flag '%-.64s' is invalid. It is ignored"
#define ER_MRN_KEY_BASED_ON_GENERATED_VIRTUAL_COLUMN_NUM 9509
#define ER_MRN_KEY_BASED_ON_GENERATED_VIRTUAL_COLUMN_STR \
  "Index for virtual generated column is not supported"
#define ER_MRN_INVALID_TABLE_FLAG_NUM 9510
#define ER_MRN_INVALID_TABLE_FLAG_STR \
  "The table flag '%-.64s' is invalid. It is ignored"
#define ER_MRN_INVALID_LEXICON_FLAG_NUM 9511
#define ER_MRN_INVALID_LEXICON_FLAG_STR \
  "The lexicon flag '%-.64s' is invalid. It is ignored"
