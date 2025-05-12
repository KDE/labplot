// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <codecvt>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <locale>

#include <Vector/BLF.h>

int main(int argc, char * argv[]) {
    if (argc != 2) {
        std::cout << "Write-Example <filename.blf>" << std::endl;
        return -1;
    }

    /* open file for writing */
    Vector::BLF::File file;
    file.open(argv[1], std::ios_base::out);
    if (!file.is_open()) {
        std::cout << "Unable to open file" << std::endl;
        return -1;
    }

    /* write a CanMessage */
    auto * canMessage = new Vector::BLF::CanMessage;
    canMessage->channel = 1;
    canMessage->flags = 1; // TX
    canMessage->dlc = 2;
    canMessage->id = 0x33;
    canMessage->data[0] = 0x44;
    canMessage->data[1] = 0x55;
    file.write(canMessage);

    /* close file */
    file.close();

    return 0;
}
