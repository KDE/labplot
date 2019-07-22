/***************************************************************************
    File                 : Folder.h
    Project              : LabPlot
    Description          : Folder in a project
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2007 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007 Knut Franke (knut.franke@gmx.de)

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

	bool isDraggable() const override;
	QVector<AspectType> dropableOn() const override;
	void processDropEvent(QDropEvent*) override;

private:
	QStringList m_pathesToLoad;

protected:
	bool readChildAspectElement(XmlStreamReader*, bool preview);
};

#endif // ifndef FOLDER_H
