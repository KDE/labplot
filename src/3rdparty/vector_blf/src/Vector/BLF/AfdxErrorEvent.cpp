// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/AfdxErrorEvent.h>

namespace Vector {
namespace BLF {

AfdxErrorEvent::AfdxErrorEvent() :
    ObjectHeader(ObjectType::AFDX_ERROR_EVENT) {
}

void AfdxErrorEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&errorLevel), sizeof(errorLevel));
    is.read(reinterpret_cast<char *>(&sourceIdentifier), sizeof(sourceIdentifier));
    is.read(reinterpret_cast<char *>(errorText.data()), static_cast<std::streamsize>(errorText.size()));
    is.read(reinterpret_cast<char *>(errorAttributes.data()), static_cast<std::streamsize>(errorAttributes.size()));
    // @note might be extended in future versions
}

void AfdxErrorEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&errorLevel), sizeof(errorLevel));
    os.write(reinterpret_cast<char *>(&sourceIdentifier), sizeof(sourceIdentifier));
    os.write(reinterpret_cast<char *>(errorText.data()), static_cast<std::streamsize>(errorText.size()));
    os.write(reinterpret_cast<char *>(errorAttributes.data()), static_cast<std::streamsize>(errorAttributes.size()));
}

uint32_t AfdxErrorEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(errorLevel) +
        sizeof(sourceIdentifier) +
        errorText.size() +
        errorAttributes.size();
}

}
}
