// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstring>

#include <Vector/BLF/EventComment.h>

namespace Vector {
namespace BLF {

EventComment::EventComment() :
    ObjectHeader(ObjectType::EVENT_COMMENT) {
}

void EventComment::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&commentedEventType), sizeof(commentedEventType));
    is.read(reinterpret_cast<char *>(&textLength), sizeof(textLength));
    is.read(reinterpret_cast<char *>(&reservedEventComment), sizeof(reservedEventComment));
    text.resize(textLength);
    is.read(const_cast<char *>(text.data()), textLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void EventComment::write(AbstractFile & os) {
    /* pre processing */
    textLength = static_cast<uint32_t>(text.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&commentedEventType), sizeof(commentedEventType));
    os.write(reinterpret_cast<char *>(&textLength), sizeof(textLength));
    os.write(reinterpret_cast<char *>(&reservedEventComment), sizeof(reservedEventComment));
    os.write(const_cast<char *>(text.data()), textLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t EventComment::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(commentedEventType) +
        sizeof(textLength) +
        sizeof(reservedEventComment) +
        textLength;
}

}
}
