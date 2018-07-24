/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2018 Kouhei Sutou <kou@clear-code.com>

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

#include <groonga.h>

namespace mrn {
  class Buffers {
  public:
    Buffers(grn_ctx *ctx);
    ~Buffers();
    void resize(size_t n);
    grn_obj *operator[](size_t i) const;

  private:
    grn_ctx *ctx_;
    size_t n_;
    grn_obj buffers_;

    void free_buffers();
  };
}
