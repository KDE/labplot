// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinMessageDescriptor.h>

namespace Vector {
namespace BLF {

void LinMessageDescriptor::read(AbstractFile & is) {
    LinSynchFieldEvent::read(is);
    is.read(reinterpret_cast<char *>(&supplierId), sizeof(supplierId));
    is.read(reinterpret_cast<char *>(&messageId), sizeof(messageId));
    is.read(reinterpret_cast<char *>(&nad), sizeof(nad));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    is.read(reinterpret_cast<char *>(&checksumModel), sizeof(checksumModel));
}

void LinMessageDescriptor::write(AbstractFile & os) {
    LinSynchFieldEvent::write(os);
    os.write(reinterpret_cast<char *>(&supplierId), sizeof(supplierId));
    os.write(reinterpret_cast<char *>(&messageId), sizeof(messageId));
    os.write(reinterpret_cast<char *>(&nad), sizeof(nad));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    os.write(reinterpret_cast<char *>(&checksumModel), sizeof(checksumModel));
}

uint32_t LinMessageDescriptor::calculateObjectSize() const {
    return
        LinSynchFieldEvent::calculateObjectSize() +
        sizeof(supplierId) +
        sizeof(messageId) +
        sizeof(nad) +
        sizeof(id) +
        sizeof(dlc) +
        sizeof(checksumModel);
}

}
}
