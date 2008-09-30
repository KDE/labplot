//LabPlot : export.h
// sync with labplot.qs

#ifndef EXPORT_H
#define EXPORT_H

enum EXPORT {EAUTO,EASCII,ECDF,ENETCDF,EAUDIO,EIMAGE,EBINARY,EDATABASE,EHDF5};
QStringList exportitems = (QStringList()<<"automatic"<<"ASCII"<<"CDF"<<"NetCDF"<<"audio"<<"image"<<"binary"<<"database"<<"HDF5");

#endif // EXPORT_H
