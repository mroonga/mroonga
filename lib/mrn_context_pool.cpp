/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2015-2020 Sutou Kouhei <kou@clear-code.com>

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

#include "mrn_context_pool.hpp"
#include "mrn_lock.hpp"

#include <time.h>

#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif
#include <vector>

namespace mrn {
  // for debug
#define MRN_CLASS_NAME "mrn::ContextPool::Impl"

  class ContextPool::Impl {
  public:
    Impl(mysql_mutex_t *mutex,
         long *n_pooling_contexts)
      : mutex_(mutex),
        n_pooling_contexts_(n_pooling_contexts),
        pool_(),
        last_pull_time_(0) {
    }

    ~Impl(void) {
      clear();
    }

    grn_ctx *pull(void) {
      MRN_DBUG_ENTER_METHOD();
      grn_ctx *ctx = NULL;

      {
        time_t now;
        time(&now);

        mrn::Lock lock(mutex_);
        if (!pool_.empty()) {
          ctx = pool_[pool_.size() - 1];
          pool_.pop_back();
          if ((now - last_pull_time_) >= CLEAR_THREATHOLD_IN_SECONDS) {
            clear_without_lock();
          }
        }
        last_pull_time_ = now;
      }

      if (!ctx) {
        mrn::Lock lock(mutex_);
        ctx = grn_ctx_open(0);
        ++(*n_pooling_contexts_);
      }

      DBUG_RETURN(ctx);
    }

    void release(grn_ctx *ctx) {
      MRN_DBUG_ENTER_METHOD();

      {
        mrn::Lock lock(mutex_);
        pool_.push_back(ctx);
        grn_ctx_use(ctx, NULL);
      }

      DBUG_VOID_RETURN;
    }

    void clear(void) {
      MRN_DBUG_ENTER_METHOD();

      {
        mrn::Lock lock(mutex_);
        clear_without_lock();
      }

      DBUG_VOID_RETURN;
    }

  private:
    static const int CLEAR_THREATHOLD_IN_SECONDS = 60 * 5;

    mysql_mutex_t *mutex_;
    long *n_pooling_contexts_;
    std::vector<grn_ctx *> pool_;
    time_t last_pull_time_;

    void clear_without_lock(void) {
      MRN_DBUG_ENTER_METHOD();
      for (std::vector<grn_ctx *>::iterator it = pool_.begin();
           it != pool_.end();
           ++it) {
        grn_ctx *ctx = *it;
        grn_ctx_close(ctx);
        --(*n_pooling_contexts_);
      }
      pool_.clear();
      DBUG_VOID_RETURN;
    }
  };

// For debug
#undef MRN_CLASS_NAME
#define MRN_CLASS_NAME "mrn::ContextPool"

  ContextPool::ContextPool(mysql_mutex_t *mutex,
                           long *n_pooling_contexts)
    : impl_(new Impl(mutex, n_pooling_contexts)) {
  }

  ContextPool::~ContextPool(void) {
    delete impl_;
  }

  grn_ctx *ContextPool::pull(void) {
    MRN_DBUG_ENTER_METHOD();
    grn_ctx *ctx = impl_->pull();
    DBUG_RETURN(ctx);
  }

  void ContextPool::release(grn_ctx *ctx) {
    MRN_DBUG_ENTER_METHOD();
    impl_->release(ctx);
    DBUG_VOID_RETURN;
  }

  void ContextPool::clear(void) {
    MRN_DBUG_ENTER_METHOD();
    impl_->clear();
    DBUG_VOID_RETURN;
  }
}
