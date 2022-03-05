/*
    File                 : FITSFilter.cpp
    Project              : LabPlot
    Description          : FITS I/O-filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FITSFilter.h"
#include "FITSFilterPrivate.h"
#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnStringIO.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/matrix/MatrixModel.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/matrix/Matrix.h"
#include "commonfrontend/matrix/MatrixView.h"

#include <QMultiMap>
#include <QFile>
#include <QDebug>

/*! \class FITSFilter
 * \brief Manages the import/export of data from/to a FITS file.
 * \since 2.2.0
 * \ingroup datasources
 */
FITSFilter::FITSFilter():AbstractFileFilter(FileType::FITS), d(new FITSFilterPrivate(this)) {}

FITSFilter::~FITSFilter() = default;

void FITSFilter::readDataFromFile(const QString &fileName, AbstractDataSource *dataSource, AbstractFileFilter::ImportMode importMode) {
	d->readCHDU(fileName, dataSource, importMode);
}

QVector<QStringList> FITSFilter::readChdu(const QString &fileName, bool* okToMatrix, int lines) {
	return d->readCHDU(fileName, nullptr, AbstractFileFilter::ImportMode::Replace, okToMatrix, lines);
}

void FITSFilter::write(const QString &fileName, AbstractDataSource *dataSource) {
	d->writeCHDU(fileName, dataSource);
}

void FITSFilter::addNewKeyword(const QString &filename, const QList<Keyword> &keywords) {
	d->addNewKeyword(filename, keywords);
}

void FITSFilter::updateKeywords(const QString &fileName, const QList<Keyword>& originals, const QVector<Keyword>& updates) {
	d->updateKeywords(fileName, originals, updates);
}

void FITSFilter::deleteKeyword(const QString &fileName, const QList<Keyword>& keywords) {
	d->deleteKeyword(fileName, keywords);
}

void FITSFilter::addKeywordUnit(const QString &fileName, const QList<Keyword> &keywords) {
	d->addKeywordUnit(fileName, keywords);
}

void FITSFilter::removeExtensions(const QStringList &extensions) {
	d->removeExtensions(extensions);
}

void FITSFilter::parseHeader(const QString &fileName, QTableWidget *headerEditTable, bool readKeys, const QList<Keyword> &keys) {
	d->parseHeader(fileName, headerEditTable, readKeys, keys);
}

void FITSFilter::parseExtensions(const QString &fileName, QTreeWidget *tw, bool checkPrimary) {
	d->parseExtensions(fileName, tw, checkPrimary);
}

QList<FITSFilter::Keyword> FITSFilter::chduKeywords(const QString &fileName) {
	return d->chduKeywords(fileName);
}

void FITSFilter::loadFilterSettings(const QString& /*fileName*/) {
}

void FITSFilter::saveFilterSettings(const QString& /*fileName*/) const {
}

/*!
 * \brief contains the {StandardKeywords \ MandatoryKeywords} keywords
 * \return A list of keywords
 */
QStringList FITSFilter::standardKeywords() {
	return QStringList() << QLatin1String("(blank)") << QLatin1String("CROTA") << QLatin1String("EQUINOX") << QLatin1String("NAXIS") << QLatin1String("TBCOL") << QLatin1String("TUNIT")
	       << QLatin1String("AUTHOR")  << QLatin1String("CRPIX")   << QLatin1String("EXTEND")   << QLatin1String("OBJECT")   << QLatin1String("TDIM")  << QLatin1String("TZERO")
	       << QLatin1String("BITPIX")  << QLatin1String("CRVAL")   << QLatin1String("EXTLEVEL") << QLatin1String("OBSERVER") << QLatin1String("TDISP") << QLatin1String("XTENSION")
	       << QLatin1String("BLANK")   << QLatin1String("CTYPE")   << QLatin1String("EXTNAME")  << QLatin1String("ORIGIN")   << QLatin1String("TELESCOP")
	       << QLatin1String("BLOCKED") << QLatin1String("DATAMAX")  << QLatin1String("EXTVER")
	       << QLatin1String("BSCALE")  << QLatin1String("DATAMIN")  << QLatin1String("PSCAL")  << QLatin1String("TFORM")
	       << QLatin1String("BUNIT")   << QLatin1String("DATE")     << QLatin1String("GROUPS")   << QLatin1String("PTYPE")   << QLatin1String("THEAP")
	       << QLatin1String("BZERO")   << QLatin1String("DATE-OBS") << QLatin1String("HISTORY")  << QLatin1String("PZERO")   << QLatin1String("TNULL")
	       << QLatin1String("CDELT")  << QLatin1String("INSTRUME") << QLatin1String("REFERENC") << QLatin1String("TSCAL")
	       << QLatin1String("COMMENT") << QLatin1String("EPOCH")    << QLatin1String("NAXIS")    << QLatin1String("SIMPLE")   << QLatin1String("TTYPE");
}

/*!
 * \brief Returns a list of keywords, that are mandatory for an image extension of a FITS file
 * see:
 * https://archive.stsci.edu/fits/fits_standard/node64.html
 * \return A list of keywords
 */

QStringList FITSFilter::mandatoryImageExtensionKeywords() {
	return QStringList() << QLatin1String("XTENSION") << QLatin1String("BITPIX")
		<< QLatin1String("NAXIS") << QLatin1String("PCOUNT")
		<< QLatin1String("GCOUNT") << QLatin1String("END");
}

/*!
 * \brief Returns a list of keywords, that are mandatory for a table extension (ascii or bintable)
 * of a FITS file
 * see:
 * https://archive.stsci.edu/fits/fits_standard/node58.html
 * https://archive.stsci.edu/fits/fits_standard/node68.html
 * \return A list of keywords
 */
QStringList FITSFilter::mandatoryTableExtensionKeywords() {
	return QStringList() << QLatin1String("XTENSION") << QLatin1String("BITPIX")
		<< QLatin1String("NAXIS") << QLatin1String("NAXIS1")
		<< QLatin1String("NAXIS2") << QLatin1String("PCOUNT")
		<< QLatin1String("GCOUNT") << QLatin1String("TFIELDS")
		<< QLatin1String("END");
}

/*!
 * \brief Returns a list of strings that represent units which are used for autocompletion when adding
 * keyword units to keywords
 * \return A list of strings that represent units
 */
QStringList FITSFilter::units() {
	return QStringList() << QLatin1String("m (Metre)") << QLatin1String("kg (Kilogram)") << QLatin1String("s (Second)")
		<< QString("M☉ (Solar mass)") << QLatin1String("AU (Astronomical unit") << QLatin1String("l.y (Light year)")
		<< QLatin1String("km (Kilometres") << QLatin1String("pc (Parsec)") << QLatin1String("K (Kelvin)")
		<< QLatin1String("mol (Mole)") << QLatin1String("cd (Candela)");
}

