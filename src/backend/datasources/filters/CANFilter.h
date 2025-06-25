/*
	File                 : VectorBLFFilter.h
	Project              : LabPlot
	Description          : Vector BLF I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef CANFILTER_H
#define CANFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class CANFilterPrivate;

/*!
	\class CANFilter
	\brief Manages the import/export of data which are using dbc files for parsing the log files

	\ingroup datasources
*/
#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT CANFilter : public AbstractFileFilter {
#else
class CANFilter : public AbstractFileFilter {
#endif
	Q_OBJECT

public:
	CANFilter(FileType type, CANFilterPrivate*);
	~CANFilter() override;

	/*!
	 * \brief fileInfoString
	 * Information about the file content
	 * \return
	 */
	static QString fileInfoString(const QString&);
	QVector<QStringList> preview(const QString& filename, int lines);

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;

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
	std::unique_ptr<CANFilterPrivate> const d;

	std::vector<void*> dataContainer() const;

	friend class BLFFilterTest;
};

#endif // CANFILTER_H
