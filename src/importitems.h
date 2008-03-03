//LabPlot : importitems.h

#ifndef IMPORTITEMS_H
#define IMPORTITEMS_H

#include "import.h"

QStringList separatoritems = (QStringList()<<"auto"<<","<<":");
QStringList commentitems = (QStringList()<<"#"<<"!"<<"//"<<"+"<<"c"<<":"<<";");
QStringList formatitems = (QStringList()<<"double"<<"float"<<"int (8 bit)"<<"int (16 bit)"<<"int (32 bit)"<<"int (64 bit)");
QStringList byteorderitems = (QStringList()<<"Big endian"<<"Little Endian");
#endif // IMPORTITEMS_H
