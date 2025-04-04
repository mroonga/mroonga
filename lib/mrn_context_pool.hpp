/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2015-2017 Kouhei Sutou <kou@clear-code.com>

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

#ifndef MRN_CONTEXT_POOL_HPP_
#define MRN_CONTEXT_POOL_HPP_

#include <mrn_mysql.h>

#include <groonga.h>

namespace mrn {
  class ContextPool {
  public:
    ContextPool(mysql_mutex_t *mutex,
                long *n_pooling_contexts);
    ~ContextPool(void);
    grn_ctx *pull(void);
    void release(grn_ctx *context);
    void clear(void);
    void set_n_workers(int n_workers);

  private:
    class Impl;
    Impl *impl_;
  };
}

#endif /* MRN_CONTEXT_POOL_HPP_ */
