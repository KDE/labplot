// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostSpy.h>

namespace Vector {
namespace BLF {

MostSpy::MostSpy() :
    ObjectHeader(ObjectType::MOST_SPY) {
}

void MostSpy::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedMostSpy1), sizeof(reservedMostSpy1));
    is.read(reinterpret_cast<char *>(&sourceAdr), sizeof(sourceAdr));
    is.read(reinterpret_cast<char *>(&destAdr), sizeof(destAdr));
    is.read(reinterpret_cast<char *>(msg.data()), static_cast<std::streamsize>(msg.size()));
    is.read(reinterpret_cast<char *>(&reservedMostSpy2), sizeof(reservedMostSpy2));
    is.read(reinterpret_cast<char *>(&rTyp), sizeof(rTyp));
    is.read(reinterpret_cast<char *>(&rTypAdr), sizeof(rTypAdr));
    is.read(reinterpret_cast<char *>(&state), sizeof(state));
    is.read(reinterpret_cast<char *>(&reservedMostSpy3), sizeof(reservedMostSpy3));
    is.read(reinterpret_cast<char *>(&ackNack), sizeof(ackNack));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
}

void MostSpy::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedMostSpy1), sizeof(reservedMostSpy1));
    os.write(reinterpret_cast<char *>(&sourceAdr), sizeof(sourceAdr));
    os.write(reinterpret_cast<char *>(&destAdr), sizeof(destAdr));
    os.write(reinterpret_cast<char *>(msg.data()), static_cast<std::streamsize>(msg.size()));
    os.write(reinterpret_cast<char *>(&reservedMostSpy2), sizeof(reservedMostSpy2));
    os.write(reinterpret_cast<char *>(&rTyp), sizeof(rTyp));
    os.write(reinterpret_cast<char *>(&rTypAdr), sizeof(rTypAdr));
    os.write(reinterpret_cast<char *>(&state), sizeof(state));
    os.write(reinterpret_cast<char *>(&reservedMostSpy3), sizeof(reservedMostSpy3));
    os.write(reinterpret_cast<char *>(&ackNack), sizeof(ackNack));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
}

uint32_t MostSpy::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(reservedMostSpy1) +
        sizeof(sourceAdr) +
        sizeof(destAdr) +
        static_cast<uint32_t>(msg.size()) +
        sizeof(reservedMostSpy2) +
        sizeof(rTyp) +
        sizeof(rTypAdr) +
        sizeof(state) +
        sizeof(reservedMostSpy3) +
        sizeof(ackNack) +
        sizeof(crc);
}

}
}
