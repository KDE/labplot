/***************************************************************************
File                 : ROOTFilter.h
Project              : LabPlot
Description          : ROOT(CERN) I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2018 Christoph Roick (chrisito@gmx.de)
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

#ifndef ROOTFILTER_H
#define ROOTFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class ROOTFilterPrivate;
class QStringList;
class QIODevice;


/// Manages the importing of histograms from ROOT files
class ROOTFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	ROOTFilter();
	~ROOTFilter() override;

    enum ColumnTypes {Center = 1, Low = 2, Content = 4, Error = 8};

	/**
	 * @brief Read data from the currently selected histogram
	 *
	 * The ROOT file is kept open until the file name is changed
	 */
	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* dataSource,
	                                      AbstractFileFilter::ImportMode importMode, int) override;
	/// Currently writing to ROOT files is not supported
	void write(const QString& fileName, AbstractDataSource*) override;

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	/// List names of histograms contained in ROOT file
	QStringList listHistograms(const QString& fileName);

	/// Set the current histograms, which is one out of listHistograms
	void setCurrentHistogram(const QString&);
	/// Get the name of the currently set histogram
	const QString currentHistogram() const;

	/// Get preview data of the currently set histogram
	QVector<QStringList> previewCurrentHistogram(const QString& fileName,
	                                             int first, int last);

	/// Get the number of bins in the current histogram
	int binsInCurrentHistogram(const QString& fileName);

	/**
	 * @brief Set the first bin of the histogram to be read
	 *
	 * The default of -1 skips the underflow bin with index 0
	 */
	void setStartBin(const int bin);
	/// Get the index of the first bin to be read
	int startBin() const;
	/**
	 * @brief Set the last bin of the histogram to be read
	 *
	 * The default of -1 skips the overflow bin
	 */
	void setEndBin(const int bin);
	/// Get the index of the last bin to be read
	int endBin() const;

	/**
	 * @brief Set the first column of the histogram to be read
	 *
	 * The following columns are available: Bin Center, Content, Error
	 */
	void setColumns(const int columns);
	/// Get the index of the first column to be read
	int columns() const;

	/// Save bin limitation settings
	void save(QXmlStreamWriter*) const override;
	/// Load bin limitation settings
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<ROOTFilterPrivate> const d;
	friend class ROOTFilterPrivate;
};

#endif
