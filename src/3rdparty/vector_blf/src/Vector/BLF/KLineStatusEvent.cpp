// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/KLineStatusEvent.h>

namespace Vector {
namespace BLF {

KLineStatusEvent::KLineStatusEvent() :
    ObjectHeader(ObjectType::KLINE_STATUSEVENT) {
}

void KLineStatusEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&type), sizeof(type));
    is.read(reinterpret_cast<char *>(&dataLen), sizeof(dataLen));
    is.read(reinterpret_cast<char *>(&port), sizeof(port));
    is.read(reinterpret_cast<char *>(&reservedKLineStatusEvent), sizeof(reservedKLineStatusEvent));
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(uint64_t)));
    // @note might be extended in future versions
}

void KLineStatusEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&type), sizeof(type));
    os.write(reinterpret_cast<char *>(&dataLen), sizeof(dataLen));
    os.write(reinterpret_cast<char *>(&port), sizeof(port));
    os.write(reinterpret_cast<char *>(&reservedKLineStatusEvent), sizeof(reservedKLineStatusEvent));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(uint64_t)));
}

uint32_t KLineStatusEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(type) +
        sizeof(dataLen) +
        sizeof(port) +
        sizeof(reservedKLineStatusEvent) +
        static_cast<uint32_t>(data.size() * sizeof(uint64_t));
}

}
}
