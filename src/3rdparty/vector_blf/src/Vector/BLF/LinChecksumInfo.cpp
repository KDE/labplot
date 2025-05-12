// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinChecksumInfo.h>

namespace Vector {
namespace BLF {

LinChecksumInfo::LinChecksumInfo() :
    ObjectHeader(ObjectType::LIN_CHECKSUM_INFO) {
}

void LinChecksumInfo::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&checksumModel), sizeof(checksumModel));
    is.read(reinterpret_cast<char *>(&reservedLinChecksumInfo), sizeof(reservedLinChecksumInfo));
}

void LinChecksumInfo::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&checksumModel), sizeof(checksumModel));
    os.write(reinterpret_cast<char *>(&reservedLinChecksumInfo), sizeof(reservedLinChecksumInfo));
}

uint32_t LinChecksumInfo::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(id) +
        sizeof(checksumModel) +
        sizeof(reservedLinChecksumInfo);
}

}
}
