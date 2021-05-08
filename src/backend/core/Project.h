/***************************************************************************
    File                 : Project.h
    Project              : LabPlot
    Description          : Represents a LabPlot project.
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs@gmx.net)
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
#ifndef PROJECT_H
#define PROJECT_H

#include "backend/core/Folder.h"
#include "backend/lib/macros.h"

class AbstractColumn;
class BoxPlot;
class Histogram;
class XYCurve;
class QMimeData;
class QString;

class Project : public Folder {
	Q_OBJECT

public:
	enum class MdiWindowVisibility {
		folderOnly,
		folderAndSubfolders,
		allMdiWindows
	};

public:
	Project();
	~Project() override;

	virtual const Project* project() const {
		return this;
	}
	Project* project() override {
		return this;
	}
	QUndoStack* undoStack() const override;
	QString path() const override {
		return name();
	}
	QMenu* createContextMenu() override;
	virtual QMenu* createFolderContextMenu(const Folder*);

	void setMdiWindowVisibility(MdiWindowVisibility visibility);
	MdiWindowVisibility mdiWindowVisibility() const;
	CLASS_D_ACCESSOR_DECL(QString, fileName, FileName)
	CLASS_D_ACCESSOR_DECL(QString, author, Author)
	CLASS_D_ACCESSOR_DECL(QDateTime, modificationTime, ModificationTime)

	void setChanged(const bool value=true);
	bool hasChanged() const;
	void navigateTo(const QString& path);

	void setSuppressAspectAddedSignal(bool);
	bool aspectAddedSignalSuppressed() const;

	void save(const QPixmap&, QXmlStreamWriter*) const;
	bool load(XmlStreamReader*, bool preview) override;
	bool load(const QString&, bool preview = false);
	static void restorePointers(AbstractAspect*, bool preview = false);

	static bool isLabPlotProject(const QString& fileName);
	static QString supportedExtensions();
	QVector<quintptr> droppedAspects(const QMimeData*);
    static QString version();
    static int versionNumber();
	static int xmlVersion();

	class Private;

public slots:
	void descriptionChanged(const AbstractAspect*);
	void aspectAddedSlot(const AbstractAspect*);

signals:
	void authorChanged(const QString&);
	void requestSaveState(QXmlStreamWriter*) const;
	void requestLoadState(XmlStreamReader*);
	void requestProjectContextMenu(QMenu*);
	void requestFolderContextMenu(const Folder*, QMenu*);
	void mdiWindowVisibilityChanged();
	void changed();
	void requestNavigateTo(const QString& path);
	void loaded();
	void closeRequested();

private:
	Private* d;
	void updateColumnDependencies(const QVector<XYCurve*>&, const AbstractColumn*) const;
	void updateColumnDependencies(const QVector<Histogram*>&, const AbstractColumn*) const;
	void updateColumnDependencies(const QVector<BoxPlot*>& boxPlots, const AbstractColumn* column) const;
	bool readProjectAttributes(XmlStreamReader*);
	void save(QXmlStreamWriter*) const override;
};

#endif // ifndef PROJECT_H
