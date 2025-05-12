// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/SingleByteSerialEvent.h>

namespace Vector {
namespace BLF {


void SingleByteSerialEvent::read(AbstractFile & is) {
    is.read(reinterpret_cast<char *>(&byte), sizeof(byte));
    is.seekg(15, std::ios_base::cur); // due to union
    // @note might be extended in future versions
}

void SingleByteSerialEvent::write(AbstractFile & os) {
    os.write(reinterpret_cast<char *>(&byte), sizeof(byte));
    os.skipp(15); // due to union
}

uint32_t SingleByteSerialEvent::calculateObjectSize() const {
    return sizeof(byte);
}

}
}
