// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/A429Error.h>

namespace Vector {
namespace BLF {

A429Error::A429Error() :
    ObjectHeader(ObjectType::A429_ERROR) {
}

void A429Error::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&errorType), sizeof(errorType));
    is.read(reinterpret_cast<char *>(&sourceIdentifier), sizeof(sourceIdentifier));
    is.read(reinterpret_cast<char *>(&errReason), sizeof(errReason));
    is.read(reinterpret_cast<char *>(errorText.data()), static_cast<std::streamsize>(errorText.size()));
    is.read(reinterpret_cast<char *>(errorAttributes.data()), static_cast<std::streamsize>(errorAttributes.size()));
    is.read(reinterpret_cast<char *>(&reservedA429Error), sizeof(reservedA429Error));
}

void A429Error::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&errorType), sizeof(errorType));
    os.write(reinterpret_cast<char *>(&sourceIdentifier), sizeof(sourceIdentifier));
    os.write(reinterpret_cast<char *>(&errReason), sizeof(errReason));
    os.write(reinterpret_cast<char *>(errorText.data()), static_cast<std::streamsize>(errorText.size()));
    os.write(reinterpret_cast<char *>(errorAttributes.data()), static_cast<std::streamsize>(errorAttributes.size()));
    os.write(reinterpret_cast<char *>(&reservedA429Error), sizeof(reservedA429Error));
}

uint32_t A429Error::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(errorType) +
        sizeof(sourceIdentifier) +
        sizeof(errReason) +
        static_cast<uint32_t>(errorText.size()) +
        static_cast<uint32_t>(errorAttributes.size()) +
        sizeof(reservedA429Error);
}

}
}
