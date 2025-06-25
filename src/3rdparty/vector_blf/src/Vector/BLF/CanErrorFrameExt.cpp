// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanErrorFrameExt.h>

namespace Vector {
namespace BLF {

CanErrorFrameExt::CanErrorFrameExt() :
    ObjectHeader(ObjectType::CAN_ERROR_EXT) {
}

void CanErrorFrameExt::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&length), sizeof(length));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&ecc), sizeof(ecc));
    is.read(reinterpret_cast<char *>(&position), sizeof(position));
    is.read(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    is.read(reinterpret_cast<char *>(&reservedCanErrorFrameExt1), sizeof(reservedCanErrorFrameExt1));
    is.read(reinterpret_cast<char *>(&frameLengthInNs), sizeof(frameLengthInNs));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&flagsExt), sizeof(flagsExt));
    is.read(reinterpret_cast<char *>(&reservedCanErrorFrameExt2), sizeof(reservedCanErrorFrameExt2));
    data.resize(objectSize - calculateObjectSize()); // all remaining data
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
}

void CanErrorFrameExt::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&length), sizeof(length));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&ecc), sizeof(ecc));
    os.write(reinterpret_cast<char *>(&position), sizeof(position));
    os.write(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    os.write(reinterpret_cast<char *>(&reservedCanErrorFrameExt1), sizeof(reservedCanErrorFrameExt1));
    os.write(reinterpret_cast<char *>(&frameLengthInNs), sizeof(frameLengthInNs));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&flagsExt), sizeof(flagsExt));
    os.write(reinterpret_cast<char *>(&reservedCanErrorFrameExt2), sizeof(reservedCanErrorFrameExt2));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
}

uint32_t CanErrorFrameExt::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(length) +
        sizeof(flags) +
        sizeof(ecc) +
        sizeof(position) +
        sizeof(dlc) +
        sizeof(reservedCanErrorFrameExt1) +
        sizeof(frameLengthInNs) +
        sizeof(id) +
        sizeof(flagsExt) +
        sizeof(reservedCanErrorFrameExt2) +
        static_cast<uint32_t>(data.size());
}

}
}
