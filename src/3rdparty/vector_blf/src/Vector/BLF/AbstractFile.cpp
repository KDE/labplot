// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/AbstractFile.h>

#include <vector>

namespace Vector {
namespace BLF {

void AbstractFile::skipp(std::streamsize s) {
    std::vector<char> zero;
    zero.resize(s);
    write(zero.data(), s);
}

}
}
