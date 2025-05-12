// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinReceiveError2.h>

namespace Vector {
namespace BLF {

LinReceiveError2::LinReceiveError2() :
    ObjectHeader(ObjectType::LIN_RCV_ERROR2, 1) {
}

void LinReceiveError2::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinDatabyteTimestampEvent::read(is);
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    is.read(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    is.read(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    is.read(reinterpret_cast<char *>(&stateReason), sizeof(stateReason));
    is.read(reinterpret_cast<char *>(&offendingByte), sizeof(offendingByte));
    is.read(reinterpret_cast<char *>(&shortError), sizeof(shortError));
    is.read(reinterpret_cast<char *>(&timeoutDuringDlcDetection), sizeof(timeoutDuringDlcDetection));
    is.read(reinterpret_cast<char *>(&isEtf), sizeof(isEtf));
    is.read(reinterpret_cast<char *>(&hasDatabytes), sizeof(hasDatabytes));
    is.read(reinterpret_cast<char *>(&respBaudrate), sizeof(respBaudrate));
    is.read(reinterpret_cast<char *>(&reservedLinReceiveError), sizeof(reservedLinReceiveError));
    is.read(reinterpret_cast<char *>(&exactHeaderBaudrate), sizeof(exactHeaderBaudrate));
    is.read(reinterpret_cast<char *>(&earlyStopbitOffset), sizeof(earlyStopbitOffset));
    is.read(reinterpret_cast<char *>(&earlyStopbitOffsetResponse), sizeof(earlyStopbitOffsetResponse));
    // @note might be extended in future versions
}

void LinReceiveError2::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinDatabyteTimestampEvent::write(os);
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    os.write(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    os.write(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    os.write(reinterpret_cast<char *>(&stateReason), sizeof(stateReason));
    os.write(reinterpret_cast<char *>(&offendingByte), sizeof(offendingByte));
    os.write(reinterpret_cast<char *>(&shortError), sizeof(shortError));
    os.write(reinterpret_cast<char *>(&timeoutDuringDlcDetection), sizeof(timeoutDuringDlcDetection));
    os.write(reinterpret_cast<char *>(&isEtf), sizeof(isEtf));
    os.write(reinterpret_cast<char *>(&hasDatabytes), sizeof(hasDatabytes));
    os.write(reinterpret_cast<char *>(&respBaudrate), sizeof(respBaudrate));
    os.write(reinterpret_cast<char *>(&reservedLinReceiveError), sizeof(reservedLinReceiveError));
    os.write(reinterpret_cast<char *>(&exactHeaderBaudrate), sizeof(exactHeaderBaudrate));
    os.write(reinterpret_cast<char *>(&earlyStopbitOffset), sizeof(earlyStopbitOffset));
    os.write(reinterpret_cast<char *>(&earlyStopbitOffsetResponse), sizeof(earlyStopbitOffsetResponse));
}

uint32_t LinReceiveError2::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinDatabyteTimestampEvent::calculateObjectSize() +
        static_cast<uint32_t>(data.size()) +
        sizeof(fsmId) +
        sizeof(fsmState) +
        sizeof(stateReason) +
        sizeof(offendingByte) +
        sizeof(shortError) +
        sizeof(timeoutDuringDlcDetection) +
        sizeof(isEtf) +
        sizeof(hasDatabytes) +
        sizeof(respBaudrate) +
        sizeof(reservedLinReceiveError) +
        sizeof(exactHeaderBaudrate) +
        sizeof(earlyStopbitOffset) +
        sizeof(earlyStopbitOffsetResponse);
}

}
}
