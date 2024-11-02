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

#include <QLocale>
#include <QString>

#include "AsciiFilter.h"

class AsciiFilterPrivate {
public:
	AsciiFilterPrivate(AsciiFilter* owner);
	AsciiFilter::Status initialize(AsciiFilter::Properties p);
	AsciiFilter::Status initialize(QIODevice& device);
	void setDataSource(AbstractDataSource* dataSource);
	AsciiFilter::Status readFromDevice(QIODevice& device,
									   AbstractFileFilter::ImportMode columnImportMode,
									   AbstractFileFilter::ImportMode rowImportMode,
									   qint64 from,
									   qint64 lines,
									   qint64 keepNRows,
									   qint64& bytes_read,
									   bool skipFirstLine = false);
	QVector<QStringList> preview(QIODevice& device, int lines, bool reinit = true, bool skipFirstLine = false);
	QVector<QStringList> preview(const QString& fileName, int lines, bool reinit = true);

	static QMap<QString, QPair<QString, AbstractColumn::ColumnMode>> modeMap();
	static bool determineColumnModes(const QStringView& s, QVector<AbstractColumn::ColumnMode>& modes, QString& invalidString);
	static QString convertTranslatedColumnModesToNative(const QStringView s);
	AsciiFilter::Status setLastError(AsciiFilter::Status);

	AsciiFilter::Properties properties;
	bool initialized{false};
	size_t fileNumberLines{0};

private:
	static bool ignoringLine(QStringView line, const AsciiFilter::Properties& p);
	static QStringList determineColumnsSimplifyWhiteSpace(const QStringView& line, const AsciiFilter::Properties& properties);
	static QStringList determineColumnsSimplifyWhiteSpace(QStringView line,
														  const QString& separator,
														  bool removeQuotes,
														  bool simplifyWhiteSpaces,
														  bool skipEmptyParts,
														  int startColumn,
														  int endColumn);
	static size_t determineColumns(const QStringView& line, const AsciiFilter::Properties& properties, QVector<QStringView> &columnValues);
	static AsciiFilter::Status determineSeparator(const QString& line, bool removeQuotes, bool simplifyWhiteSpaces, QString& separator);
	static QVector<AbstractColumn::ColumnMode>
	determineColumnModes(const QVector<QStringList>& values, const AsciiFilter::Properties& properties, QString& dateTimeFormat);
	AsciiFilter::Status getLine(QIODevice& device, QString& line);
	static QString statusToString(AsciiFilter::Status);

	template<typename T>
	void setValues(const QVector<T>& values, int rowIndex, const AsciiFilter::Properties& properties);

	// Copied from CANFilterPrivate
	// TODO: think about moving it to a common place
	struct DataContainer {
		void clear();
		void appendVector(AbstractColumn::ColumnMode cm);
		int rowCount(unsigned long index = 0) const;

		template<class T>
		void appendVector(QVector<T>* data, AbstractColumn::ColumnMode cm) {
			m_dataContainer.push_back(data);
			m_columnModes.append(cm);
		}

		void appendVector(void* data, AbstractColumn::ColumnMode cm) {
			m_dataContainer.push_back(data);
			m_columnModes.append(cm);
		}

		// Removes the first n elements
		void removeFirst(int n);

		template<class T>
		void setData(int indexDataContainer, int indexData, T value) {
			static_cast<QVector<T>*>(m_dataContainer.at(indexDataContainer))->operator[](indexData) = value;
		}

		void setData(int indexDataContainer, int indexData, const QStringView& value) {
			static_cast<QVector<QString>*>(m_dataContainer.at(indexDataContainer))->operator[](indexData) = value.toString();
		}

		template<class T>
		T data(int indexDataContainer, int indexData) {
			return static_cast<QVector<T>*>(m_dataContainer.at(indexDataContainer))->at(indexData);
		}

		qsizetype size() const;
		const QVector<AbstractColumn::ColumnMode> columnModes() const;

		/*!
		 * \brief dataContainer
		 * Do not modify outside as long as DataContainer exists!
		 * \return
		 */
		std::vector<void*>& dataContainer();
		AbstractColumn::ColumnMode columnMode(int index) const;
		const void* datas(qsizetype index) const;
		bool resize(qsizetype) const;
		bool reserve(qsizetype) const;

	private:
		QVector<AbstractColumn::ColumnMode> m_columnModes;
		std::vector<void*> m_dataContainer; // pointers to the actual data containers
	};

	DataContainer m_DataContainer;
	qint64 m_index{1}; // Index counter used when a index column was prepended
	AsciiFilter::Status lastStatus{AsciiFilter::Status::Success};
	AbstractDataSource* m_dataSource{nullptr};

private:
	AsciiFilter* const q;
	static const qsizetype m_dataTypeLines = 10; // maximum lines to read for determining data types
	qsizetype numberRowsReallocation = 10000; // When importing new data reallocate that amount of rows. So not for every row it must be reallocated

	friend class AsciiFilterTest;
};

#endif // ASCIIFILTERPRIVATE_H
