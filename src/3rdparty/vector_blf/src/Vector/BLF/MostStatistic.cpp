// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostStatistic.h>

namespace Vector {
namespace BLF {

MostStatistic::MostStatistic() :
    ObjectHeader(ObjectType::MOST_STATISTIC) {
}

void MostStatistic::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&pktCnt), sizeof(pktCnt));
    is.read(reinterpret_cast<char *>(&frmCnt), sizeof(frmCnt));
    is.read(reinterpret_cast<char *>(&lightCnt), sizeof(lightCnt));
    is.read(reinterpret_cast<char *>(&bufferLevel), sizeof(bufferLevel));
}

void MostStatistic::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&pktCnt), sizeof(pktCnt));
    os.write(reinterpret_cast<char *>(&frmCnt), sizeof(frmCnt));
    os.write(reinterpret_cast<char *>(&lightCnt), sizeof(lightCnt));
    os.write(reinterpret_cast<char *>(&bufferLevel), sizeof(bufferLevel));
}

uint32_t MostStatistic::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(pktCnt) +
        sizeof(frmCnt) +
        sizeof(lightCnt) +
        sizeof(bufferLevel);
}

}
}
