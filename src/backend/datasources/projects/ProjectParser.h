/***************************************************************************
    File                 : ProjectParser.h
    Project              : LabPlot
    Description          : base class for project parsers
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)

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
#ifndef PROJECTPARSER_H
#define PROJECTPARSER_H

#include <QObject>

class QAbstractItemModel;
class QString;
class Folder;
class Project;

enum class AspectType : quint64;

class ProjectParser : public QObject {
	Q_OBJECT

public:
	ProjectParser();
	~ProjectParser() override;

	void setProjectFileName(const QString&);
	const QString& projectFileName() const;

	QAbstractItemModel* model();
	void importTo(Folder*, const QStringList&);

	QList<AspectType> topLevelClasses() const ;

protected:
	virtual bool load(Project*, bool preview) = 0;

	QString m_projectFileName;
	Project* m_project{nullptr};
	QList<AspectType>  m_topLevelClasses;

private:
	void moveFolder(Folder* targetParentFolder, Folder* sourceChildFolderToMove) const;

signals:
	void completed(int);
};

#endif // PROJECTPARSER_H
