/*
File                 : ROOTFilter.h
Project              : LabPlot
Description          : ROOT(CERN) I/O-filter
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Christoph Roick (chrisito@gmx.de)
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ROOTFILTER_H
#define ROOTFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

#include <QVector>

class QStringList;
class ROOTFilterPrivate;
class QStringList;
class QIODevice;

/// Manages the importing of histograms from ROOT files
class ROOTFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	ROOTFilter();
	~ROOTFilter() override;

	//UNUSED enum class ColumnTypes {Center = 1, Low = 2, Content = 4, Error = 8};

	static QString fileInfoString(const QString&);

	/**
	 * @brief Read data from the currently selected histogram
	 *
	 * The ROOT file is kept open until the file name is changed
	 */
	void readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) override;
	/// Currently writing to ROOT files is not supported
	void write(const QString& fileName, AbstractDataSource*) override;

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	/// Internal directory structure in a ROOT file
	struct Directory
	{
		QString name;
		QVector<QPair<QString, quint64> > content;
		QVector<Directory> children;
	};

	/// List names of histograms contained in ROOT file
	Directory listHistograms(const QString& fileName) const;
	/// List names of trees contained in ROOT file
	Directory listTrees(const QString& fileName) const;
	/// List names of leaves contained in ROOT tree
	QVector<QStringList> listLeaves(const QString& fileName, qint64 pos) const;

	/// Set the current histograms, which is one out of listHistograms
	void setCurrentObject(const QString&);
	/// Get the name of the currently set object
	const QString currentObject() const;

	/// Get preview data of the currently set object
	QVector<QStringList> previewCurrentObject(const QString& fileName, int first, int last) const;

	/// Get the number of rows in the current object
	int rowsInCurrentObject(const QString& fileName) const;

	/**
	 * @brief Set the last bin of the object to be read
	 *
	 * -1 skips the underflow bin of histograms
	 */
	void setStartRow(const int bin);
	/// Get the index of the first row to be read
	int startRow() const;
	/**
	 * @brief Set the last row of the object to be read
	 *
	 * -1 skips the overflow bin of histograms
	 */
	void setEndRow(const int bin);
	/// Get the index of the last row to be read
	int endRow() const;

	/**
	 * @brief Set the columns of the object to be read
	 *
	 * For histograms the following are available: center, low, content, error
	 */
	void setColumns(const QVector<QStringList>& columns);
	/**
	 * @brief Get the columns to be read
	 *
	 * For histograms, the identifiers for location, content and error are given
	 * as the first part, the corresponding translation as the second part.
	 * For trees, the branch name and the leaf name are returned.
	 *
	 * @return A pair of strings with different content depending on the object type
	 */
	QVector<QStringList> columns() const;

	/// Save bin limitation settings
	void save(QXmlStreamWriter*) const override;
	/// Load bin limitation settings
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<ROOTFilterPrivate> const d;
	friend class ROOTFilterPrivate;
};

#endif
