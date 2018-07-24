/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2018  Kouhei Sutou <kou@clear-code.com>

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

#include "mrn_buffers.hpp"

namespace mrn {
  Buffers::Buffers(grn_ctx *ctx)
    : ctx_(ctx),
      n_(0) {
    GRN_TEXT_INIT(&buffers_, 0);
  }

  Buffers::~Buffers() {
    for (size_t i = 0; i < n_; ++i) {
      grn_obj *buffer = (*this)[i];
      GRN_OBJ_FIN(ctx_, buffer);
    }
    GRN_OBJ_FIN(ctx_, &buffers_);
  }

  void Buffers::resize(size_t n) {
    if (n_ != n) {
      for (size_t i = 0; i < n_; ++i) {
        grn_obj *buffer = (*this)[i];
        GRN_OBJ_FIN(ctx_, buffer);
      }
      grn_bulk_reserve(ctx_, &buffers_, sizeof(grn_obj) * n);
      n_ = n;
      for (size_t i = 0; i < n_; ++i) {
        grn_obj *buffer = (*this)[i];
        GRN_TEXT_INIT(buffer, 0);
      }
    }
  }

  grn_obj *Buffers::operator[](size_t i) const {
    if (i >= n_) {
      return NULL;
    }
    return(&(reinterpret_cast<grn_obj *>(GRN_BULK_HEAD(&buffers_))[i]));
  }
}
