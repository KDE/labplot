/*
    File                 : Folder.h
    Project              : LabPlot
    Description          : Folder in a project
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2010-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FOLDER_H
#define FOLDER_H

#include "AbstractAspect.h"

class Folder : public AbstractAspect {
Q_OBJECT

public:
	explicit Folder(const QString& name, AspectType type = AspectType::Folder);

	QIcon icon() const override;
	QMenu* createContextMenu() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void setPathesToLoad(const QStringList&);
	const QStringList& pathesToLoad() const;

	QVector<AspectType> pasteTypes() const override;
	bool isDraggable() const override;
	QVector<AspectType> dropableOn() const override;
	void processDropEvent(const QVector<quintptr>&) override;

private:
	QStringList m_pathesToLoad;

protected:
	bool readChildAspectElement(XmlStreamReader*, bool preview);
};

#endif // ifndef FOLDER_H
