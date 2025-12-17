// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/A429BusStatistic.h>

namespace Vector {
namespace BLF {

A429BusStatistic::A429BusStatistic() :
    ObjectHeader(ObjectType::A429_BUS_STATISTIC) {
}

void A429BusStatistic::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedA429BusStatistic), sizeof(reservedA429BusStatistic));
    is.read(reinterpret_cast<char *>(&busload), sizeof(busload));
    is.read(reinterpret_cast<char *>(&dataTotal), sizeof(dataTotal));
    is.read(reinterpret_cast<char *>(&errorTotal), sizeof(errorTotal));
    is.read(reinterpret_cast<char *>(&bitrate), sizeof(bitrate));
    is.read(reinterpret_cast<char *>(&parityErrors), sizeof(parityErrors));
    is.read(reinterpret_cast<char *>(&bitrateErrors), sizeof(bitrateErrors));
    is.read(reinterpret_cast<char *>(&gapErrors), sizeof(gapErrors));
    is.read(reinterpret_cast<char *>(&lineErrors), sizeof(lineErrors));
    is.read(reinterpret_cast<char *>(&formatErrors), sizeof(formatErrors));
    is.read(reinterpret_cast<char *>(&dutyFactorErrors), sizeof(dutyFactorErrors));
    is.read(reinterpret_cast<char *>(&wordLenErrors), sizeof(wordLenErrors));
    is.read(reinterpret_cast<char *>(&codingErrors), sizeof(codingErrors));
    is.read(reinterpret_cast<char *>(&idleErrors), sizeof(idleErrors));
    is.read(reinterpret_cast<char *>(&levelErrors), sizeof(levelErrors));
    is.read(reinterpret_cast<char *>(labelCount.data()), static_cast<std::streamsize>(labelCount.size() * sizeof(uint16_t)));
}

void A429BusStatistic::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedA429BusStatistic), sizeof(reservedA429BusStatistic));
    os.write(reinterpret_cast<char *>(&busload), sizeof(busload));
    os.write(reinterpret_cast<char *>(&dataTotal), sizeof(dataTotal));
    os.write(reinterpret_cast<char *>(&errorTotal), sizeof(errorTotal));
    os.write(reinterpret_cast<char *>(&bitrate), sizeof(bitrate));
    os.write(reinterpret_cast<char *>(&parityErrors), sizeof(parityErrors));
    os.write(reinterpret_cast<char *>(&bitrateErrors), sizeof(bitrateErrors));
    os.write(reinterpret_cast<char *>(&gapErrors), sizeof(gapErrors));
    os.write(reinterpret_cast<char *>(&lineErrors), sizeof(lineErrors));
    os.write(reinterpret_cast<char *>(&formatErrors), sizeof(formatErrors));
    os.write(reinterpret_cast<char *>(&dutyFactorErrors), sizeof(dutyFactorErrors));
    os.write(reinterpret_cast<char *>(&wordLenErrors), sizeof(wordLenErrors));
    os.write(reinterpret_cast<char *>(&codingErrors), sizeof(codingErrors));
    os.write(reinterpret_cast<char *>(&idleErrors), sizeof(idleErrors));
    os.write(reinterpret_cast<char *>(&levelErrors), sizeof(levelErrors));
    os.write(reinterpret_cast<char *>(labelCount.data()), static_cast<std::streamsize>(labelCount.size() * sizeof(uint16_t)));
}

uint32_t A429BusStatistic::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(reservedA429BusStatistic) +
        sizeof(busload) +
        sizeof(dataTotal) +
        sizeof(errorTotal) +
        sizeof(bitrate) +
        sizeof(parityErrors) +
        sizeof(bitrateErrors) +
        sizeof(gapErrors) +
        sizeof(lineErrors) +
        sizeof(formatErrors) +
        sizeof(dutyFactorErrors) +
        sizeof(wordLenErrors) +
        sizeof(codingErrors) +
        sizeof(idleErrors) +
        sizeof(levelErrors) +
        static_cast<uint32_t>(labelCount.size() * sizeof(uint16_t));
}

}
}
