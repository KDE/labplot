// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinCrcError2.h>

namespace Vector {
namespace BLF {

LinCrcError2::LinCrcError2() :
    ObjectHeader(ObjectType::LIN_CRC_ERROR2) {
}

void LinCrcError2::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinDatabyteTimestampEvent::read(is);
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    is.read(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    is.read(reinterpret_cast<char *>(&simulated), sizeof(simulated));
    is.read(reinterpret_cast<char *>(&reservedLinCrcError1), sizeof(reservedLinCrcError1));
    is.read(reinterpret_cast<char *>(&respBaudrate), sizeof(respBaudrate));
    is.read(reinterpret_cast<char *>(&reservedLinCrcError2), sizeof(reservedLinCrcError2));
    is.read(reinterpret_cast<char *>(&exactHeaderBaudrate), sizeof(exactHeaderBaudrate));
    is.read(reinterpret_cast<char *>(&earlyStopbitOffset), sizeof(earlyStopbitOffset));
    is.read(reinterpret_cast<char *>(&earlyStopbitOffsetResponse), sizeof(earlyStopbitOffsetResponse));
    // @note might be extended in future versions
}

void LinCrcError2::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinDatabyteTimestampEvent::write(os);
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    os.write(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    os.write(reinterpret_cast<char *>(&simulated), sizeof(simulated));
    os.write(reinterpret_cast<char *>(&reservedLinCrcError1), sizeof(reservedLinCrcError1));
    os.write(reinterpret_cast<char *>(&respBaudrate), sizeof(respBaudrate));
    os.write(reinterpret_cast<char *>(&reservedLinCrcError2), sizeof(reservedLinCrcError2));
    os.write(reinterpret_cast<char *>(&exactHeaderBaudrate), sizeof(exactHeaderBaudrate));
    os.write(reinterpret_cast<char *>(&earlyStopbitOffset), sizeof(earlyStopbitOffset));
    os.write(reinterpret_cast<char *>(&earlyStopbitOffsetResponse), sizeof(earlyStopbitOffsetResponse));
}

uint32_t LinCrcError2::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinDatabyteTimestampEvent::calculateObjectSize() +
        static_cast<uint32_t>(data.size()) +
        sizeof(crc) +
        sizeof(dir) +
        sizeof(fsmId) +
        sizeof(fsmState) +
        sizeof(simulated) +
        sizeof(reservedLinCrcError1) +
        sizeof(respBaudrate) +
        sizeof(reservedLinCrcError2) +
        sizeof(exactHeaderBaudrate) +
        sizeof(earlyStopbitOffset) +
        sizeof(earlyStopbitOffsetResponse);
}

}
}
