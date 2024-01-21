/*
	File                 : ProjectParser.h
	Project              : LabPlot
	Description          : base class for project parsers
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROJECTPARSER_H
#define PROJECTPARSER_H

#include <QObject>

class Folder;
class Project;
class QAbstractItemModel;
class QString;

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

	QList<AspectType> topLevelClasses() const;

protected:
	virtual bool load(Project*, bool preview) = 0;

	QString m_projectFileName;
	Project* m_previewProject{nullptr};
	QList<AspectType> m_topLevelClasses;

private:
	void moveFolder(Folder* targetParentFolder, Folder* sourceChildFolderToMove) const;

Q_SIGNALS:
	void completed(int);
};

#endif // PROJECTPARSER_H
