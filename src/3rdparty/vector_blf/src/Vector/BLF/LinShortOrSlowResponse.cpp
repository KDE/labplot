// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinShortOrSlowResponse.h>

namespace Vector {
namespace BLF {

LinShortOrSlowResponse::LinShortOrSlowResponse() :
    ObjectHeader(ObjectType::LIN_SHORT_OR_SLOW_RESPONSE) {
}

void LinShortOrSlowResponse::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinDatabyteTimestampEvent::read(is);
    is.read(reinterpret_cast<char *>(&numberOfRespBytes), sizeof(numberOfRespBytes));
    is.read(reinterpret_cast<char *>(respBytes.data()), static_cast<std::streamsize>(respBytes.size()));
    is.read(reinterpret_cast<char *>(&slowResponse), sizeof(slowResponse));
    is.read(reinterpret_cast<char *>(&interruptedByBreak), sizeof(interruptedByBreak));
    is.read(reinterpret_cast<char *>(&reservedLinShortOrSlowResponse), sizeof(reservedLinShortOrSlowResponse));
    // @note might be extended in future versions
}

void LinShortOrSlowResponse::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinDatabyteTimestampEvent::write(os);
    os.write(reinterpret_cast<char *>(&numberOfRespBytes), sizeof(numberOfRespBytes));
    os.write(reinterpret_cast<char *>(respBytes.data()), static_cast<std::streamsize>(respBytes.size()));
    os.write(reinterpret_cast<char *>(&slowResponse), sizeof(slowResponse));
    os.write(reinterpret_cast<char *>(&interruptedByBreak), sizeof(interruptedByBreak));
    os.write(reinterpret_cast<char *>(&reservedLinShortOrSlowResponse), sizeof(reservedLinShortOrSlowResponse));
}

uint32_t LinShortOrSlowResponse::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinDatabyteTimestampEvent::calculateObjectSize() +
        sizeof(numberOfRespBytes) +
        static_cast<uint32_t>(respBytes.size()) +
        sizeof(slowResponse) +
        sizeof(interruptedByBreak) +
        sizeof(reservedLinShortOrSlowResponse);
}

}
}
