// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/AfdxStatus.h>

namespace Vector {
namespace BLF {

void AfdxLineStatus::read(AbstractFile & is) {
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&linkStatus), sizeof(linkStatus));
    is.read(reinterpret_cast<char *>(&ethernetPhy), sizeof(ethernetPhy));
    is.read(reinterpret_cast<char *>(&duplex), sizeof(duplex));
    is.read(reinterpret_cast<char *>(&mdi), sizeof(mdi));
    is.read(reinterpret_cast<char *>(&connector), sizeof(connector));
    is.read(reinterpret_cast<char *>(&clockMode), sizeof(clockMode));
    is.read(reinterpret_cast<char *>(&pairs), sizeof(pairs));
    is.read(reinterpret_cast<char *>(&reservedAfdxLineStatus1), sizeof(reservedAfdxLineStatus1));
    is.read(reinterpret_cast<char *>(&reservedAfdxLineStatus2), sizeof(reservedAfdxLineStatus2));
    is.read(reinterpret_cast<char *>(&bitrate), sizeof(bitrate));
    // @note might be extended in future versions
}

void AfdxLineStatus::write(AbstractFile & os) {
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&linkStatus), sizeof(linkStatus));
    os.write(reinterpret_cast<char *>(&ethernetPhy), sizeof(ethernetPhy));
    os.write(reinterpret_cast<char *>(&duplex), sizeof(duplex));
    os.write(reinterpret_cast<char *>(&mdi), sizeof(mdi));
    os.write(reinterpret_cast<char *>(&connector), sizeof(connector));
    os.write(reinterpret_cast<char *>(&clockMode), sizeof(clockMode));
    os.write(reinterpret_cast<char *>(&pairs), sizeof(pairs));
    os.write(reinterpret_cast<char *>(&reservedAfdxLineStatus1), sizeof(reservedAfdxLineStatus1));
    os.write(reinterpret_cast<char *>(&reservedAfdxLineStatus2), sizeof(reservedAfdxLineStatus2));
    os.write(reinterpret_cast<char *>(&bitrate), sizeof(bitrate));
}

uint32_t AfdxLineStatus::calculateObjectSize() const {
    return
        sizeof(flags) +
        sizeof(linkStatus) +
        sizeof(ethernetPhy) +
        sizeof(duplex) +
        sizeof(mdi) +
        sizeof(connector) +
        sizeof(clockMode) +
        sizeof(pairs) +
        sizeof(reservedAfdxLineStatus1) +
        sizeof(reservedAfdxLineStatus2) +
        sizeof(bitrate);
}

}
}
