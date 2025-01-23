// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstring>

#include <Vector/BLF/AppText.h>

namespace Vector {
namespace BLF {

AppText::AppText() :
    ObjectHeader(ObjectType::APP_TEXT) {
}

void AppText::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&source), sizeof(source));
    is.read(reinterpret_cast<char *>(&reservedAppText1), sizeof(reservedAppText1));
    is.read(reinterpret_cast<char *>(&textLength), sizeof(textLength));
    is.read(reinterpret_cast<char *>(&reservedAppText2), sizeof(reservedAppText2));
    text.resize(textLength);
    is.read(const_cast<char *>(text.data()), textLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void AppText::write(AbstractFile & os) {
    /* pre processing */
    textLength = static_cast<uint32_t>(text.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&source), sizeof(source));
    os.write(reinterpret_cast<char *>(&reservedAppText1), sizeof(reservedAppText1));
    os.write(reinterpret_cast<char *>(&textLength), sizeof(textLength));
    os.write(reinterpret_cast<char *>(&reservedAppText2), sizeof(reservedAppText2));
    os.write(const_cast<char *>(text.data()), textLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t AppText::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(source) +
        sizeof(reservedAppText1) +
        sizeof(textLength) +
        sizeof(reservedAppText2) +
        textLength;
}

}
}
