// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinCrcError.h>

namespace Vector {
namespace BLF {

LinCrcError::LinCrcError() :
    ObjectHeader(ObjectType::LIN_CRC_ERROR) {
}

void LinCrcError::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    is.read(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    is.read(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    is.read(reinterpret_cast<char *>(&headerTime), sizeof(headerTime));
    is.read(reinterpret_cast<char *>(&fullTime), sizeof(fullTime));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedLinCrcError1), sizeof(reservedLinCrcError1));
    is.read(reinterpret_cast<char *>(&reservedLinCrcError2), sizeof(reservedLinCrcError2));
}

void LinCrcError::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    os.write(reinterpret_cast<char *>(&fsmId), sizeof(fsmId));
    os.write(reinterpret_cast<char *>(&fsmState), sizeof(fsmState));
    os.write(reinterpret_cast<char *>(&headerTime), sizeof(headerTime));
    os.write(reinterpret_cast<char *>(&fullTime), sizeof(fullTime));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedLinCrcError1), sizeof(reservedLinCrcError1));
    os.write(reinterpret_cast<char *>(&reservedLinCrcError2), sizeof(reservedLinCrcError2));
}

uint32_t LinCrcError::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(id) +
        sizeof(dlc) +
        static_cast<uint32_t>(data.size()) +
        sizeof(fsmId) +
        sizeof(fsmState) +
        sizeof(headerTime) +
        sizeof(fullTime) +
        sizeof(crc) +
        sizeof(dir) +
        sizeof(reservedLinCrcError1) +
        sizeof(reservedLinCrcError2);
}

}
}
