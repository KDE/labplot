#ifndef MDFFILTER_H
#define MDFFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class MDFFilterPrivate;

class MDFFilter: public AbstractFileFilter {
    Q_OBJECT
public:
    MDFFilter();
    static QString fileInfoString(const QString&);
    // Check if valid mdf file
    static bool isValid(const QString& fileName);
    QVector<QStringList> preview(const QString& fileName, int lines);
    void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
    void write(const QString& fileName, AbstractDataSource*) override;

    static const QString xmlElementName;

protected:
    std::unique_ptr<MDFFilterPrivate> const d;
};

#endif // MDFFILTER_H
