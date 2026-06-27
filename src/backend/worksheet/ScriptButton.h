/*
	File                 : ScriptButton.h
	Project              : LabPlot
	Description          : Worksheet element representing a button that executes a script
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTBUTTON_H
#define SCRIPTBUTTON_H

#include "backend/worksheet/WorksheetElement.h"
#include "backend/lib/macros.h"


class Script;
class ScriptButtonPrivate;
class QBrush;
class QPen;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT ScriptButton : public WorksheetElement {
#else
class ScriptButton : public WorksheetElement {
#endif
	Q_OBJECT

public:
	explicit ScriptButton(const QString& name);
	~ScriptButton() override;

	QIcon icon() const override;
	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	QWidget* widget() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	// void loadThemeConfig(const KConfig&) override;
	// void saveThemeConfig(const KConfig&) override;

	// properties
	POINTER_D_ACCESSOR_DECL(const Script, script, Script)
	CLASS_D_ACCESSOR_DECL(QString, text, Text)
	BASIC_D_ACCESSOR_DECL(int, width, Width)
	BASIC_D_ACCESSOR_DECL(int, height, Height)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundColor, BackgroundColor)
	CLASS_D_ACCESSOR_DECL(QColor, textColor, TextColor)
	CLASS_D_ACCESSOR_DECL(QFont, font, Font)

	typedef ScriptButtonPrivate Private;

private:
	Q_DECLARE_PRIVATE(ScriptButton)
	void init();

Q_SIGNALS:
	void scriptChanged(Script*);
	void textChanged(const QString&);
	void widthChanged(int);
	void heightChanged(int);
	void backgroundColorChanged(const QColor&);
	void textColorChanged(const QColor&);
	void fontChanged(const QFont&);
};

#endif
