/*
	File                 : ScriptWorksheetElement.h
	Project              : LabPlot
	Description          : Worksheet element representing a button that executes a script
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTWORKSHEETELEMENT_H
#define SCRIPTWORKSHEETELEMENT_H

#include "backend/worksheet/WorksheetElement.h"

class Script;
class QBrush;
class QPen;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT ScriptWorksheetElement : public WorksheetElement {
#else
class ScriptWorksheetElement : public WorksheetElement {
#endif
	Q_OBJECT

public:
	explicit ScriptWorksheetElement(const QString& name);
	~ScriptWorksheetElement() override;

	QIcon icon() const override;
	void setParentGraphicsItem(QGraphicsItem*) override;
	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	QGraphicsItem* graphicsItem() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	// Properties
	QString text() const;
	void setText(const QString&);
	
	int width() const;
	void setWidth(int);
	
	int height() const;
	void setHeight(int);
	
	QColor backgroundColor() const;
	void setBackgroundColor(const QColor&);
	
	QColor borderColor() const;
	void setBorderColor(const QColor&);
	
	double borderWidth() const;
	void setBorderWidth(double);
	
	QFont font() const;
	void setFont(const QFont&);
	
	QColor textColor() const;
	void setTextColor(const QColor&);

	// Script association
	Script* script() const;
	void setScript(Script*);

public Q_SLOTS:
	void executeScript();

private:
	void init();

Q_SIGNALS:
	void textChanged(const QString&);
	void widthChanged(int);
	void heightChanged(int);
	void backgroundColorChanged(const QColor&);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void fontChanged(const QFont&);
	void textColorChanged(const QColor&);
	void scriptChanged(Script*);
};

#endif
