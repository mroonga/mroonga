/*
  Copyright(C) 2010 Tetsuro IKEDA
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

#ifndef _mrn_sys_hpp
#define _mrn_sys_hpp

#include <groonga.h>
#include "mrn_macro.h"

MRN_BEGIN_DECLS

/* functions */
bool mrn_hash_put(grn_ctx *ctx, grn_hash *hash, const char *key, grn_obj *value);
bool mrn_hash_get(grn_ctx *ctx, grn_hash *hash, const char *key, grn_obj **value);
bool mrn_hash_remove(grn_ctx *ctx, grn_hash *hash, const char *key);

MRN_END_DECLS

#endif /* _mrn_sys_hpp */
