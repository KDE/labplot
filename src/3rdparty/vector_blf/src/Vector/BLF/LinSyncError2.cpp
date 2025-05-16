// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinSyncError2.h>

namespace Vector {
namespace BLF {

LinSyncError2::LinSyncError2() :
    ObjectHeader(ObjectType::LIN_SYN_ERROR2) {
}

void LinSyncError2::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinSynchFieldEvent::read(is);
    is.read(reinterpret_cast<char *>(timeDiff.data()), static_cast<std::streamsize>(timeDiff.size() * sizeof(uint16_t)));
    // @note might be extended in future versions
}

void LinSyncError2::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinSynchFieldEvent::write(os);
    os.write(reinterpret_cast<char *>(timeDiff.data()), static_cast<std::streamsize>(timeDiff.size() * sizeof(uint16_t)));
}

uint32_t LinSyncError2::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinSynchFieldEvent::calculateObjectSize() +
        static_cast<uint32_t>(timeDiff.size() * sizeof(uint16_t));
}

}
}
