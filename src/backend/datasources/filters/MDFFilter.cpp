#include "MDFFilter.h"
#include "MDFFilterPrivate.h"

#include <mdf/mdfreader.h>
using namespace mdf;

const QString MDFFilter::xmlElementName = QStringLiteral("MDFFilter");

MDFFilter::MDFFilter(): AbstractFileFilter(FileType::MDF), d(new MDFFilterPrivate(this)) {

}

bool MDFFilter::isValid(const QString& fileName) {

    const QStringList extensions{"mf4", "mdf4", "mdf"};

    // Fast check: check extension
    const auto& extension = fileName.split(".").last();
    bool ending_valid = false;
    for (auto& e: extensions) {
        if (e == extension) {
            ending_valid = true;
            break;
        }
    }
    if (!ending_valid)
        return false;

    MdfReader reader(fileName.toStdString());
    // If header is valid. It will be assumed it is valid
    reader.ReadHeader();

    return true;
}

QString MDFFilter::fileInfoString(const QString& filename) {
    if (!MDFFilter::isValid(filename)) {
        DEBUG(qPrintable(filename) << " is not a MDF file!");
        return tr("Not a MDF file");
    }
    return "";
}

QVector<QStringList> MDFFilter::preview(const QString& fileName, int lines) {
    //https://ihedvall.github.io/mdflib/mdfreader.html
}

void MDFFilter::readDataFromFile(const QString& fileName, AbstractDataSource*, AbstractFileFilter::ImportMode) {

}

void MDFFilter::write(const QString& filename, AbstractDataSource* dataSource) {
    d->write(filename, dataSource);
}

//#############################################################################
//####### MDFFilter Private ###################################################
//#############################################################################

MDFFilterPrivate::MDFFilterPrivate(MDFFilter* owner): q(owner) {

}

void MDFFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource*) {
    // TODO: not implemented yet
}
