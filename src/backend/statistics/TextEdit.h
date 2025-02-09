/***************************************************************************
	File                 : TextEdit.h
	Project              : LabPlot
	Description          : Derived class of QTextEdit with enhanced tooltip support
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal (agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2025  Kuntal Bar (barkuntal6@gmail.com)
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QMap>
#include <QPair>
#include <QTextEdit>

class TextEdit : public QTextEdit {
	Q_OBJECT

public:
	explicit TextEdit(QWidget* parent = nullptr);
	void setHtml(const QString& htmlText);

protected:
	bool event(QEvent* event) override;

private:
	// Processes the tooltip markup in the given text.
	// If updateTooltips is true, the extracted tooltip ranges and texts
	// are stored in m_tooltips.
	void processToolTipTags(QString& text, bool updateTooltips);

	// Map of text ranges (start, end) to tooltip text.
	QMap<QPair<int, int>, QString> m_tooltips;
};

#endif
