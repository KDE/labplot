/***************************************************************************
	File                 : TextEdit.cpp
	Project              : LabPlot
	Description          : Derived class of QTextEdit with enhanced tooltip support
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal (agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2025  Kuntal Bar (barkuntal6@gmail.com)
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "TextEdit.h"
#include "backend/lib/macros.h" // if needed for i18n or debug macros

#include <QHelpEvent>
#include <QTextCursor>
#include <QToolTip>

TextEdit::TextEdit(QWidget* parent)
	: QTextEdit(parent) {
}

bool TextEdit::event(QEvent* event) {
	if (event->type() == QEvent::ToolTip || event->type() == QEvent::WhatsThis) {
		QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);

		// Obtain the text cursor position corresponding to the event's position.
		QTextCursor origCursor = textCursor();
		int cursorPos = cursorForPosition(helpEvent->pos()).position();
		setTextCursor(origCursor);

		// Look for a tooltip whose range covers the cursor position.
		QString tooltipText;
		for (auto it = m_tooltips.constBegin(); it != m_tooltips.constEnd(); ++it) {
			const QPair<int, int>& range = it.key();
			if (cursorPos >= range.first && cursorPos <= range.second) {
				tooltipText = it.value();
				break;
			}
		}

		if (!tooltipText.isEmpty())
			QToolTip::showText(helpEvent->globalPos(), tooltipText);
		else
			QToolTip::hideText();

		return true;
	}
	return QTextEdit::event(event);
}

void TextEdit::setHtml(const QString& htmlText) {
	// Set the provided HTML first.
	QTextEdit::setHtml(htmlText);

	// Clear any previously stored tooltip ranges.
	m_tooltips.clear();

	// Process the plain-text version and HTML version to extract tooltip markup.
	QString plainText = toPlainText();
	processToolTipTags(plainText, true);
	QString processedHtml = htmlText;
	processToolTipTags(processedHtml, false);

	// Finally, update the HTML content with tooltip markup removed.
	QTextEdit::setHtml(processedHtml);
}

void TextEdit::processToolTipTags(QString& text, bool updateTooltips) {
	// Define the markers.
	const QString startToolTip = QLatin1String("[tooltip]");
	const QString endToolTip = QLatin1String("[/tooltip]");
	const QString startData = QLatin1String("[data]");
	const QString endData = QLatin1String("[/data]");
	const QString startTip = QLatin1String("[tip]");
	const QString endTip = QLatin1String("[/tip]");

	int searchPos = 0;
	// Look for each occurrence of the tooltip start tag.
	while ((searchPos = text.indexOf(startToolTip, searchPos)) != -1) {
		int tooltipEndPos = text.indexOf(endToolTip, searchPos);
		if (tooltipEndPos == -1)
			break; // No matching end tag found

		// Attempt to locate the data block inside the tooltip.
		int dataStartPos = text.indexOf(startData, searchPos);
		int dataEndPos = text.indexOf(endData, searchPos);
		QString dataText;
		if (dataStartPos != -1 && dataEndPos != -1 && dataStartPos < tooltipEndPos && dataEndPos < tooltipEndPos) {
			dataText = text.mid(dataStartPos + startData.length(), dataEndPos - dataStartPos - startData.length());
		} else {
			dataText.clear();
			dataEndPos = searchPos + startToolTip.length();
		}

		// Attempt to locate the tip (actual tooltip) block.
		int tipStartPos = text.indexOf(startTip, dataEndPos);
		int tipEndPos = text.indexOf(endTip, dataEndPos);
		QString tipText;
		if (tipStartPos != -1 && tipEndPos != -1 && tipStartPos < tooltipEndPos && tipEndPos < tooltipEndPos) {
			tipText = text.mid(tipStartPos + startTip.length(), tipEndPos - tipStartPos - startTip.length());
		} else {
			tipText.clear();
		}

		// Calculate the length of the entire tooltip markup block.
		int replaceLength = tooltipEndPos - searchPos + endToolTip.length();
		// Replace the markup with the plain data text.
		text.replace(searchPos, replaceLength, dataText);

		// If updating tooltip mappings, record the range and associated tip text.
		if (updateTooltips) {
			QPair<int, int> range(searchPos, searchPos + dataText.size());
			m_tooltips.insert(range, tipText);
		}

		// Advance searchPos past the inserted data.
		searchPos += dataText.size();
	}
}
