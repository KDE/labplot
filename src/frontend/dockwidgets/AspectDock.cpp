/*
	File                 : AspectDock.h
	Project              : LabPlot
	Description          : widget for aspect properties showing name and comments only
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AspectDock.h"
#include "backend/core/AbstractColumn.h"

/*!
  \class AspectDock
  \brief Provides a widget for editing the properties of the aspects where only the name and comments need to be modified.
  Example objects are Folder, Workbook, etc.

  \ingroup frontend
*/

AspectDock::AspectDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment, 1.2);
}

void AspectDock::setAspects(QList<AbstractAspect*> list) {
	BaseDock::setAspects(list);

	CONDITIONAL_LOCK_RETURN;
	if (list.size() == 1) {
		ui.leName->setEnabled(true);
		ui.teComment->setEnabled(true);

		ui.leName->setText(aspect()->name());
		ui.teComment->setText(aspect()->comment());
	} else {
		ui.leName->setEnabled(false);
		ui.teComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}

	// slots
}

//*************************************************************
//********* SLOTs for changes triggered in Column *************
//*************************************************************
void AspectDock::aspectDescriptionChanged(const AbstractAspect* aspect) {
	if (this->aspect() != aspect)
		return;

	CONDITIONAL_LOCK_RETURN;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.teComment->text())
		ui.teComment->setText(aspect->comment());
}
