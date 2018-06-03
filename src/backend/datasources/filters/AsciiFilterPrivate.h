/***************************************************************************
    File                 : AsciiFilterPrivate.h
    Project              : LabPlot
    Description          : Private implementation class for AsciiFilter.
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2013 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef ASCIIFILTERPRIVATE_H
#define ASCIIFILTERPRIVATE_H

class KFilterDev;
class AbstractDataSource;
class AbstractColumn;

class AsciiFilterPrivate {

public:
	explicit AsciiFilterPrivate(AsciiFilter*);

	QStringList getLineString(QIODevice&);
	int prepareDeviceToRead(QIODevice&);
	void readDataFromDevice(QIODevice&, AbstractDataSource* = nullptr,
	                        AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void readFromLiveDeviceNotFile(QIODevice& device, AbstractDataSource*,
	                               AbstractFileFilter::ImportMode = AbstractFileFilter::Replace);
	qint64 readFromLiveDevice(QIODevice&, AbstractDataSource*, qint64 from = -1);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
	                      AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& fileName, int lines);
	QVector<QStringList> preview(QIODevice& device);

    int readFromMqtt(const QString&, const QString&, AbstractDataSource*dataSource);
    int prepareMqttToRead(const QString&,  const QString&);
	void mqttPreview(QVector<QStringList>&, const QString&, const QString&);

	const AsciiFilter* q;

	QString commentCharacter;
	QString separatingCharacter;
	QString dateTimeFormat;
	QLocale::Language numberFormat;
	bool autoModeEnabled;
	bool headerEnabled;
	bool skipEmptyParts;
	bool simplifyWhitespacesEnabled;
	double nanValue;
	bool removeQuotesEnabled;
	bool createIndexEnabled;
	QStringList vectorNames;
	QVector<AbstractColumn::ColumnMode> columnModes;
	int startRow;
	int endRow;
	int startColumn;
	int endColumn;
	bool indexCreated;
	int mqttPreviewFirstEmptyColCount;

    int isPrepared();

private:
	static const unsigned int m_dataTypeLines = 10;	// lines to read for determining data types
	QString m_separator;
	int m_actualStartRow;
	int m_actualRows;
	int m_actualCols;
	int m_maxActualRows;
	int m_lastRowNum;
	int m_prepared;
	int m_columnOffset; // indexes the "start column" in the datasource. Data will be imported starting from this column.
	QVector<void*> m_dataContainer; // pointers to the actual data containers
};

#endif
