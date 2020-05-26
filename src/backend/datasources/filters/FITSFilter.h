/***************************************************************************
File                 : FITSFilter.h
Project              : LabPlot
Description          : FITS I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#ifndef FITSFILTER_H
#define FITSFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

#include <KI18n/KLocalizedString>
#include <QTableWidget>
#include <QTreeWidgetItem>

class QStringList;
class QXmlStreamReader;
class FITSFilterPrivate;
class FITSHeaderEditWidget;

class FITSFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	FITSFilter();
	~FITSFilter() override;

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;
	QVector<QStringList> readChdu(const QString& fileName, bool *okToMatrix = nullptr, int lines = -1);
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

	struct KeywordUpdate {
		KeywordUpdate() : keyUpdated(false), valueUpdated(false),
			commentUpdated(false), unitUpdated(false) {}
		bool keyUpdated;
		bool valueUpdated;
		bool commentUpdated;
		bool unitUpdated;
	};

	struct Keyword {
		Keyword(const QString& key, const QString& value, const QString& comment): key(key), value(value),
			comment(comment) {}
		Keyword() {}
		QString key;
		QString value;
		QString comment;
		QString unit;
		bool operator==(const Keyword& other) const {
			return other.key == key &&
			       other.value == value &&
			       other.comment == comment;
		}
		bool isEmpty() const {
			return key.isEmpty() && value.isEmpty() && comment.isEmpty();
		}

		KeywordUpdate updates;
	};

	static QString fileInfoString(const QString&);
	void updateKeywords(const QString& fileName, const QList<Keyword>& originals, const QVector<Keyword>& updates);
	void addNewKeyword(const QString& filename, const QList<Keyword>& keywords);
	void addKeywordUnit(const QString& fileName, const QList<Keyword>& keywords);
	void deleteKeyword(const QString& fileName, const QList<Keyword>& keywords);
	void removeExtensions(const QStringList& extensions);
	void parseHeader(const QString &fileName, QTableWidget* headerEditTable,
	                 bool readKeys = true,
	                 const QList<Keyword> &keys = QList<Keyword>());
	void parseExtensions(const QString& fileName, QTreeWidget* tw, bool checkPrimary = false);
	QList<Keyword> chduKeywords(const QString &fileName);

	static QStringList standardKeywords();
	static QStringList mandatoryImageExtensionKeywords();
	static QStringList mandatoryTableExtensionKeywords();
	static QStringList units();

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	void setStartRow(const int);
	int startRow() const;
	void setEndRow(const int);
	int endRow() const;
	void setStartColumn(const int);
	int startColumn() const;
	void setEndColumn(const int);
	int endColumn() const;
	void setCommentsAsUnits(const bool);
	void setExportTo(const int);
private:
	std::unique_ptr<FITSFilterPrivate> const d;
	friend class FITSFilterPrivate;
};
#endif // FITSFILTER_H
