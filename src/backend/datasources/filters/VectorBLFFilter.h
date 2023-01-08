/*
	File                 : VectorBLFFilter.h
	Project              : LabPlot
	Description          : Vector BLF I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef VECTORBLFFILTER_H
#define VECTORBLFFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class QStringList;
class QTreeWidgetItem;
class VectorBLFFilterPrivate;

class VectorBLFFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	VectorBLFFilter();
	~VectorBLFFilter() override;

	static bool isValid(const QString& filename);
	/*!
	 * \brief fileInfoString
	 * Information about the file content
	 * \return
	 */
	static QString fileInfoString(const QString&);
	QVector<QStringList> preview(const QString& filename, int lines);

	void
	readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	QStringList vectorNames() const;
	const QVector<AbstractColumn::ColumnMode> columnModes() const;

	enum class TimeHandling {
		Separate, // separate timestamp for every message id
		ConcatNAN, // common timestamp vector for all messages, but NAN if the current value is not available
		ConcatPrevious // common timestamp vector for all messages, but use the previous value if available, otherwise 0
	};

	void setConvertTimeToSeconds(bool);
	void setTimeHandlingMode(TimeHandling);

	/*!
	 * \brief setDBCFile
	 * Sets dbc file which is used to decode the messages
	 * \param file
	 * \return
	 */
	bool setDBCFile(const QString& file);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<VectorBLFFilterPrivate> const d;
	friend class VectorBLFFilterPrivate;
};

#endif // VECTORBLFFILTER_H
