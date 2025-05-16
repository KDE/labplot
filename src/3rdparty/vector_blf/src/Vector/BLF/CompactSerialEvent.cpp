// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CompactSerialEvent.h>

namespace Vector {
namespace BLF {

uint32_t CompactSerialEvent::calculateObjectSize() const {
    return
        sizeof(compactLength) +
        static_cast<uint32_t>(compactData.size());
}

void CompactSerialEvent::read(AbstractFile & is) {
    is.read(reinterpret_cast<char *>(&compactLength), sizeof(compactLength));
    is.read(reinterpret_cast<char *>(compactData.data()), static_cast<std::streamsize>(compactData.size()));
    // @note might be extended in future versions
}

void CompactSerialEvent::write(AbstractFile & os) {
    os.write(reinterpret_cast<char *>(&compactLength), sizeof(compactLength));
    os.write(reinterpret_cast<char *>(&compactData), sizeof(compactData));
}

}
}
