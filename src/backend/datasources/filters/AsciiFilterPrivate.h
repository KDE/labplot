/*
	File                 : AsciiFilterPrivate.h
	Project              : LabPlot
	Description          : Private implementation class for AsciiFilter.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASCIIFILTERPRIVATE_H
#define ASCIIFILTERPRIVATE_H

#include <QString>
#include <QLocale>

#include "AsciiFilter.h"

class AsciiFilterPrivate {
public:
	AsciiFilterPrivate(AsciiFilter *owner);

	AsciiFilter::Status initialize(QIODevice& device);
	AsciiFilter::Status readFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines);
	QVector<QStringList> preview(const QString& fileName, int lines);

	AsciiFilter::Properties properties;
	bool initialized{true}; // false if an initialization with the current properties was done, otherwise true

private:
	static QStringList determineColumns(const QString& line, const AsciiFilter::Properties& properties);
	static QStringList determineColumns(const QString& line, const QString& separator, bool removeQuotesEnabled, bool simplifyWhiteSpaces, bool skipEmptyParts, int startColumn, int numberColumns);
	static AsciiFilter::Status determineSeparator(const QString &line, bool removeQuotesEnabled, bool simplifyWhiteSpaces, QString &separator);
	static QVector<AbstractColumn::ColumnMode> determineColumnModes(const QVector<QStringList>& values, const AsciiFilter::Properties& properties, QString &dateTimeFormat);
	AsciiFilter::Status getLine(QIODevice& device, QString& line);
	static QString deviceStatusToString(AsciiFilter::Status);
	void setLastError(AsciiFilter::Status);

	// Copied from CANFilterPrivate
	// TODO: think about moving it to a common place
	struct DataContainer {
		void clear();
		void appendVector(AbstractColumn::ColumnMode cm);
		size_t elementCount(size_t index = 0) const;

		template<class T>
		void appendVector(QVector<T>* data, AbstractColumn::ColumnMode cm) {
			m_dataContainer.push_back(data);
			m_columnModes.append(cm);
		}

		void appendVector(void* data, AbstractColumn::ColumnMode cm) {
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
		std::vector<void*>& dataContainer();
		AbstractColumn::ColumnMode columnMode(int index) const;
		const void* datas(size_t index) const;
		bool resize(uint32_t) const;
		bool reserve(uint32_t) const;

	private:
		QVector<AbstractColumn::ColumnMode> m_columnModes;
		std::vector<void*> m_dataContainer; // pointers to the actual data containers
	};

	DataContainer m_DataContainer;

private:
	AsciiFilter* const q;
	static const size_t m_dataTypeLines = 10; // maximum lines to read for determining data types

	friend class AsciiFilterTest;
};

#endif // ASCIIFILTERPRIVATE_H
