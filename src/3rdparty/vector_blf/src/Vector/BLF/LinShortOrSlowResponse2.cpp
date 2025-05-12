// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinShortOrSlowResponse2.h>

namespace Vector {
namespace BLF {

LinShortOrSlowResponse2::LinShortOrSlowResponse2() :
    ObjectHeader(ObjectType::LIN_SHORT_OR_SLOW_RESPONSE2) {
}

void LinShortOrSlowResponse2::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinDatabyteTimestampEvent::read(is);
    is.read(reinterpret_cast<char *>(&numberOfRespBytes), sizeof(numberOfRespBytes));
    is.read(reinterpret_cast<char *>(respBytes.data()), static_cast<std::streamsize>(respBytes.size()));
    is.read(reinterpret_cast<char *>(&slowResponse), sizeof(slowResponse));
    is.read(reinterpret_cast<char *>(&interruptedByBreak), sizeof(interruptedByBreak));
    is.read(reinterpret_cast<char *>(&reservedLinShortOrSlowResponse1), sizeof(reservedLinShortOrSlowResponse1));
    is.read(reinterpret_cast<char *>(&exactHeaderBaudrate), sizeof(exactHeaderBaudrate));
    is.read(reinterpret_cast<char *>(&earlyStopbitOffset), sizeof(earlyStopbitOffset));
    is.read(reinterpret_cast<char *>(&reservedLinShortOrSlowResponse2), sizeof(reservedLinShortOrSlowResponse2));
}

void LinShortOrSlowResponse2::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinDatabyteTimestampEvent::write(os);
    os.write(reinterpret_cast<char *>(&numberOfRespBytes), sizeof(numberOfRespBytes));
    os.write(reinterpret_cast<char *>(respBytes.data()), static_cast<std::streamsize>(respBytes.size()));
    os.write(reinterpret_cast<char *>(&slowResponse), sizeof(slowResponse));
    os.write(reinterpret_cast<char *>(&interruptedByBreak), sizeof(interruptedByBreak));
    os.write(reinterpret_cast<char *>(&reservedLinShortOrSlowResponse1), sizeof(reservedLinShortOrSlowResponse1));
    os.write(reinterpret_cast<char *>(&exactHeaderBaudrate), sizeof(exactHeaderBaudrate));
    os.write(reinterpret_cast<char *>(&earlyStopbitOffset), sizeof(earlyStopbitOffset));
    os.write(reinterpret_cast<char *>(&reservedLinShortOrSlowResponse2), sizeof(reservedLinShortOrSlowResponse2));
}

uint32_t LinShortOrSlowResponse2::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinDatabyteTimestampEvent::calculateObjectSize() +
        sizeof(numberOfRespBytes) +
        static_cast<uint32_t>(respBytes.size()) +
        sizeof(slowResponse) +
        sizeof(interruptedByBreak) +
        sizeof(reservedLinShortOrSlowResponse1) +
        sizeof(exactHeaderBaudrate) +
        sizeof(earlyStopbitOffset) +
        sizeof(reservedLinShortOrSlowResponse2);
}

}
}
