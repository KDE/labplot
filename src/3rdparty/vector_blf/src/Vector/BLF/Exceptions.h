// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <stdexcept>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * Exception class for all possible exceptions the Vector BLF library throws.
 */
class VECTOR_BLF_EXPORT Exception : public std::runtime_error {
  public:
    /**
     * Exception constructor
     *
     * @param[in] arg argument to fill what() return
     */
    explicit Exception(const char * arg) :
        std::runtime_error(arg) {
    }
};

}
}
