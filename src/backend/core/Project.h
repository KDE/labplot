/*
	File                 : Project.h
	Project              : LabPlot
	Description          : Represents a LabPlot project.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2007-2008 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke@gmx.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROJECT_H
#define PROJECT_H

#include "backend/core/Folder.h"
#include "backend/lib/macros.h"

class AbstractColumn;
class Spreadsheet;
class ProjectPrivate;

class QMimeData;
class QString;

Project* project();

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Project : public Folder {
#else
class Project : public Folder {
#endif
	Q_OBJECT

public:
	enum class DockVisibility { folderOnly, folderAndSubfolders, allDocks };

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

	void setDockVisibility(DockVisibility visibility);
	DockVisibility dockVisibility() const;
	CLASS_D_ACCESSOR_DECL(QString, fileName, FileName)
	CLASS_D_ACCESSOR_DECL(QString, author, Author)
	CLASS_D_ACCESSOR_DECL(QDateTime, modificationTime, ModificationTime)
	BASIC_D_ACCESSOR_DECL(bool, saveCalculations, SaveCalculations)
	CLASS_D_ACCESSOR_DECL(QString, dockWidgetState, DockWidgetState)
	BASIC_D_ACCESSOR_DECL(bool, saveDefaultDockWidgetState, SaveDefaultDockWidgetState)
	CLASS_D_ACCESSOR_DECL(QString, defaultDockWidgetState, DefaultDockWidgetState)

	bool hasChanged() const;
	void navigateTo(const QString& path);

	void setSuppressAspectAddedSignal(bool);
	bool aspectAddedSignalSuppressed() const;

	void save(const QPixmap&, QXmlStreamWriter*);
	bool load(XmlStreamReader*, bool preview) override;
	bool load(const QString&, bool preview = false);
#ifndef SDK
	bool loadNotebook(const QString&);
#endif
	static void restorePointers(AbstractAspect*);
	static void retransformElements(AbstractAspect*);

	static bool isSupportedProject(const QString& fileName);
	static bool isLabPlotProject(const QString& fileName);
	static QString supportedExtensions();
	QVector<quintptr> droppedAspects(const QMimeData*);
	static QString version();
	static int versionNumber();
	static int xmlVersion();
	static void setXmlVersion(int version);
	static int currentBuildXmlVersion();

	typedef ProjectPrivate Private;

	static Project* currentProject;

public Q_SLOTS:
	void descriptionChanged(const AbstractAspect*);
	void aspectAddedSlot(const AbstractAspect*);

Q_SIGNALS:
	void authorChanged(const QString&);
	void saveDefaultDockWidgetStateChanged(bool);
	void saveCalculationsChanged(bool);
	void requestSaveState(QXmlStreamWriter*) const;
	void requestLoadState(XmlStreamReader*);
	void requestProjectContextMenu(QMenu*);
	void requestFolderContextMenu(const Folder*, QMenu*);
	void mdiWindowVisibilityChanged();
	void changed();
	void requestNavigateTo(const QString& path);
	void closeRequested();
	void saved() const;
	void loaded() const;
	void aboutToClose() const;

private:
	Q_DECLARE_PRIVATE(Project)
	ProjectPrivate* const d_ptr;
	void updateSpreadsheetDependencies(const Spreadsheet*) const;
	bool readProjectAttributes(XmlStreamReader*);
	void save(QXmlStreamWriter*) const override;

	template<typename T>
	void updateDependencies(const QVector<const AbstractAspect*>);

private:
	void setChanged(const bool value = true);
	friend class AbstractAspect;
	friend class MainWin;
	friend class ImportDialog;
};

#endif // ifndef PROJECT_H