/*!
 * \brief Sets the startColumn to \a column
 * \param column the column to be set
 */
void FITSFilter::setStartColumn(const int column) {
	d->startColumn = column;
}

/*!
 * \brief Returns startColumn
 * \return The startColumn
 */
int FITSFilter::startColumn() const {
	return d->startColumn;
}

/*!
 * \brief Sets the endColumn to \a column
 * \param column the column to be set
 */
void FITSFilter::setEndColumn(const int column) {
	d->endColumn = column;
}

/*!
 * \brief Returns endColumn
 * \return The endColumn
 */
int FITSFilter::endColumn() const {
	return d->endColumn;
}

/*!
 * \brief Sets the startRow to \a row
 * \param row the row to be set
 */
void FITSFilter::setStartRow(const int row) {
	d->startRow = row;
}

/*!
 * \brief Returns startRow
 * \return The startRow
 */
int FITSFilter::startRow() const {
	return d->startRow;
}

/*!
 * \brief Sets the endRow to \a row
 * \param row the row to be set
 */
void FITSFilter::setEndRow(const int row) {
	d->endRow = row;
}

/*!
 * \brief Returns endRow
 * \return The endRow
 */
int FITSFilter::endRow() const {
	return d->endRow;
}

/*!
 * \brief Sets commentsAsUnits to \a commentsAsUnits
 *
 * This is used when spreadsheets are exported to FITS table extensions and comments are used as the
 * units of the table's columns.
 * \param commentsAsUnits
 */
void FITSFilter::setCommentsAsUnits(const bool commentsAsUnits) {
	d->commentsAsUnits = commentsAsUnits;
}

/*!
 * \brief Sets exportTo to \a exportTo
 *
 * This is used to decide whether the container should be exported to a FITS image or a FITS table
 * For an image \a exportTo should be 0, for a table 1
 * \param exportTo
 */
void FITSFilter::setExportTo(const int exportTo) {
	d->exportTo = exportTo;
}

