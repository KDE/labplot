// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanFdExtFrameData.h>

namespace Vector {
namespace BLF {

void CanFdExtFrameData::read(AbstractFile & is) {
    is.read(reinterpret_cast<char *>(&btrExtArb), sizeof(btrExtArb));
    is.read(reinterpret_cast<char *>(&btrExtData), sizeof(btrExtData));
    // @note reservedCanFdExtFrameData is read by CanFdMessage64/CanFdErrorFrame64 due to objectSize known there.
}

void CanFdExtFrameData::write(AbstractFile & os) {
    os.write(reinterpret_cast<char *>(&btrExtArb), sizeof(btrExtArb));
    os.write(reinterpret_cast<char *>(&btrExtData), sizeof(btrExtData));
    os.write(reinterpret_cast<char *>(reservedCanFdExtFrameData.data()), reservedCanFdExtFrameData.size());
}

uint32_t CanFdExtFrameData::calculateObjectSize() const {
    return
        sizeof(btrExtArb) +
        sizeof(btrExtData) +
        static_cast<uint32_t>(reservedCanFdExtFrameData.size());
}

}
}
