// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostCtrl.h>

namespace Vector {
namespace BLF {

MostCtrl::MostCtrl() :
    ObjectHeader(ObjectType::MOST_CTRL) {
}

void MostCtrl::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedMostCtrl1), sizeof(reservedMostCtrl1));
    is.read(reinterpret_cast<char *>(&sourceAdr), sizeof(sourceAdr));
    is.read(reinterpret_cast<char *>(&destAdr), sizeof(destAdr));
    is.read(reinterpret_cast<char *>(msg.data()), static_cast<std::streamsize>(msg.size()));
    is.read(reinterpret_cast<char *>(&reservedMostCtrl2), sizeof(reservedMostCtrl2));
    is.read(reinterpret_cast<char *>(&rTyp), sizeof(rTyp));
    is.read(reinterpret_cast<char *>(&rTypAdr), sizeof(rTypAdr));
    is.read(reinterpret_cast<char *>(&state), sizeof(state));
    is.read(reinterpret_cast<char *>(&reservedMostCtrl3), sizeof(reservedMostCtrl3));
    is.read(reinterpret_cast<char *>(&ackNack), sizeof(ackNack));
    is.read(reinterpret_cast<char *>(&reservedMostCtrl4), sizeof(reservedMostCtrl4));
}

void MostCtrl::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedMostCtrl1), sizeof(reservedMostCtrl1));
    os.write(reinterpret_cast<char *>(&sourceAdr), sizeof(sourceAdr));
    os.write(reinterpret_cast<char *>(&destAdr), sizeof(destAdr));
    os.write(reinterpret_cast<char *>(msg.data()), static_cast<std::streamsize>(msg.size()));
    os.write(reinterpret_cast<char *>(&reservedMostCtrl2), sizeof(reservedMostCtrl2));
    os.write(reinterpret_cast<char *>(&rTyp), sizeof(rTyp));
    os.write(reinterpret_cast<char *>(&rTypAdr), sizeof(rTypAdr));
    os.write(reinterpret_cast<char *>(&state), sizeof(state));
    os.write(reinterpret_cast<char *>(&reservedMostCtrl3), sizeof(reservedMostCtrl3));
    os.write(reinterpret_cast<char *>(&ackNack), sizeof(ackNack));
    os.write(reinterpret_cast<char *>(&reservedMostCtrl4), sizeof(reservedMostCtrl4));
}

uint32_t MostCtrl::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(reservedMostCtrl1) +
        sizeof(sourceAdr) +
        sizeof(destAdr) +
        static_cast<uint32_t>(msg.size()) +
        sizeof(reservedMostCtrl2) +
        sizeof(rTyp) +
        sizeof(rTypAdr) +
        sizeof(state) +
        sizeof(reservedMostCtrl3) +
        sizeof(ackNack) +
        sizeof(reservedMostCtrl4);
}

}
}
