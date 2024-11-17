/*
	File                 : AbstractPart.h
	Project              : LabPlot
	Description          : Base class of Aspects with MDI windows as views.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACT_PART_H
#define ABSTRACT_PART_H

#include "AbstractAspect.h"
#ifndef SDK
class ContentDockWidget;
#endif
class QMenu;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT AbstractPart : public AbstractAspect {
#else
class AbstractPart : public AbstractAspect {
#endif
	Q_OBJECT

public:
	AbstractPart(const QString& name, AspectType type);
	~AbstractPart() override;

	virtual QWidget* view() const = 0;
	bool viewCreated() const;
	void deleteView() const;
#ifndef SDK
	ContentDockWidget* dockWidget() const;
#endif
	bool dockWidgetExists() const;
	bool hasMdiSubWindow() const;

	QMenu* createContextMenu() override;
	virtual bool exportView() const = 0;
	virtual bool printView() = 0;
	virtual bool printPreview() const = 0;

	bool isDraggable() const override;
	QVector<AspectType> dropableOn() const override;

	// TODO: move these functions to a new class AbstractPartView
	virtual void registerShortcuts() { };
	virtual void unregisterShortcuts() { };

	void suppressDeletion(bool suppress);

private:
#ifndef SDK
	mutable ContentDockWidget* m_dockWidget{nullptr};
#endif
	bool m_suppressDeletion{false};

protected:
	mutable QWidget* m_partView{nullptr};

Q_SIGNALS:
	void showRequested();
	void importFromFileRequested();
	void importFromSQLDatabaseRequested();
	void exportRequested();
	void printRequested();
	void printPreviewRequested();
	void viewAboutToBeDeleted() const;
};

#endif // ifndef ABSTRACT_PART_H
