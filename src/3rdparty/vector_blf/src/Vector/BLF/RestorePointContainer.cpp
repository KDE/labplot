// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/RestorePointContainer.h>

namespace Vector {
namespace BLF {

RestorePointContainer::RestorePointContainer() :
    ObjectHeader(ObjectType::Unknown115) {
}

void RestorePointContainer::read(AbstractFile & is) {
    ObjectHeader::read(is);

    is.read(reinterpret_cast<char *>(reservedRestorePointContainer.data()), static_cast<std::streamsize>(reservedRestorePointContainer.size()));
    is.read(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    data.resize(dataLength);
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
}

void RestorePointContainer::write(AbstractFile & os) {
    /* pre processing */
    dataLength = static_cast<uint16_t>(data.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(reservedRestorePointContainer.data()), static_cast<std::streamsize>(reservedRestorePointContainer.size()));
    os.write(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
}

uint32_t RestorePointContainer::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        static_cast<uint32_t>(reservedRestorePointContainer.size()) +
        sizeof(dataLength) +
        dataLength;
}

}
}
