// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinMessage2.h>

namespace Vector {
namespace BLF {

LinMessage2::LinMessage2() :
    ObjectHeader(ObjectType::LIN_MESSAGE2, 1) {
}

void LinMessage2::read(AbstractFile & is) {
    apiMajor = 1;
    ObjectHeader::read(is);
    LinDatabyteTimestampEvent::read(is);
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&simulated), sizeof(simulated));
    is.read(reinterpret_cast<char *>(&isEtf), sizeof(isEtf));
    is.read(reinterpret_cast<char *>(&etfAssocIndex), sizeof(etfAssocIndex));
    is.read(reinterpret_cast<char *>(&etfAssocEtfId), sizeof(etfAssocEtfId));
    is.read(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    is.read(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    is.read(reinterpret_cast<char *>(&reservedLinMessage1), sizeof(reservedLinMessage1));
    is.read(reinterpret_cast<char *>(&reservedLinMessage2), sizeof(reservedLinMessage2));

    /* the following variables are only available in Version 2 and above */
    if (objectSize > calculateObjectSize()) {
        apiMajor = 2;
        is.read(reinterpret_cast<char *>(&respBaudrate), sizeof(respBaudrate));
    }

    /* the following variables are only available in Version 3 and above */
    if (objectSize > calculateObjectSize()) {
        apiMajor = 3;
        is.read(reinterpret_cast<char *>(&exactHeaderBaudrate), sizeof(exactHeaderBaudrate));
        is.read(reinterpret_cast<char *>(&earlyStopbitOffset), sizeof(earlyStopbitOffset));
        is.read(reinterpret_cast<char *>(&earlyStopbitOffsetResponse), sizeof(earlyStopbitOffsetResponse));
    }

    // @note might be extended in future versions
}

void LinMessage2::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinDatabyteTimestampEvent::write(os);
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&simulated), sizeof(simulated));
    os.write(reinterpret_cast<char *>(&isEtf), sizeof(isEtf));
    os.write(reinterpret_cast<char *>(&etfAssocIndex), sizeof(etfAssocIndex));
    os.write(reinterpret_cast<char *>(&etfAssocEtfId), sizeof(etfAssocEtfId));
    os.write(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    os.write(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    os.write(reinterpret_cast<char *>(&reservedLinMessage1), sizeof(reservedLinMessage1));
    os.write(reinterpret_cast<char *>(&reservedLinMessage2), sizeof(reservedLinMessage2));

    /* the following variables are only available in Version 2 and above */
    if (apiMajor < 2)
        return;
    os.write(reinterpret_cast<char *>(&respBaudrate), sizeof(respBaudrate));

    /* the following variables are only available in Version 3 and above */
    if (apiMajor < 3)
        return;
    os.write(reinterpret_cast<char *>(&exactHeaderBaudrate), sizeof(exactHeaderBaudrate));
    os.write(reinterpret_cast<char *>(&earlyStopbitOffset), sizeof(earlyStopbitOffset));
    os.write(reinterpret_cast<char *>(&earlyStopbitOffsetResponse), sizeof(earlyStopbitOffsetResponse));
}

uint32_t LinMessage2::calculateObjectSize() const {
    uint32_t size =
        ObjectHeader::calculateObjectSize() +
        LinDatabyteTimestampEvent::calculateObjectSize() +
        static_cast<uint32_t>(data.size()) +
        sizeof(crc) +
        sizeof(dir) +
        sizeof(simulated) +
        sizeof(isEtf) +
        sizeof(etfAssocIndex) +
        sizeof(etfAssocEtfId) +
        sizeof(fsmId) +
        sizeof(fsmState) +
        sizeof(reservedLinMessage1) +
        sizeof(reservedLinMessage2);

    /* the following variables are only available in Version 2 and above */
    if (apiMajor < 2)
        return size;
    size +=
        sizeof(respBaudrate);

    /* the following variables are only available in Version 3 and above */
    if (apiMajor < 3)
        return size;
    size +=
        sizeof(exactHeaderBaudrate) +
        sizeof(earlyStopbitOffset) +
        sizeof(earlyStopbitOffsetResponse);

    return size;
}

}
}