QString FITSFilter::fileInfoString(const QString& fileName) {
	const int imagesCount = FITSFilterPrivate::extensionNames(fileName).values(QLatin1String("IMAGES")).size();
	QString info(i18n("Images: %1", QString::number(imagesCount)));

	info += QLatin1String("<br>");

	const int tablesCount = FITSFilterPrivate::extensionNames(fileName).values(QLatin1String("TABLES")).size();
	info += i18n("Tables: %1", QString::number(tablesCount));

	return info;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

FITSFilterPrivate::FITSFilterPrivate(FITSFilter* owner) : q(owner) {}

/*!
 * \brief Read the current header data unit from file \a filename in data source \a dataSource in \a importMode import mode
 * \param fileName the name of the file to be read
 * \param dataSource the data source to be filled
 * \param importMode
 */
QVector<QStringList> FITSFilterPrivate::readCHDU(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, bool* okToMatrix, int lines) {
	DEBUG("FITSFilterPrivate::readCHDU() file name = " << STDSTRING(fileName));
	QVector<QStringList> dataStrings;

#ifdef HAVE_FITS
	int status = 0;

	if (fits_open_file(&m_fitsFile, fileName.toLatin1(), READONLY, &status)) {
		DEBUG("	ERROR opening file " << STDSTRING(fileName));
		printError(status);
		return dataStrings;
	}

	int chduType;

	if (fits_get_hdu_type(m_fitsFile, &chduType, &status)) {
		printError(status);
		return dataStrings;
	}

	long actualRows;
	int actualCols;
	int columnOffset = 0;

	bool noDataSource = (dataSource == nullptr);

	if (chduType == IMAGE_HDU) {
		DEBUG("IMAGE_HDU");
		int maxdim = 2;
		int bitpix;
		int naxis;
		long naxes[2];

		if (fits_get_img_param(m_fitsFile, maxdim, &bitpix, &naxis, naxes, &status)) {
			printError(status);
			return dataStrings;
		}

		if (naxis == 0)
			return dataStrings;
		actualRows = naxes[1];
		actualCols = naxes[0];
		if (lines == -1)
			lines = actualRows;
		else {
			if (lines > actualRows)
				lines = actualRows;
		}

		if (endRow != -1) {
			if (!noDataSource)
				lines = endRow;
		}
		if (endColumn != -1)
			actualCols = endColumn;
		if (noDataSource)
			dataStrings.reserve(lines);

		int i = 0;
		int j = 0;
		if (startRow != 1)
			i = startRow;
		if (startColumn != 1)
			j = startColumn;

		const int jstart = j;

		//TODO: support other modes
		QVector<AbstractColumn::ColumnMode> columnModes;
		columnModes.resize(actualCols - j);
		QStringList vectorNames;

		std::vector<void*> dataContainer;
		if (!noDataSource) {
			dataContainer.reserve(actualCols - j);
			columnOffset = dataSource->prepareImport(dataContainer, importMode, lines - i, actualCols - j, vectorNames, columnModes);
		}

		long pixelCount = lines * actualCols;
		double* data = new double[pixelCount];

		if (!data) {
			qDebug() << "Not enough memory for data";
			return dataStrings;
		}

		if (fits_read_img(m_fitsFile, TDOUBLE, 1, pixelCount, nullptr, data, nullptr, &status)) {
			printError(status);
			return dataStrings << (QStringList() << QString("Error"));
		}

		int ii = 0;
		DEBUG("	Import " << lines << " lines");
		for (; i < lines; ++i) {
			int jj = 0;
			QStringList line;
			line.reserve(actualCols - j);
			for (; j < actualCols; ++j) {
				if (noDataSource)
					line << QString::number(data[i*naxes[0] +j]);
				else
					static_cast<QVector<double>*>(dataContainer[jj++])->operator[](ii) = data[i* naxes[0] + j];
			}
            if (noDataSource)
                dataStrings << line;
			j = jstart;
			ii++;
		}
		delete[] data;

		if (dataSource)
			dataSource->finalizeImport(columnOffset, 1, actualCols, QString(), importMode);

		fits_close_file(m_fitsFile, &status);

		return dataStrings;

	} else if ((chduType == ASCII_TBL) || (chduType == BINARY_TBL)) {
		DEBUG("ASCII_TBL or BINARY_TBL");

		if (endColumn != -1)
			actualCols = endColumn;
		else
			fits_get_num_cols(m_fitsFile, &actualCols, &status);

		if (endRow != -1)
			actualRows = endRow;
		else
			fits_get_num_rows(m_fitsFile, &actualRows, &status);

		QStringList columnNames;
		QList<int> columnsWidth;
		QStringList columnUnits;
		columnUnits.reserve(actualCols);
		columnsWidth.reserve(actualCols);
		columnNames.reserve(actualCols);
		int colWidth;
		char keyword[FLEN_KEYWORD];
		char value[FLEN_VALUE];
		int col = 1;
		if (startColumn != 1) {
			if (startColumn != 0)
				col = startColumn;
		}
		for (; col <= actualCols; ++col) {
			status = 0;
			fits_make_keyn("TTYPE", col, keyword, &status);
			fits_read_key(m_fitsFile, TSTRING, keyword, value, nullptr, &status);
			columnNames.append(QLatin1String(value));

			fits_make_keyn("TUNIT", col, keyword, &status);
			fits_read_key(m_fitsFile, TSTRING, keyword, value, nullptr, &status);
			columnUnits.append(QLatin1String(value));

			fits_get_col_display_width(m_fitsFile, col, &colWidth, &status);
			columnsWidth.append(colWidth);
		}

		status = 0;
		if (lines == -1)
			lines = actualRows;
		else if (lines > actualRows)
			lines = actualRows;

		if (endRow != -1)
			lines = endRow;
        QVector<QVector<QString>*> stringDataPointers;
		std::vector<void*> numericDataPointers;
		QList<bool> columnNumericTypes;

		int startCol = 0;
		if (startColumn != 1)
			startCol = startColumn;
		int startRrow = 0;
		if (startRow != 1)
			startRrow = startRow;

		columnNumericTypes.reserve(actualCols);
		int datatype;
		int c = 1;
		if (startColumn != 1) {
			if (startColumn != 0)
				c = startColumn;
		}
		QList<int> matrixNumericColumnIndices;
		for (; c <= actualCols; ++c) {
			fits_get_coltype(m_fitsFile, c, &datatype, nullptr, nullptr, &status);

			switch (datatype) {
			case TSTRING:
				columnNumericTypes.append(false);
				break;
			case TSHORT:
				columnNumericTypes.append(true);
				break;
			case TLONG:
				columnNumericTypes.append(true);
				break;
			case TFLOAT:
				columnNumericTypes.append(true);
				break;
			case TDOUBLE:
				columnNumericTypes.append(true);
				break;
			case TLOGICAL:
				columnNumericTypes.append(false);
				break;
			case TBIT:
				columnNumericTypes.append(true);
				break;
			case TBYTE:
				columnNumericTypes.append(true);
				break;
			case TCOMPLEX:
				columnNumericTypes.append(true);
				break;
			default:
				columnNumericTypes.append(false);
				break;
			}
			if ((datatype != TSTRING) && (datatype != TLOGICAL))
				matrixNumericColumnIndices.append(c);
		}

		if (noDataSource)
			*okToMatrix = matrixNumericColumnIndices.isEmpty() ? false : true;
		if (!noDataSource) {
			DEBUG("HAS DataSource");
			auto* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
			if (spreadsheet) {
				numericDataPointers.reserve(actualCols - startCol);

				stringDataPointers.reserve(actualCols - startCol);
				spreadsheet->setUndoAware(false);
				columnOffset = spreadsheet->resize(importMode, columnNames, actualCols - startCol);

				if (importMode == AbstractFileFilter::ImportMode::Replace) {
					spreadsheet->clear();
					spreadsheet->setRowCount(lines - startRrow);
				} else {
					if (spreadsheet->rowCount() < (lines - startRrow))
						spreadsheet->setRowCount(lines - startRrow);
				}
				DEBUG("Reading columns ...");
				for (int n = 0; n < actualCols - startCol; ++n) {
					if (columnNumericTypes.at(n)) {
						spreadsheet->column(columnOffset+ n)->setColumnMode(AbstractColumn::ColumnMode::Double);
						auto* datap = static_cast<QVector<double>* >(spreadsheet->column(columnOffset+n)->data());
						numericDataPointers.push_back(datap);
						if (importMode == AbstractFileFilter::ImportMode::Replace)
							datap->clear();
					} else {
						spreadsheet->column(columnOffset+ n)->setColumnMode(AbstractColumn::ColumnMode::Text);
                        auto* list = static_cast<QVector<QString>*>(spreadsheet->column(columnOffset+n)->data());
						stringDataPointers.push_back(list);
						if (importMode == AbstractFileFilter::ImportMode::Replace)
							list->clear();
					}
				}
				DEBUG("	... DONE");
				stringDataPointers.squeeze();
			} else {
				numericDataPointers.reserve(matrixNumericColumnIndices.size());

				columnOffset = dataSource->prepareImport(numericDataPointers, importMode, lines - startRrow, matrixNumericColumnIndices.size());
			}
		}

		int row = 1;
		if (startRow != 1) {
			if (startRow != 0)
				row = startRow;
		}

		int coll = 1;
		if (startColumn != 1) {
			if (startColumn != 0)
				coll = startColumn;
		}
		bool isMatrix = false;
		if (dynamic_cast<Matrix*>(dataSource)) {
			isMatrix = true;
			coll = matrixNumericColumnIndices.first();
			actualCols = matrixNumericColumnIndices.last();
			if (importMode == AbstractFileFilter::ImportMode::Replace) {
				for (auto* col : numericDataPointers)
					static_cast<QVector<double>*>(col)->clear();
			}
		}

		char array[FLEN_VALUE];
		char* tmpArr[1] = {array};
		for (; row <= lines; ++row) {
			int numericixd = 0;
			int stringidx = 0;
			QStringList line;
			line.reserve(actualCols-coll);
			for (int col = coll; col <= actualCols; ++col) {
				if (isMatrix) {
					if (!matrixNumericColumnIndices.contains(col))
						continue;
				}
				if (fits_read_col_str(m_fitsFile, col, row, 1, 1, nullptr, tmpArr, nullptr, &status))
					printError(status);
				if (!noDataSource) {
					QString str = QString::fromLatin1(array);
					if (str.isEmpty()) {
						if (columnNumericTypes.at(col - 1))
							static_cast<QVector<double>*>(numericDataPointers[numericixd++])->push_back(0);
						else
							stringDataPointers[stringidx++]->append(QLatin1String("NULL"));
					} else {
						if (columnNumericTypes.at(col - 1))
							static_cast<QVector<double>*>(numericDataPointers[numericixd++])->push_back(str.toDouble());
						else {
							if (!stringDataPointers.isEmpty())
								stringDataPointers[stringidx++]->operator<<(str.simplified());
						}

					}
				} else {
					QString tmpColstr = QString::fromLatin1(array);
					tmpColstr = tmpColstr.simplified();
					if (tmpColstr.isEmpty())
						line << QLatin1String("NULL");
					else
						line << tmpColstr;
				}
			}
            if (noDataSource)
                dataStrings << line;
		}

		if (!noDataSource)
			dataSource->finalizeImport(columnOffset, 1, actualCols, QString(), importMode);

		fits_close_file(m_fitsFile, &status);
		return dataStrings;
	} else
		qDebug() << "Incorrect header type";

	fits_close_file(m_fitsFile, &status);

#else
	Q_UNUSED(fileName)
	Q_UNUSED(dataSource)
	Q_UNUSED(importMode)
	Q_UNUSED(okToMatrix)
	Q_UNUSED(lines)
#endif
	return dataStrings;
}

/*!
 * \brief Export from data source \a dataSource to file \a fileName
 * \param fileName the name of the file to be exported to
 * \param dataSource the datasource whose data is exported
 */

void FITSFilterPrivate::writeCHDU(const QString &fileName, AbstractDataSource *dataSource) {
#ifdef HAVE_FITS
	if (!fileName.endsWith(QLatin1String(".fits")))
		return;
	int status = 0;
	bool existed = false;
	if (!QFile::exists(fileName)) {
		if (fits_create_file(&m_fitsFile, fileName.toLatin1(), &status)) {
			printError(status);
			qDebug() << fileName;
			return;
		}
	} else {
		if (fits_open_file(&m_fitsFile, fileName.toLatin1(), READWRITE, &status )) {
			printError(status);
			return;
		} else
			existed = true;
	}

	Matrix* const matrix = dynamic_cast<Matrix*>(dataSource);
	if (matrix) {
		//FITS image
		if (exportTo == 0) {
			long naxes[2] = { matrix->columnCount(), matrix->rowCount() };
			if (fits_create_img(m_fitsFile, FLOAT_IMG, 2, naxes, &status)) {
				printError(status);
				status = 0;
				fits_close_file(m_fitsFile, &status);
				return;
			}
			const long nelem = naxes[0] * naxes[1];
			double* const array = new double[nelem];
			const QVector<QVector<double> >* const data = static_cast<QVector<QVector<double>>*>(matrix->data());

			for (int col = 0; col < naxes[0]; ++col)
				for (int row = 0; row < naxes[1]; ++row)
					array[row * naxes[0] + col] = data->at(row).at(col);

			if (fits_write_img(m_fitsFile, TDOUBLE, 1, nelem, array, &status )) {
				printError(status);
				status = 0;
			}

			fits_close_file(m_fitsFile, &status);
			delete[] array;
			//FITS table
		} else {
			const int nrows = matrix->rowCount();
			const int tfields = matrix->columnCount();
			QVector<char*> columnNames;
			columnNames.resize(tfields);
			columnNames.squeeze();
			QVector<char*> tform;
			tform.resize(tfields);
			tform.squeeze();
			//TODO: mode
			const QVector<QVector<double>>* const matrixData = static_cast<QVector<QVector<double>>*>(matrix->data());
			const MatrixModel* matrixModel = static_cast<MatrixView*>(matrix->view())->model();
			const int precision = matrix->precision();
			for (int i = 0; i < tfields; ++i) {
				const QString& columnName = matrixModel->headerData(i, Qt::Horizontal).toString();
				columnNames[i] = new char[columnName.size() + 1];
				strcpy(columnNames[i], columnName.toLatin1().constData());
				int maxSize = -1;
				for (int row = 0; row < nrows; ++row) {
					if (matrix->text<double>(row, i).size() > maxSize)
						maxSize = matrix->text<double>(row, i).size();
				}
				QString tformn;
				if (precision > 0) {
					tformn = QLatin1String("F")+ QString::number(maxSize) + QLatin1String(".") + QString::number(precision);
				} else
					tformn = QLatin1String("F")+ QString::number(maxSize) + QLatin1String(".0");
				tform[i] = new char[tformn.size() + 1];
				strcpy(tform[i], tformn.toLatin1().constData());
			}
			//TODO extension name containing[] ?

			int r = fits_create_tbl(m_fitsFile, ASCII_TBL, nrows, tfields, columnNames.data(), tform.data(), nullptr,
			            matrix->name().toLatin1().constData(), &status);
			for (int i = 0; i < tfields; ++i) {
				delete[] tform[i];
				delete[] columnNames[i];
			}
			if (r) {
				printError(status);
				status = 0;
				fits_close_file(m_fitsFile, &status);
				if (!existed) {
					QFile file(fileName);
					file.remove();
				}
				return;
			}

			double* columnNumeric = new double[nrows];
			for (int col = 1; col <= tfields; ++col) {
				const QVector<double>& column = matrixData->at(col-1);
				for (int r = 0; r < column.size(); ++r)
					columnNumeric[r] = column.at(r);

				fits_write_col(m_fitsFile, TDOUBLE, col, 1, 1, nrows, columnNumeric, &status);
				if (status) {
					printError(status);
					delete[] columnNumeric;
					status = 0;
					if (!existed) {
						QFile file(fileName);
						file.remove();
					}

					fits_close_file(m_fitsFile, &status);
					return;
				}
			}
			delete[] columnNumeric;
			fits_close_file(m_fitsFile, &status);
		}
		return;
	}

	auto* const spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
	if (spreadsheet) {
		//FITS image
		if (exportTo == 0) {
			int maxRowIdx = -1;
			//don't export lots of empty lines if all of those contain nans
			// TODO: option?
			for (int c = 0; c < spreadsheet->columnCount(); ++c) {
				const Column* const col = spreadsheet->column(c);
				int currMaxRoxIdx = -1;
				for (int r = col->rowCount(); r >= 0; --r) {
					if (col->isValid(r)) {
						currMaxRoxIdx = r;
						break;
					}
				}

				if (currMaxRoxIdx > maxRowIdx) {
					maxRowIdx = currMaxRoxIdx;
				}
			}
			long naxes[2] = { spreadsheet->columnCount(), maxRowIdx + 1};
			if (fits_create_img(m_fitsFile, FLOAT_IMG, 2, naxes, &status)) {
				printError(status);
				status = 0;
				fits_close_file(m_fitsFile, &status);
				if (!existed) {
					QFile file(fileName);
					file.remove();
				}
				return;
			}
			const long nelem = naxes[0] * naxes[1];
			double* array = new double[nelem];

			for (int row = 0; row < naxes[1]; ++row) {
				for (int col = 0; col < naxes[0]; ++col)
					array[row * naxes[0] + col] = spreadsheet->column(col)->valueAt(row);
			}

			if (fits_write_img(m_fitsFile, TDOUBLE, 1, nelem, array, &status )) {
				printError(status);
				status = 0;
				fits_close_file(m_fitsFile, &status);
				if (!existed) {
					QFile file(fileName);
					file.remove();
				}
				return;
			}

			fits_close_file(m_fitsFile, &status);
			delete[] array;
		} else {
			const int nrows = spreadsheet->rowCount();
			const int tfields = spreadsheet->columnCount();

			QVector<char*> columnNames;
			columnNames.resize(tfields);
			columnNames.squeeze();
			QVector<char*> tform;
			tform.resize(tfields);
			tform.squeeze();
			QVector<char*> tunit;
			tunit.resize(tfields);
			tunit.squeeze();

			for (int i = 0; i < tfields; ++i) {
				const Column* const column =  spreadsheet->column(i);

				columnNames[i] = new char[column->name().size() + 1];
				strcpy(columnNames[i], column->name().toLatin1().constData());
				if (commentsAsUnits) {
					tunit[i] = new char[column->comment().size() + 1];
					strcpy(tunit[i], column->comment().toLatin1().constData());
				} else {
					tunit[i] = new char[1];
					tunit[i][0] = '\0';
				}
				switch (column->columnMode()) {
				case AbstractColumn::ColumnMode::Double: {
						int maxSize = -1;
						for (int row = 0; row < nrows; ++row) {
							if (QString::number(column->valueAt(row)).size() > maxSize)
								maxSize = QString::number(column->valueAt(row)).size();
						}

						const Double2StringFilter* const filter = static_cast<Double2StringFilter*>(column->outputFilter());
						bool decimals = false;
						for (int ii = 0; ii < nrows; ++ii) {
							bool ok;
							QString cell = column->asStringColumn()->textAt(ii);
							double val = cell.toDouble(&ok);
							if (cell.size() > QString::number(val).size() + 1) {
								decimals = true;
								break;
							}
						}
						QString tformn;
						if (decimals) {
							int maxStringSize = -1;
							for (int row = 0; row < nrows; ++row) {
								if (column->asStringColumn()->textAt(row).size() > maxStringSize)
									maxStringSize = column->asStringColumn()->textAt(row).size();
							}
							const int diff = abs(maxSize - maxStringSize);
							maxSize+= diff;
							tformn = QLatin1String("F")+ QString::number(maxSize) + QLatin1String(".") + QString::number(filter->numDigits());
						} else
							tformn = QLatin1String("F")+ QString::number(maxSize) + QLatin1String(".0");
						tform[i] = new char[tformn.size()];
						strcpy(tform[i], tformn.toLatin1().data());
						break;
					}
				case AbstractColumn::ColumnMode::Text: {
						int maxSize = -1;
						for (int row = 0; row < nrows; ++row) {
							if (column->textAt(row).size() > maxSize)
								maxSize = column->textAt(row).size();
						}
						const QString& tformn = QLatin1String("A") + QString::number(maxSize);
						tform[i] = new char[tformn.size()];
						strcpy(tform[i], tformn.toLatin1().data());
						break;
					}
				case AbstractColumn::ColumnMode::Integer:	//TODO
				case AbstractColumn::ColumnMode::BigInt:
				case AbstractColumn::ColumnMode::DateTime:
				case AbstractColumn::ColumnMode::Day:
				case AbstractColumn::ColumnMode::Month:
					break;
				}
			}
			//TODO extension name containing[] ?

			int r = fits_create_tbl(m_fitsFile, ASCII_TBL, nrows, tfields, columnNames.data(), tform.data(), tunit.data(),
			            spreadsheet->name().toLatin1().constData(), &status);
			for (int i = 0; i < tfields; ++i) {
				delete[] tform[i];
				delete[] tunit[i];
				delete[] columnNames[i];
			}
			if (r) {
				printError(status);
				status = 0;
				fits_close_file(m_fitsFile, &status);
				if (!existed) {
					QFile file(fileName);
					file.remove();
				}
				return;
			}

			QVector<char*> column;
			column.resize(nrows);
			column.squeeze();

			double* columnNumeric = new double[nrows];
			for (int col = 1; col <= tfields; ++col) {
				const Column* c =  spreadsheet->column(col-1);
				auto columnMode = c->columnMode();

				if (columnMode == AbstractColumn::ColumnMode::Double) {
					for (int row = 0; row < nrows; ++row)
						columnNumeric[row] = c->valueAt(row);

					fits_write_col(m_fitsFile, TDOUBLE, col, 1, 1, nrows, columnNumeric, &status);
					if (status) {
						printError(status);
						delete[] columnNumeric;
						status = 0;
						fits_close_file(m_fitsFile, &status);
						if (!existed) {
							QFile file(fileName);
							file.remove();
						}
						return;
					}
				} else {
					for (int row = 0; row < nrows; ++row) {
						column[row] = new char[c->textAt(row).size() + 1];
						strcpy(column[row], c->textAt(row).toLatin1().constData());
					}
					fits_write_col(m_fitsFile, TSTRING, col, 1, 1, nrows, column.data(), &status);
					for (int row = 0; row < nrows; ++row)
						delete[] column[row];
					if (status) {
						printError(status);
						status = 0;
						fits_close_file(m_fitsFile, &status);

						delete[] columnNumeric;
						return;
					}
				}
			}

			delete[] columnNumeric;

			status = 0;
			fits_close_file(m_fitsFile, &status);
		}
	}
#else
	Q_UNUSED(fileName)
	Q_UNUSED(dataSource)
#endif
}

/*!
 * \brief Return a map of the available extensions names in file \a filename
 *        The keys of the map are the extension types, the values are the names
 * \param fileName the name of the FITS file to be analyzed
 */
QMultiMap<QString, QString> FITSFilterPrivate::extensionNames(const QString& fileName) {
	DEBUG("FITSFilterPrivate::extensionNames() file name = " << STDSTRING(fileName));
#ifdef HAVE_FITS
	QMultiMap<QString, QString> extensions;
	int status = 0;
	fitsfile* fitsFile = nullptr;
	if (fits_open_file(&fitsFile, fileName.toLatin1(), READONLY, &status ))
		return {};
	int hduCount;

	if (fits_get_num_hdus(fitsFile, &hduCount, &status))
		return {};
	int imageCount = 0;
	int asciiTableCount = 0;
	int binaryTableCount = 0;
	for (int currentHDU = 1; (currentHDU <= hduCount) && !status; ++currentHDU) {
		int hduType;
		status = 0;

		fits_get_hdu_type(fitsFile, &hduType, &status);
		switch (hduType) {
		case IMAGE_HDU:
			imageCount++;
			break;
		case ASCII_TBL:
			asciiTableCount++;
			break;
		case BINARY_TBL:
			binaryTableCount++;
			break;
		}
		char* keyVal = new char[FLEN_VALUE];
		QString extName;
		if (!fits_read_keyword(fitsFile,"EXTNAME", keyVal, nullptr, &status)) {
			extName = QLatin1String(keyVal);
			extName = extName.mid(1, extName.length() -2).simplified();
		} else {
			status = 0;
			if (!fits_read_keyword(fitsFile,"HDUNAME", keyVal, nullptr, &status)) {
				extName = QLatin1String(keyVal);
				extName = extName.mid(1, extName.length() -2).simplified();
			} else {
				status = 0;
				switch (hduType) {
				case IMAGE_HDU:
					if (imageCount == 1)
						extName = i18n("Primary header");
					else
						extName = i18n("IMAGE #%1", imageCount);
					break;
				case ASCII_TBL:
					extName = i18n("ASCII_TBL #%1", asciiTableCount);
					break;
				case BINARY_TBL:
					extName = i18n("BINARY_TBL #%1", binaryTableCount);
					break;
				}
			}
		}
		delete[] keyVal;
		status = 0;
		extName = extName.trimmed();
		switch (hduType) {
		case IMAGE_HDU:
			extensions.insert(QLatin1String("IMAGES"), extName);
			break;
		case ASCII_TBL:
			extensions.insert(QLatin1String("TABLES"), extName);
			break;
		case BINARY_TBL:
			extensions.insert(QLatin1String("TABLES"), extName);
			break;
		}
		fits_movrel_hdu(fitsFile, 1, nullptr, &status);
	}

	if (status == END_OF_FILE)
		status = 0;

	fits_close_file(fitsFile, &status);
	return extensions;
#else
	Q_UNUSED(fileName)
	return QMultiMap<QString, QString>();
#endif
}

/*!
 * \brief Prints the error text corresponding to the status code \a status
 * \param status the status code of the error
 */
void FITSFilterPrivate::printError(int status) const {
#ifdef HAVE_FITS
	if (status) {
		char errorText[FLEN_ERRMSG];
		fits_get_errstatus(status, errorText);
		qDebug() << QLatin1String(errorText);
	}
#else
	Q_UNUSED(status)
#endif
}

/*!
 * \brief Add the keywords \a keywords to the current header unit
 * \param keywords the keywords to be added
 * \param fileName the name of the FITS file (extension) in which the keywords are added
 */

void FITSFilterPrivate::addNewKeyword(const QString& fileName, const QList<FITSFilter::Keyword>& keywords) {
#ifdef HAVE_FITS
	int status = 0;
	if (fits_open_file(&m_fitsFile, fileName.toLatin1(), READWRITE, &status )) {
		printError(status);
		return;
	}
	for (const FITSFilter::Keyword& keyword: keywords) {
		status = 0;
		if (!keyword.key.compare(QLatin1String("COMMENT"))) {
			if (fits_write_comment(m_fitsFile, keyword.value.toLatin1(), &status))
				printError(status);
		} else if (!keyword.key.compare(QLatin1String("HISTORY"))) {
			if (fits_write_history(m_fitsFile, keyword.value.toLatin1(), &status))
				printError(status);
		} else if (!keyword.key.compare(QLatin1String("DATE"))) {
			if (fits_write_date(m_fitsFile, &status))
				printError(status);
		} else {
			int ok = 0;
			if (keyword.key.length() <= FLEN_KEYWORD) {
				ok++;
				if (keyword.value.length() <= FLEN_VALUE) {
					ok++;
					if (keyword.comment.length() <= FLEN_COMMENT)
						ok++;
				}
			}
			if (ok == 3) {
				bool ok;
				double val = keyword.value.toDouble(&ok);
				if (ok) {
					if (fits_write_key(m_fitsFile,
					                   TDOUBLE,
					                   keyword.key.toLatin1().data(),
					                   &val,
					                   keyword.comment.toLatin1().data(),
					                   &status))
						printError(status);
				} else {
					if (fits_write_key(m_fitsFile,
					                   TSTRING,
					                   keyword.key.toLatin1().data(),
					                   keyword.value.toLatin1().data(),
					                   keyword.comment.toLatin1().data(),
					                   &status))
						printError(status);
				}
			} else if ( ok == 2) {
				//comment too long
			} else if ( ok == 1) {
				//value too long
			} else {
				//keyword too long
			}
		}
	}
	status = 0;
	fits_close_file(m_fitsFile, &status);
#else
	Q_UNUSED(keywords)
	Q_UNUSED(fileName)

#endif
}
/*!
 * \brief Update keywords in the current header unit
 * \param fileName The name of the FITS file (extension) in which the keywords will be updated
 * \param originals The original keywords of the FITS file (extension)
 * \param updates The keywords that contain the updated values
 */
void FITSFilterPrivate::updateKeywords(const QString& fileName,
                                       const QList<FITSFilter::Keyword>& originals,
                                       const QVector<FITSFilter::Keyword>& updates) {
#ifdef HAVE_FITS
	int status = 0;
	if (fits_open_file(&m_fitsFile, fileName.toLatin1(), READWRITE, &status )) {
		printError(status);
		return;
	}
	FITSFilter::Keyword updatedKeyword;
	FITSFilter::Keyword originalKeyword;
	FITSFilter::KeywordUpdate keywordUpdate;

	for (int i = 0; i < updates.size(); ++i) {
		updatedKeyword = updates.at(i);
		originalKeyword = originals.at(i);
		keywordUpdate = originals.at(i).updates;
		if (keywordUpdate.keyUpdated &&
		        keywordUpdate.valueUpdated &&
		        keywordUpdate.commentUpdated) {
			if (updatedKeyword.isEmpty()) {
				if (fits_delete_key(m_fitsFile, originalKeyword.key.toLatin1(), &status)) {
					printError(status);
					status = 0;
				}
				continue;
			}
		}
		if (!updatedKeyword.key.isEmpty()) {
			if (fits_modify_name(m_fitsFile, originalKeyword.key.toLatin1(), updatedKeyword.key.toLatin1(), &status )) {
				printError(status);
				status = 0;
			}
		}

		if (!updatedKeyword.value.isEmpty()) {
			bool ok;
			int intValue;
			double doubleValue;
			bool updated = false;

			doubleValue = updatedKeyword.value.toDouble(&ok);
			if (ok) {
				if (fits_update_key(m_fitsFile,TDOUBLE,
				                    keywordUpdate.keyUpdated ? updatedKeyword.key.toLatin1() : originalKeyword.key.toLatin1(),
				                    &doubleValue,
				                    nullptr, &status))
					printError(status);
				else
					updated = true;
			}
			if (!updated) {
				intValue = updatedKeyword.value.toInt(&ok);
				if (ok) {
					if (fits_update_key(m_fitsFile,TINT,
					                    keywordUpdate.keyUpdated ? updatedKeyword.key.toLatin1() : originalKeyword.key.toLatin1(),
					                    &intValue,
					                    nullptr, &status))
						printError(status);
					else
						updated = true;
				}
			}
			if (!updated) {
				if (fits_update_key(m_fitsFile,TSTRING,
				                    keywordUpdate.keyUpdated ? updatedKeyword.key.toLatin1() : originalKeyword.key.toLatin1(),
				                    updatedKeyword.value.toLatin1().data(),
				                    nullptr, &status))
					printError(status);
			}
		} else {
			if (keywordUpdate.valueUpdated) {
				if (fits_update_key_null(m_fitsFile,
				                         keywordUpdate.keyUpdated ? updatedKeyword.key.toLatin1() : originalKeyword.key.toLatin1(),
				                         nullptr, &status)) {
					printError(status);
					status = 0;
				}
			}
		}

		if (!updatedKeyword.comment.isEmpty()) {
			if (fits_modify_comment(m_fitsFile, keywordUpdate.keyUpdated ? updatedKeyword.key.toLatin1() : originalKeyword.key.toLatin1(),
			                        updatedKeyword.comment.toLatin1().data(), &status)) {
				printError(status);
				status = 0;
			}
		} else {
			if (keywordUpdate.commentUpdated) {
				if (fits_modify_comment(m_fitsFile,
				                        keywordUpdate.keyUpdated ? updatedKeyword.key.toLatin1() : originalKeyword.key.toLatin1(),
				                        QByteArray().constData(), &status)) {
					printError(status);
					status = 0;
				}
			}
		}
	}
	status = 0;
	fits_close_file(m_fitsFile, &status);
#else
	Q_UNUSED(fileName)
	Q_UNUSED(originals)
	Q_UNUSED(updates)
#endif
}

/*!
 * \brief Delete the keywords \a keywords from the current header unit
 * \param fileName the name of the FITS file (extension) in which the keywords will be deleted.
 * \param keywords the keywords to deleted
 */

void FITSFilterPrivate::deleteKeyword(const QString& fileName, const QList<FITSFilter::Keyword> &keywords) {
#ifdef HAVE_FITS
	int status = 0;
	if (fits_open_file(&m_fitsFile, fileName.toLatin1(), READWRITE, &status )) {
		printError(status);
		return;
	}
	for (const auto& keyword : keywords) {
		if (!keyword.key.isEmpty()) {
			status = 0;
			if (fits_delete_key(m_fitsFile, keyword.key.toLatin1(), &status))
				printError(status);
		}
	}
	status = 0;
	fits_close_file(m_fitsFile, &status);
#else
	Q_UNUSED(keywords)
	Q_UNUSED(fileName)
#endif
}

/*!
 * \brief FITSFilterPrivate::addKeywordUnit
 * \param fileName the FITS file (extension) in which the keyword units are updated/added
 * \param keywords the keywords whose units were modified/added
 */

void FITSFilterPrivate::addKeywordUnit(const QString &fileName, const QList<FITSFilter::Keyword> &keywords) {
#ifdef HAVE_FITS
	int status = 0;
	if (fits_open_file(&m_fitsFile, fileName.toLatin1(), READWRITE, &status )) {
		printError(status);
		return;
	}

	for (const FITSFilter::Keyword& keyword : keywords) {
		if (keyword.updates.unitUpdated) {
			if (fits_write_key_unit(m_fitsFile, keyword.key.toLatin1(), keyword.unit.toLatin1().data(), &status)) {
				printError(status);
				status = 0;
			}
		}
	}
	status = 0;
	fits_close_file(m_fitsFile, &status);
#else
	Q_UNUSED(fileName)
	Q_UNUSED(keywords)
#endif
}

/*!
 * \brief Remove extensions from a FITS file
 * \param extensions The extensions to be removed from the FITS file
 */
void FITSFilterPrivate::removeExtensions(const QStringList &extensions) {
#ifdef HAVE_FITS
	int status = 0;
	for (const auto& ext : extensions) {
		status = 0;
		if (fits_open_file(&m_fitsFile, ext.toLatin1(), READWRITE, &status )) {
			printError(status);
			continue;
		}

		if (fits_delete_hdu(m_fitsFile, nullptr, &status))
			printError(status);

		status = 0;
		fits_close_file(m_fitsFile, &status);
	}
#else
	Q_UNUSED(extensions)
#endif
}

/*!
 * \brief Returns a list of keywords in the current header of \a fileName
 * \param fileName the name of the FITS file (extension) to be opened
 * \return A list of keywords
 */
QList<FITSFilter::Keyword> FITSFilterPrivate::chduKeywords(const QString& fileName) {
#ifdef HAVE_FITS
	int status = 0;

	if (fits_open_file(&m_fitsFile, fileName.toLatin1(), READONLY, &status )) {
		printError(status);
		return {};
	}
	int numberOfKeys;
	if (fits_get_hdrspace(m_fitsFile, &numberOfKeys, nullptr, &status)) {
		printError(status);
		return {};
	}

	QList<FITSFilter::Keyword> keywords;
	keywords.reserve(numberOfKeys);
	char* key = new char[FLEN_KEYWORD];
	char* value = new char[FLEN_VALUE];
	char* comment = new char[FLEN_COMMENT];
	char* unit = new char[FLEN_VALUE];
	for (int i = 1; i <= numberOfKeys; ++i) {
		QStringList recordValues;
		FITSFilter::Keyword keyword;

		if (fits_read_keyn(m_fitsFile, i, key, value, comment, &status)) {
			printError(status);
			status = 0;
			continue;
		}

		fits_read_key_unit(m_fitsFile, key, unit, &status);

		recordValues << QLatin1String(key) << QLatin1String(value) << QLatin1String(comment) << QLatin1String(unit);

		keyword.key = recordValues[0].simplified();
		keyword.value = recordValues[1].simplified();
		keyword.comment = recordValues[2].simplified();
		keyword.unit = recordValues[3].simplified();

		keywords.append(keyword);
	}
	delete[] key;
	delete[] value;
	delete[] comment;
	delete[] unit;

	fits_close_file(m_fitsFile, &status);

	return keywords;
#else
	Q_UNUSED(fileName)
	return QList<FITSFilter::Keyword>();
#endif
}

/*!
 * \brief Builds the table \a headerEditTable from the keywords \a keys
 * \param fileName The name of the FITS file from which the keys are read if \a readKeys is \c true
 * \param headerEditTable The table to be built
 * \param readKeys It's used to determine whether the keywords are provided or they should be read from
 * file \a fileName
 * \param keys The keywords that are provided if the keywords were read already.
 */
void FITSFilterPrivate::parseHeader(const QString &fileName, QTableWidget *headerEditTable, bool readKeys, const QList<FITSFilter::Keyword>& keys) {
#ifdef HAVE_FITS
	QList<FITSFilter::Keyword> keywords;
	if (readKeys)
		keywords = chduKeywords(fileName);
	else
		keywords = keys;

	headerEditTable->setRowCount(keywords.size());
	QTableWidgetItem* item;
	for (int i = 0; i < keywords.size(); ++i) {
		const FITSFilter::Keyword& keyword = keywords.at(i);
		const bool mandatory = FITSFilter::mandatoryImageExtensionKeywords().contains(keyword.key) ||
					FITSFilter::mandatoryTableExtensionKeywords().contains(keyword.key);
		item = new QTableWidgetItem(keyword.key);
		const QString& itemText = item->text();
		const bool notEditableKey = mandatory || itemText.contains(QLatin1String("TFORM")) ||
						itemText.contains(QLatin1String("TTYPE")) ||
						itemText.contains(QLatin1String("TUNIT"))  ||
						itemText.contains(QLatin1String("TDISP")) ||
						itemText.contains(QLatin1String("TBCOL")) ||
						itemText.contains(QLatin1String("TZERO"));
		const bool notEditableValue = mandatory || itemText.contains(QLatin1String("TFORM")) ||
						itemText.contains(QLatin1String("TDISP")) ||
						itemText.contains(QLatin1String("TBCOL")) ||
						itemText.contains(QLatin1String("TZERO"));

		if (notEditableKey)
			item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		else
			item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		headerEditTable->setItem(i, 0, item );

		item = new QTableWidgetItem(keyword.value);
		if (notEditableValue)
			item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		else
			item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		headerEditTable->setItem(i, 1, item );
		QString commentFieldText;
		if (!keyword.unit.isEmpty()) {
			if (keyword.updates.unitUpdated) {
				const QString& comment = keyword.comment.right(keyword.comment.size() - keyword.comment.indexOf(QChar(']'))-1);
				commentFieldText = QLatin1String("[") + keyword.unit + QLatin1String("] ") + comment;
			} else {
				if (keyword.comment.at(0) == QLatin1Char('['))
					commentFieldText = keyword.comment;
				else
					commentFieldText = QLatin1String("[") + keyword.unit + QLatin1String("] ") + keyword.comment;
			}
		} else
			commentFieldText = keyword.comment;
		item = new QTableWidgetItem(commentFieldText);

		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		headerEditTable->setItem(i, 2, item );
	}

	headerEditTable->resizeColumnsToContents();
#else
	Q_UNUSED(fileName)
	Q_UNUSED(headerEditTable)
	Q_UNUSED(readKeys)
	Q_UNUSED(keys)
#endif
}

/*!
 * \brief Helper function to return the value of the key \a key
 * \param fileName The name of the FITS file (extension) in which the keyword with key \a key should exist
 * \param key The key of the keyword whose value it's returned
 * \return The value of the keyword as a string
 */
const QString FITSFilterPrivate::valueOf(const QString& fileName, const char *key) {
#ifdef HAVE_FITS
	int status = 0;
	if (fits_open_file(&m_fitsFile, fileName.toLatin1(), READONLY, &status )) {
		printError(status);
		return QString ();
	}

	char* keyVal = new char[FLEN_VALUE];
	QString keyValue;
	if (!fits_read_keyword(m_fitsFile, key, keyVal, nullptr, &status)) {
		keyValue = QLatin1String(keyVal);
		keyValue = keyValue.simplified();
	} else {
		printError(status);
		delete[] keyVal;
		fits_close_file(m_fitsFile, &status);
		return QString();
	}

	delete[] keyVal;
	status = 0;
	fits_close_file(m_fitsFile, &status);
	return keyValue;
#else
	Q_UNUSED(fileName)
	Q_UNUSED(key)
	return QString();
#endif
}

/*!
 * \brief Build the extensions tree from FITS file (extension) \a fileName
 * \param fileName The name of the FITS file to be opened
 * \param tw The QTreeWidget to be built
 * \param checkPrimary Used to determine whether the tree will be used for import or the header edit,
 * if it's \c true and if the primary array it's empty, then the item won't be added to the tree
 */
void FITSFilterPrivate::parseExtensions(const QString &fileName, QTreeWidget *tw, bool checkPrimary) {
	DEBUG("FITSFilterPrivate::parseExtensions()");
#ifdef HAVE_FITS
	const QMultiMap<QString, QString>& extensions = extensionNames(fileName);
	const QStringList& imageExtensions = extensions.values(QLatin1String("IMAGES"));
	const QStringList& tableExtensions = extensions.values(QLatin1String("TABLES"));

	QTreeWidgetItem* root = tw->invisibleRootItem();
	//TODO: fileName may contain any data type: check if it's a FITS file
	auto* treeNameItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList() << fileName);
	root->addChild(treeNameItem);
	treeNameItem->setExpanded(true);

	auto* imageExtensionItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList() << i18n("Images"));
	imageExtensionItem->setFlags(imageExtensionItem->flags() & ~Qt::ItemIsSelectable );
	QString primaryHeaderNaxis = valueOf(fileName, "NAXIS");
	const int naxis = primaryHeaderNaxis.toInt();
	bool noImage = false;
	for (const QString& ext : imageExtensions) {
		auto* treeItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList() << ext);
		if (ext == i18n("Primary header")) {
			if (checkPrimary && naxis == 0) {
				delete treeItem;
				continue;
			}
		}
		imageExtensionItem->addChild(treeItem);
	}
	if (imageExtensionItem->childCount() > 0) {
		treeNameItem->addChild(imageExtensionItem);
		imageExtensionItem->setIcon(0,QIcon::fromTheme("view-preview"));
		imageExtensionItem->setExpanded(true);
		imageExtensionItem->child(0)->setSelected(true);

		tw->setCurrentItem(imageExtensionItem->child(0));
	} else
		noImage = true;

	if (tableExtensions.size() > 0) {
		auto* tableExtensionItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList() << i18n("Tables"));
		tableExtensionItem->setFlags(tableExtensionItem->flags() & ~Qt::ItemIsSelectable );

		for (const QString& ext : tableExtensions) {
			auto* treeItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList() << ext);
			tableExtensionItem->addChild(treeItem);
		}
		if (tableExtensionItem->childCount() > 0) {
			treeNameItem->addChild(tableExtensionItem);
			tableExtensionItem->setIcon(0,QIcon::fromTheme("x-office-spreadsheet"));
			tableExtensionItem->setExpanded(true);
			if (noImage) {
				tableExtensionItem->child(0)->setSelected(true);
				tw->setCurrentItem(tableExtensionItem->child(0));
			}
		}
	}
#else
	Q_UNUSED(fileName)
	Q_UNUSED(tw)
	Q_UNUSED(checkPrimary)
#endif
	DEBUG("FITSFilterPrivate::parseExtensions() DONE");
}

/*!
 * \brief FITSFilterPrivate::~FITSFilterPrivate
 */

FITSFilterPrivate::~FITSFilterPrivate() = default;

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
*/

void FITSFilter::save(QXmlStreamWriter*) const {
}

/*!
  Loads from XML.
*/

bool FITSFilter::load(XmlStreamReader*) {
	return false;
}
