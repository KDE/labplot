/*
	File                 : AbstractPart.h
	Project              : LabPlot
	Description          : Base class of Aspects with MDI windows as views.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACT_PART_H
#define ABSTRACT_PART_H

#include "AbstractAspect.h"

class ContentDockWidget;
class QMenu;

class AbstractPart : public AbstractAspect {
	Q_OBJECT

public:
	AbstractPart(const QString& name, AspectType type);
	~AbstractPart() override;

	virtual QWidget* view() const = 0;
	void deleteView() const;

	ContentDockWidget* dockWidget() const;
	bool dockWidgetExists() const;
	bool hasMdiSubWindow() const;

	QMenu* createContextMenu() override;
	virtual bool exportView() const = 0;
	virtual bool printView() = 0;
	virtual bool printPreview() const = 0;

	bool isDraggable() const override;
	QVector<AspectType> dropableOn() const override;

	// TODO: move these functions to a new class AbstractPartView
	virtual void registerShortcuts(){};
	virtual void unregisterShortcuts(){};

	void suppressDeletion(bool suppress);

private:
	mutable ContentDockWidget* m_dockWidget{nullptr};
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
