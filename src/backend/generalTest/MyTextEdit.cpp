/***************************************************************************
	File                 : MyTextEdit.cpp
	Project              : LabPlot
	Description          : Derived class of QTextEdit to add ToolTip Functionality
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)

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

#include "backend/generalTest/MyTextEdit.h"
#include "backend/lib/macros.h"

#include <QTextEdit>
#include <QHelpEvent>
#include <QToolTip>

MyTextEdit::MyTextEdit(QWidget* parent) : QTextEdit(parent) {
}

bool MyTextEdit::event(QEvent *e) {
	if (e->type() == QEvent::ToolTip || e->type() == QEvent::WhatsThis) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);

		QTextCursor origCursor = this->textCursor();
		int cursorPos = this->cursorForPosition(helpEvent->pos()).position();
		this->setTextCursor(origCursor);

		QMapIterator<std::pair<double, double>, QString> iter(m_tooltips);
		QString tooltip;
		while (iter.hasNext()) {
			iter.next();
			if (cursorPos >= iter.key().first && cursorPos <= iter.key().second) {
				tooltip = iter.value();
				break;
			}
		}

		if (!tooltip.isEmpty())
			QToolTip::showText(helpEvent->globalPos(), tooltip);
		else
			QToolTip::hideText();
		return true;
	}
	return 	inherited::event(e);
}

void MyTextEdit::setHtml(QString text) {
	inherited::setHtml(text);
	QString plainText = this->toPlainText();
	extractToolTips(plainText, true);
	extractToolTips(text, false);
	return inherited::setHtml(text);
}

void MyTextEdit::extractToolTips(QString &text, bool insert) {
	QString startToolTip = "[tooltip]";
	QString endToolTip = "[/tooltip]";
	QString startData = "[data]";
	QString endData = "[/data]";
	QString startTip = "[tip]";
	QString endTip = "[/tip]";

	int i_startToolTip = 0;
	int i_endToolTip = 0;
	int i_startData = 0;
	int i_endData = 0;
	int i_startTip = 0;
	int i_endTip = 0;

	QString tip;
	QString data;
	while ((i_startToolTip = text.indexOf(startToolTip, i_startToolTip)) != -1) {
		i_endToolTip = text.indexOf(endToolTip, i_startToolTip);

		if (i_endToolTip != -1) {
			i_startData = text.indexOf(startData, i_startToolTip);
			i_endData = text.indexOf(endData, i_startToolTip);

			if (i_startData != -1 && i_endData != -1 &&
					i_startData < i_endToolTip && i_endData < i_endToolTip)
				data = text.mid(i_startData + startData.size(), i_endData - i_startData - startData.size());
			else {
				data = "";
				i_endData = i_startToolTip + startToolTip.size();
			}

			i_startTip = text.indexOf(startTip, i_endData);
			i_endTip = text.indexOf(endTip, i_endData);

			if (i_startTip != -1 && i_endTip != -1 &&
					i_startTip < i_endToolTip && i_endTip < i_endToolTip)
				tip = text.mid(i_startTip + startTip.size(), i_endTip - i_startTip - startTip.size());
			else
				tip = "";
			text.replace(i_startToolTip, i_endToolTip - i_startToolTip + endToolTip.size(), data);

			if (insert) {
//				QDEBUG("data is " << data);
//				QDEBUG("low is " << i_startToolTip);
//				QDEBUG("high is " << i_startToolTip + data.size());
				m_tooltips.insert(std::pair<double, double>(i_startToolTip, i_startToolTip + data.size()), tip);
			}
			i_startToolTip += data.size();
		}
	}
}

