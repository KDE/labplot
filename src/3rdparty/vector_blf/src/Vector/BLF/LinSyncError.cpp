// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinSyncError.h>

namespace Vector {
namespace BLF {

LinSyncError::LinSyncError() :
    ObjectHeader(ObjectType::LIN_SYN_ERROR) {
}

void LinSyncError::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedLinSyncError1), sizeof(reservedLinSyncError1));
    is.read(reinterpret_cast<char *>(timeDiff.data()), static_cast<std::streamsize>(timeDiff.size() * sizeof(uint16_t)));
    is.read(reinterpret_cast<char *>(&reservedLinSyncError2), sizeof(reservedLinSyncError2));
}

void LinSyncError::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedLinSyncError1), sizeof(reservedLinSyncError1));
    os.write(reinterpret_cast<char *>(timeDiff.data()), static_cast<std::streamsize>(timeDiff.size() * sizeof(uint16_t)));
    os.write(reinterpret_cast<char *>(&reservedLinSyncError2), sizeof(reservedLinSyncError2));
}

uint32_t LinSyncError::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedLinSyncError1) +
        static_cast<uint32_t>(timeDiff.size() * sizeof(uint16_t)) +
        sizeof(reservedLinSyncError2);
}

}
}
