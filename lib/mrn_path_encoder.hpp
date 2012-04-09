/* -*- c-basic-offset: 2 -*- */
/*
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

#ifndef MRN_PATH_ENCODER_HPP_
#define MRN_PATH_ENCODER_HPP_

#include "mrn_sys.hpp"

namespace mrn {
  class PathEncoder {
  public:
    PathEncoder(const char *name);
    const char *path();
  private:
    const char *name_;
    char path_[MRN_MAX_PATH_SIZE];

    uint encode(char *buf_st, char *buf_ed, const char *st, const char *ed);
  };
}

#endif /* MRN_PATH_ENCODER_HPP_ */
