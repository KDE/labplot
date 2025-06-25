/*
	File                 : SpiceFilter.cpp
	Project              : LabPlot
	Description          : Filters for reading spice files
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "SpiceFilter.h"
#include "SpiceFilterPrivate.h"
#include "SpiceReader.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <QXmlStreamWriter>

const QString SpiceFilter::xmlElementName = QStringLiteral("SpiceFilter");

SpiceFilter::SpiceFilter()
	: AbstractFileFilter(FileType::Spice)
	, d(new SpiceFilterPrivate(this)) {
}

SpiceFilter::~SpiceFilter() = default;

bool SpiceFilter::isSpiceFile(const QString& fileName, bool* binary) {
	SpiceFileReader reader(fileName);
	if (!reader.open())
		return false;

	if (!reader.validSpiceFile())
		return false;

	if (binary)
		*binary = reader.binary();

	return true;
}

QString SpiceFilter::fileInfoString(const QString& fileName) {
	SpiceFileReader reader(fileName);
	if (!reader.open())
		return {};

	if (!reader.validSpiceFile())
		return {};

	return reader.infoString();
}

QVector<QStringList> SpiceFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

/*!
  reads the content of the file \c fileName.
*/
void SpiceFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}

/*!
  writes the content of the data source \c dataSource to the file \c fileName.
*/
void SpiceFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

void SpiceFilter::setStartRow(const int r) {
	d->startRow = r;
}
int SpiceFilter::startRow() const {
	return d->startRow;
}

void SpiceFilter::setEndRow(const int r) {
	d->endRow = r;
}
int SpiceFilter::endRow() const {
	return d->endRow;
}

QStringList SpiceFilter::vectorNames() const {
	return d->vectorNames;
}

QVector<AbstractColumn::ColumnMode> SpiceFilter::columnModes() {
	return d->columnModes;
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
SpiceFilterPrivate::SpiceFilterPrivate(SpiceFilter* owner)
	: q(owner) {
}

void SpiceFilterPrivate::generateVectorNamesColumnModes(const SpiceFileReader& reader) {
	// add names of the variables
	vectorNames.clear();
	columnModes.clear();
	for (const auto& variable : reader.variables()) {
		if (!reader.isReal()) {
			vectorNames << variable.variableName + QStringLiteral(", ") + variable.type + QStringLiteral(" REAL");
			vectorNames << variable.variableName + QStringLiteral(", ") + variable.type + QStringLiteral(" IMAGINARY");
			columnModes << AbstractColumn::ColumnMode::Double;
			columnModes << AbstractColumn::ColumnMode::Double;
		} else {
			vectorNames << variable.variableName + QStringLiteral(", ") + variable.type;
			columnModes << AbstractColumn::ColumnMode::Double;
		}
	}
}

/*!
 * generates the preview for the file \c fileName reading the provided number of \c lines.
 */
QVector<QStringList> SpiceFilterPrivate::preview(const QString& fileName, int lines) {
	DEBUG(Q_FUNC_INFO);

	SpiceFileReader reader(fileName);
#ifdef SPICEFILTERTEST_EN
	reader.setBulkReadLines(q->mBulkLineCount);
#endif
	if (!reader.open()) {
		q->setLastError(i18n("Failed to open the device/file."));
		return {};
	}
	if (!reader.validSpiceFile()) {
		q->setLastError(i18n("Invalid file."));
		return {};
	}

	generateVectorNamesColumnModes(reader);

	// prepare the data container
	const int numberVariables = reader.variables().count();

	// skip data lines, if required
	const int skip = startRow - 1;

	// create new datacontainer to store the preview
	std::vector<void*> dataContainer;
	dataContainer.resize(numberVariables * (1 + !reader.isReal()));
	for (uint i = 0; i < dataContainer.size(); i++)
		dataContainer[i] = new QVector<double>(lines);

	const int linesRead = reader.readData(dataContainer, skip, lines);

	QStringList lineString;
	int isComplex = !reader.isReal();

	QVector<QStringList> dataStrings;
	for (int l = 0; l < linesRead; l++) {
		lineString.clear();
		for (int i = 0; i < numberVariables * (1 + isComplex); i++) {
			const auto values = static_cast<const QVector<double>*>(dataContainer[i]);
			lineString << QString::number(values->at(l), 'e', 15); // real part
		}
		dataStrings << lineString;
	}

	// delete all element of the datacontainer again
	for (uint i = 0; i < dataContainer.size(); i++)
		delete static_cast<QVector<double>*>(dataContainer[i]);

	return dataStrings;
}

/*!
	reads the content of the file \c fileName to the data source \c dataSource. Uses the settings defined in the data source.
*/
void SpiceFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG(Q_FUNC_INFO << ", fileName = \'" << STDSTRING(fileName) << "\', dataSource = " << dataSource
					  << ", mode = " << ENUM_TO_STRING(AbstractFileFilter, ImportMode, importMode));

	SpiceFileReader reader(fileName);
#ifdef SPICEFILTERTEST_EN
	reader.setBulkReadLines(q->mBulkLineCount);
#endif
	if (!reader.open()) {
		q->setLastError(i18n("Failed to open the device/file."));
		return;
	}
	if (!reader.validSpiceFile()) {
		q->setLastError(i18n("Invalid file."));
		return;
	}

	q->connect(&reader, &SpiceFileReader::processed, [=](double processed) {
		Q_EMIT q->completed(processed);
	});

	generateVectorNamesColumnModes(reader);

	// prepare the data container
	const int numberVariables = reader.variables().count();
	const int actualEndRow = (endRow == -1 || endRow > reader.numberSimulationPoints()) ? reader.numberSimulationPoints() : endRow;
	const int actualRows = actualEndRow - startRow + 1;
	const int actualCols = reader.isReal() ? numberVariables : 2 * numberVariables;
	// resize dataContainer
	bool ok = false;
	const int columnOffset = dataSource->prepareImport(m_dataContainer, importMode, actualRows, actualCols, vectorNames, columnModes, ok);
	if (!ok) {
		q->setLastError(i18n("Not enough memory."));
		return;
	}

	// skip data lines, if required
	const int skip = startRow - 1;
	reader.readData(m_dataContainer, skip, actualRows);

	dataSource->finalizeImport(columnOffset, 1, actualCols, QString(), importMode);
}

/*!
	writes the content of \c dataSource to the file \c fileName.
*/
void SpiceFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	// TODO: not implemented yet
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
/*!
  Saves as XML.
 */
void SpiceFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(xmlElementName);
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool SpiceFilter::load(XmlStreamReader*) {
	return true;
}
