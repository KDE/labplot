// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/AttributeEvent.h>

namespace Vector {
namespace BLF {

AttributeEvent::AttributeEvent() :
    ObjectHeader(ObjectType::ATTRIBUTE_EVENT) {
}

void AttributeEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&mainAttributableObjectPathLength), sizeof(mainAttributableObjectPathLength));
    is.read(reinterpret_cast<char *>(&memberPathLength), sizeof(memberPathLength));
    is.read(reinterpret_cast<char *>(&attributeDefinitionPathLength), sizeof(attributeDefinitionPathLength));
    is.read(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    mainAttributableObjectPath.resize(mainAttributableObjectPathLength);
    is.read(const_cast<char *>(mainAttributableObjectPath.data()), mainAttributableObjectPathLength);
    memberPath.resize(memberPathLength);
    is.read(const_cast<char *>(memberPath.data()), memberPathLength);
    attributeDefinitionPath.resize(attributeDefinitionPathLength);
    is.read(const_cast<char *>(attributeDefinitionPath.data()), attributeDefinitionPathLength);
    data.resize(dataLength);
    is.read(reinterpret_cast<char *>(data.data()), dataLength);
}

void AttributeEvent::write(AbstractFile & os) {
    /* pre processing */
    mainAttributableObjectPathLength = static_cast<uint16_t>(mainAttributableObjectPath.size());
    memberPathLength = static_cast<uint32_t>(memberPath.size());
    attributeDefinitionPathLength = static_cast<uint16_t>(attributeDefinitionPath.size());
    dataLength = static_cast<uint32_t>(data.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&mainAttributableObjectPathLength), sizeof(mainAttributableObjectPathLength));
    os.write(reinterpret_cast<char *>(&memberPathLength), sizeof(memberPathLength));
    os.write(reinterpret_cast<char *>(&attributeDefinitionPathLength), sizeof(attributeDefinitionPathLength));
    os.write(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    os.write(const_cast<char *>(mainAttributableObjectPath.data()), mainAttributableObjectPathLength);
    os.write(const_cast<char *>(memberPath.data()), memberPathLength);
    os.write(const_cast<char *>(attributeDefinitionPath.data()), attributeDefinitionPathLength);
    os.write(reinterpret_cast<char *>(data.data()), dataLength);
}

uint32_t AttributeEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(mainAttributableObjectPathLength) +
        sizeof(memberPathLength) +
        sizeof(attributeDefinitionPathLength) +
        sizeof(dataLength) +
        mainAttributableObjectPathLength +
        memberPathLength +
        attributeDefinitionPathLength +
        dataLength;
}

}
}
