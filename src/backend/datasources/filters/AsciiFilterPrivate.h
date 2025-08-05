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

#include "AsciiFilter.h"
#include "FilterStatus.h"
#include <QString>

struct Status;

class AsciiFilterPrivate {
public:
	AsciiFilterPrivate(AsciiFilter* owner);
	Status initialize(AsciiFilter::Properties);
	Status initialize(QIODevice&);
	void setDataSource(AbstractDataSource*);
	Status readFromDevice(QIODevice&,
						  AbstractFileFilter::ImportMode columnImportMode,
						  AbstractFileFilter::ImportMode rowImportMode,
						  qint64 from,
						  qint64 lines,
						  qint64 keepNRows,
						  qint64& bytes_read,
						  bool skipFirstLine = false);
	QVector<QStringList> preview(QIODevice&, int lines, bool reinit = true, bool skipFirstLine = false);
	QVector<QStringList> preview(const QString& fileName, int lines, bool reinit = true);

	static QMap<QString, QPair<QString, AbstractColumn::ColumnMode>> modeMap();
	static bool determineColumnModes(const QStringView&, QVector<AbstractColumn::ColumnMode>&, QString& invalidString);
	static QString convertTranslatedColumnModesToNative(const QStringView);
	Status setLastError(Status);
	bool isUTF16(QIODevice&);

	AsciiFilter::Properties properties;
	bool initialized{false};
	size_t fileNumberLines{0};

private:
	static bool ignoringLine(QStringView line, const AsciiFilter::Properties&);
	static QStringList determineColumnsSimplifyWhiteSpace(const QStringView& line, const AsciiFilter::Properties&);
	static QStringList determineColumnsSimplifyWhiteSpace(QStringView line,
														  const QString& separator,
														  bool removeQuotes,
														  bool simplifyWhiteSpaces,
														  bool skipEmptyParts,
														  int startColumn,
														  int endColumn);
	static size_t determineColumns(const QStringView& line,
								   const AsciiFilter::Properties&,
								   bool separatorSingleCharacter,
								   const QChar separatorCharacter,
								   QVector<QStringView>& columnValues);
	static Status determineSeparator(const QString& line, bool removeQuotes, bool simplifyWhiteSpaces, QString& separator);
	static QVector<AbstractColumn::ColumnMode>
	determineColumnModes(const QVector<QStringList>& values, const AsciiFilter::Properties&, QString& dateTimeFormat);
	Status getLine(QIODevice&, QString& line);

	template<typename T>
	void setValues(const QVector<T>& values, int rowIndex, const AsciiFilter::Properties&);

	// Copied from CANFilterPrivate
	// TODO: think about moving it to a common place
	struct DataContainer {
		void clear();
		void appendVector(AbstractColumn::ColumnMode);
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
	Status lastStatus;
	AbstractDataSource* m_dataSource{nullptr};

private:
	AsciiFilter* const q;
	static const qsizetype m_dataTypeLines = 10; // maximum lines to read for determining data types
	qsizetype numberRowsReallocation = 10000; // When importing new data reallocate that amount of rows. So not for every row it must be reallocated

	friend class AsciiFilterTest;
};

#endif // ASCIIFILTERPRIVATE_H
