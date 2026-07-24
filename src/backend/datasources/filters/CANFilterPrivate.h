/*
	File                 : CANFilterPrivate.h
	Project              : LabPlot
	Description          : Private implementation class for CANFilter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef CANFILTERPRIVATE_H
#define CANFILTERPRIVATE_H

#include <QList>

#include "AbstractFileFilter.h"
#include "CANFilter.h"
#include "DBCParser.h"
#include <cmath>

class AbstractDataSource;
class CANFilter;

class CANFilterPrivate {
public:
	explicit CANFilterPrivate(CANFilter*);
	virtual ~CANFilterPrivate() {
	}
	QVector<QStringList> preview(const QString& fileName, int lines);
	int readDataFromFile(const QString& fileName,
						 AbstractDataSource* = nullptr,
						 AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace,
						 int lines = -1);

	/*!
	 * \brief readDataFromFile
	 * Read data from file and store them in the datacontainer \p m_dataContainer
	 * \param fileName
	 * \param lines
	 * \return number of messages read
	 */
	int readDataFromFile(const QString& fileName, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*);
	bool setDBCFile(const QString& filename);

	virtual bool isValid(const QString&) const = 0;

	const QVector<AbstractColumn::ColumnMode> columnModes();

	void clearParseState();

	CANFilter* const q;

	DbcParser::Signals m_signals;

	QString currentDataSetName;
	int startRow{1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};
	CANFilter::TimeHandling timeHandlingMode{CANFilter::TimeHandling::ConcatPrevious};
	bool convertTimeToSeconds{true};

private:
	virtual int readDataFromFileCommonTime(const QString& fileName, int lines = -1) = 0;
	virtual int readDataFromFileSeparateTime(const QString& fileName, int lines = -1) = 0;

protected:
	DbcParser m_dbcParser;
	struct DataContainer {
		void clear();

		template<class T>
		void appendVector(QVector<T>* data, AbstractColumn::ColumnMode cm) {
			m_dataContainer.push_back(data);
			m_columnModes.append(cm);
		}

		template<class T>
		void setData(int indexDataContainer, int indexData, T value) {
			static_cast<QVector<T>*>(m_dataContainer.at(indexDataContainer))->operator[](indexData) = value;
		}

		template<class T>
		T data(int indexDataContainer, int indexData) {
			return static_cast<QVector<T>*>(m_dataContainer.at(indexDataContainer))->at(indexData);
		}

		size_t size() const;
		const QVector<AbstractColumn::ColumnMode> columnModes() const;

		/*!
		 * \brief dataContainer
		 * Do not modify outside as long as DataContainer exists!
		 * \return
		 */
		std::vector<void*> dataContainer() const;
		AbstractColumn::ColumnMode columnMode(int index) const;
		const void* datas(size_t index) const;
		bool resize(uint32_t) const;

	private:
		QVector<AbstractColumn::ColumnMode> m_columnModes;
		std::vector<void*> m_dataContainer; // pointers to the actual data containers
	};

	DataContainer m_DataContainer;

	struct ParseState {
		ParseState() {
		}
		ParseState(int linesRequested, int linesRead)
			: ready(true)
			, requestedLines(linesRequested)
			, readLines(linesRead) {
		}
		bool ready{false}; // If true m_DataContainer is up to date and it is not needed to update
		int requestedLines{0}; // Lines requested during read
		int readLines{0}; // Lines after read (Because file does not contain that much messages, a lot of message are not parsable, ...)
	};

	ParseState m_parseState;
	friend class BLFFilterTest;
	friend class CANFilter;
};

#endif // CANFILTERPRIVATE_H
