// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/WlanFrame.h>

namespace Vector {
namespace BLF {

WlanFrame::WlanFrame() :
    ObjectHeader(ObjectType::WLAN_FRAME) {
}

void WlanFrame::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&radioChannel), sizeof(radioChannel));
    is.read(reinterpret_cast<char *>(&signalStrength), sizeof(signalStrength));
    is.read(reinterpret_cast<char *>(&signalQuality), sizeof(signalQuality));
    is.read(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    is.read(reinterpret_cast<char *>(&reservedWlanFrame), sizeof(reservedWlanFrame));
    frameData.resize(frameLength);
    is.read(reinterpret_cast<char *>(frameData.data()), frameLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void WlanFrame::write(AbstractFile & os) {
    /* pre processing */
    frameLength = static_cast<uint16_t>(frameData.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&radioChannel), sizeof(radioChannel));
    os.write(reinterpret_cast<char *>(&signalStrength), sizeof(signalStrength));
    os.write(reinterpret_cast<char *>(&signalQuality), sizeof(signalQuality));
    os.write(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    os.write(reinterpret_cast<char *>(&reservedWlanFrame), sizeof(reservedWlanFrame));
    os.write(reinterpret_cast<char *>(frameData.data()), frameLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t WlanFrame::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(flags) +
        sizeof(dir) +
        sizeof(radioChannel) +
        sizeof(signalStrength) +
        sizeof(signalQuality) +
        sizeof(frameLength) +
        sizeof(reservedWlanFrame) +
        frameLength;
}

}
}
