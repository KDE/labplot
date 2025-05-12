// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/DiagRequestInterpretation.h>

namespace Vector {
namespace BLF {

DiagRequestInterpretation::DiagRequestInterpretation() :
    ObjectHeader(ObjectType::DIAG_REQUEST_INTERPRETATION) {
}

void DiagRequestInterpretation::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&diagDescriptionHandle), sizeof(diagDescriptionHandle));
    is.read(reinterpret_cast<char *>(&diagVariantHandle), sizeof(diagVariantHandle));
    is.read(reinterpret_cast<char *>(&diagServiceHandle), sizeof(diagServiceHandle));
    is.read(reinterpret_cast<char *>(&ecuQualifierLength), sizeof(ecuQualifierLength));
    is.read(reinterpret_cast<char *>(&variantQualifierLength), sizeof(variantQualifierLength));
    is.read(reinterpret_cast<char *>(&serviceQualifierLength), sizeof(serviceQualifierLength));
    ecuQualifier.resize(ecuQualifierLength);
    is.read(const_cast<char *>(ecuQualifier.data()), ecuQualifierLength);
    variantQualifier.resize(variantQualifierLength);
    is.read(const_cast<char *>(variantQualifier.data()), variantQualifierLength);
    serviceQualifier.resize(serviceQualifierLength);
    is.read(const_cast<char *>(serviceQualifier.data()), serviceQualifierLength);
}

void DiagRequestInterpretation::write(AbstractFile & os) {
    /* pre processing */
    ecuQualifierLength = static_cast<uint32_t>(ecuQualifier.size());
    variantQualifierLength = static_cast<uint32_t>(variantQualifier.size());
    serviceQualifierLength = static_cast<uint32_t>(serviceQualifier.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&diagDescriptionHandle), sizeof(diagDescriptionHandle));
    os.write(reinterpret_cast<char *>(&diagVariantHandle), sizeof(diagVariantHandle));
    os.write(reinterpret_cast<char *>(&diagServiceHandle), sizeof(diagServiceHandle));
    os.write(reinterpret_cast<char *>(&ecuQualifierLength), sizeof(ecuQualifierLength));
    os.write(reinterpret_cast<char *>(&variantQualifierLength), sizeof(variantQualifierLength));
    os.write(reinterpret_cast<char *>(&serviceQualifierLength), sizeof(serviceQualifierLength));
    os.write(const_cast<char *>(ecuQualifier.data()), ecuQualifierLength);
    os.write(const_cast<char *>(variantQualifier.data()), variantQualifierLength);
    os.write(const_cast<char *>(serviceQualifier.data()), serviceQualifierLength);
}

uint32_t DiagRequestInterpretation::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(diagDescriptionHandle) +
        sizeof(diagVariantHandle) +
        sizeof(diagServiceHandle) +
        sizeof(ecuQualifierLength) +
        sizeof(variantQualifierLength) +
        sizeof(serviceQualifierLength) +
        ecuQualifierLength +
        variantQualifierLength +
        serviceQualifierLength;
}

}
}
