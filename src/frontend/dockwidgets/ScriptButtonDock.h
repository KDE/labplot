/*
	File                 : ScriptButtonDock.h
	Project              : LabPlot
	Description          : widget to edit the properties of a script button
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTBUTTONDOCK_H
#define SCRIPTBUTTONDOCK_H

#include "frontend/dockwidgets/BaseDock.h"
#include "ui_scriptbuttondock.h"

class ScriptButton;
class Script;
class TreeViewComboBox;
class KConfig;
class QModelIndex;

class ScriptButtonDock : public BaseDock {
	Q_OBJECT

public:
	explicit ScriptButtonDock(QWidget*);
	void setScriptButtons(QList<ScriptButton*>);

	void updateLocale() override;
	void retranslateUi() override;
	void updateUnits() override;

private:
	Ui::ScriptButtonDock ui;
	TreeViewComboBox* cbScript{nullptr};
	QList<ScriptButton*> m_scriptButtons;
	ScriptButton* m_scriptButton{nullptr};

	void load();
	void loadConfig(KConfig&);
	void setModel();

private Q_SLOTS:
	// SLOTs for changes triggered in ScriptButtonDock
	void textChanged();
	void scriptChanged(const QModelIndex&);
	void fontChanged(const QFont&);
	void fontColorChanged(const QColor&);
	void backgroundColorChanged(const QColor&);

	// geometry
	void widthChanged(double);
	void heightChanged(double);

	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
	void horizontalAlignmentChanged(int);
	void verticalAlignmentChanged(int);

	// SLOTs for changes triggered in ScriptButton
	void buttonTextChanged(const QString&);
	void buttonScriptChanged(Script*);
	void buttonFontChanged(const QFont&);
	void buttonTextColorChanged(const QColor&);
	void buttonBackgroundColorChanged(const QColor&);
	void buttonWidthChanged(int);
	void buttonHeightChanged(int);

	void buttonPositionChanged(const WorksheetElement::PositionWrapper&);
	void buttonHorizontalAlignmentChanged(WorksheetElement::HorizontalAlignment);
	void buttonVerticalAlignmentChanged(WorksheetElement::VerticalAlignment);

Q_SIGNALS:
	void info(const QString&);
};

#endif // SCRIPTBUTTONDOCK_H
