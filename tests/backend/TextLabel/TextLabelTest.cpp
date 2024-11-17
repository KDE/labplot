/*
	File                 : TextLabelTest.cpp
	Project              : LabPlot
	Description          : Tests for TextLabel
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "TextLabelTest.h"
#include "backend/core/Project.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/TextLabelPrivate.h"
#include "frontend/widgets/LabelWidget.h"

struct TextProperties {
	QColor fontColor;
	QColor backgroundColor;
	QString text;
	QString plainText;
	QFont font;
	int weigth;
	bool italic;
	bool underline;
};

QColor getColorFromHTMLText(const QString& text, const QString& colortype) {
	const QString htmlColorPattern(QStringLiteral("(#[0-9A-Fa-f]{6})|(transparent)"));
	QRegularExpression fontColorPattern(colortype + QStringLiteral(":") + htmlColorPattern);
	QRegularExpressionMatch matchColor = fontColorPattern.match(text);
	// QVERIFY(matchColor.hasMatch());
	// QCOMPARE(matchColor.capturedTexts().count(), 1);
	const auto& splitted = matchColor.capturedTexts().at(0).split(colortype + QStringLiteral(":"));
	QColor c;
	if (splitted.length() > 1) {
		QString color = splitted.at(1);
		int r = color.mid(1, 2).toInt(nullptr, 16);
		int g = color.mid(3, 2).toInt(nullptr, 16);
		int b = color.mid(5, 2).toInt(nullptr, 16);

		c = QColor(r, g, b);
	} else {
		c = QColor(Qt::transparent);
	}
	return c;
}

#define STORETEXTPROPERTIES(label, propertyVariable)                                                                                                           \
	TextProperties propertyVariable;                                                                                                                           \
	{                                                                                                                                                          \
		propertyVariable.text = label->text().text;                                                                                                            \
		QTextEdit te(label->text().text);                                                                                                                      \
		propertyVariable.font = te.font();                                                                                                                     \
		/* For some reason the two below don't work */                                                                                                         \
		/* propertyVariable.fontColor = te.textColor(); */                                                                                                     \
		/* propertyVariable.backgroundColor = te.textBackgroundColor(); */                                                                                     \
		propertyVariable.fontColor = getColorFromHTMLText(label->text().text, QStringLiteral("color"));                                                        \
		propertyVariable.backgroundColor = getColorFromHTMLText(label->text().text, QStringLiteral("background-color"));                                       \
		propertyVariable.italic = te.fontItalic();                                                                                                             \
		propertyVariable.underline = te.fontUnderline();                                                                                                       \
		propertyVariable.plainText = te.toPlainText();                                                                                                         \
		propertyVariable.weigth = te.fontWeight();                                                                                                             \
	}

#define COMPARETEXTPROPERTIES(actual, expected)                                                                                                                \
	QCOMPARE(actual.backgroundColor, expected.backgroundColor);                                                                                                \
	QCOMPARE(actual.fontColor, expected.fontColor);                                                                                                            \
	QCOMPARE(actual.font, expected.font);                                                                                                                      \
	QCOMPARE(actual.italic, expected.italic);                                                                                                                  \
	QCOMPARE(actual.plainText, expected.plainText);                                                                                                            \
	QCOMPARE(actual.underline, expected.underline);                                                                                                            \
	QCOMPARE(actual.weigth, expected.weigth);                                                                                                                  \
	/* QCOMPARE(actual.text, expected.text); Cannot be used, because then also in the expected html text the color must be replaced*/

#define COMPARETEXTPROPERTIESLABEL(label, expected) {STORETEXTPROPERTIES(label, propertyVariable) COMPARETEXTPROPERTIES(propertyVariable, expected)}

#define VERIFYLABELCOLORS(label, fontcolor_, backgroundColor_)                                                                                                 \
	{                                                                                                                                                          \
		QCOMPARE(getColorFromHTMLText(label->text().text, QStringLiteral("color")), fontcolor_);                                                               \
		QCOMPARE(getColorFromHTMLText(label->text().text, QStringLiteral("background-color")), backgroundColor_);                                              \
	}

void TextLabelTest::addPlot() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* l = new TextLabel(QStringLiteral("Label"));
	QVERIFY(l != nullptr);
	l->setText(QStringLiteral("TextLabelText"));
	ws->addChild(l);

	QCOMPARE(l->text().mode, TextLabel::Mode::Text);
	VERIFYLABELCOLORS(l, Qt::black, Qt::transparent);
	QCOMPARE(l->fontColor(), Qt::black);
	QCOMPARE(l->backgroundColor(), QColor(1, 1, 1, 0));

	// add title?

	// add axes?
	// check axis label
}

/*!
 * \brief TextLabelTest::multiLabelEdit
 * Test if changing the background color of one label
 * only the background color will be changed for the second
 * label, but no other properties
 */
void TextLabelTest::multiLabelEditColorChange() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* l1 = new TextLabel(QStringLiteral("Label"));
	QVERIFY(l1 != nullptr);
	l1->setText(QStringLiteral("Text1"));
	ws->addChild(l1);

	VERIFYLABELCOLORS(l1, Qt::black, Qt::transparent);

	auto* l2 = new TextLabel(QStringLiteral("Label"));
	QVERIFY(l2 != nullptr);
	l2->setText(QStringLiteral("Text2"));
	ws->addChild(l2);

	STORETEXTPROPERTIES(l1, l1InitProperties);
	STORETEXTPROPERTIES(l2, l2InitProperties);

	LabelWidget w(nullptr);
	w.setLabels({l1}); // change only one textlabel
	w.fontColorChanged(Qt::red);
	w.backgroundColorChanged(Qt::blue);

	TextProperties l1p = l1InitProperties;
	l1p.fontColor = Qt::red;
	l1p.backgroundColor = Qt::blue;
	COMPARETEXTPROPERTIESLABEL(l1, l1p);
	COMPARETEXTPROPERTIESLABEL(l2, l2InitProperties);

	w.setLabels({l1, l2});
	w.fontColorChanged(Qt::cyan);
	w.backgroundColorChanged(Qt::green);

	l1p = l1InitProperties;
	l1p.fontColor = Qt::cyan;
	l1p.backgroundColor = Qt::green;
	COMPARETEXTPROPERTIESLABEL(l1, l1p);
	TextProperties l2p = l2InitProperties;
	l2p.fontColor = Qt::cyan;
	l2p.backgroundColor = Qt::green;
	COMPARETEXTPROPERTIESLABEL(l2, l2p);
}

/*!
 * \brief TextLabelTest::multiLabelEditTextChange
 * Changing the text of two labels changes the text content and more text related
 * properties
 */
void TextLabelTest::multiLabelEditTextChange() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	ws->addChild(p);

	auto* l1 = new TextLabel(QStringLiteral("Label"));
	QVERIFY(l1 != nullptr);
	l1->setText(QStringLiteral("Text1"));
	ws->addChild(l1);

	auto* l2 = new TextLabel(QStringLiteral("Label"));
	QVERIFY(l2 != nullptr);
	l2->setText(QStringLiteral("Text2"));
	ws->addChild(l2);

	LabelWidget w(nullptr);
	w.setLabels({l1}); // change only one textlabel
	w.fontColorChanged(Qt::red);
	w.backgroundColorChanged(Qt::blue);
	w.fontSuperScriptChanged(true);
	w.fontStrikeOutChanged(true);
	w.fontUnderlineChanged(true);
	w.fontItalicChanged(true);
	w.fontBoldChanged(true);
	w.fontChanged(QFont(QStringLiteral("AkrutiTml2")));

	STORETEXTPROPERTIES(l1, l1InitProperties);

	w.setLabels({l1, l2});
	w.ui.teLabel->setText(QStringLiteral("New text"));

	TextProperties l1p = l1InitProperties;
	l1p.plainText = QStringLiteral("New text");
	COMPARETEXTPROPERTIESLABEL(l1, l1p);
	COMPARETEXTPROPERTIESLABEL(l2, l1p); // textlabel2 received all properties of the first label, because the text content changes
}

void TextLabelTest::multiLabelEditColorChangeSelection() {
	Project project;
	auto* ws = new Worksheet(QStringLiteral("worksheet"));
	QVERIFY(ws != nullptr);
	project.addChild(ws);

	auto* p = new CartesianPlot(QStringLiteral("plot"));
	QVERIFY(p != nullptr);
	ws->addChild(p);

	LabelWidget w(nullptr);

	auto* l1 = new TextLabel(QStringLiteral("Label"));
	QVERIFY(l1 != nullptr);
	ws->addChild(l1);
	l1->setText(QStringLiteral("This is the text of label 1"));

	VERIFYLABELCOLORS(l1, Qt::black, Qt::transparent);

	auto* l2 = new TextLabel(QStringLiteral("Label"));
	QVERIFY(l2 != nullptr);
	ws->addChild(l2);
	w.setLabels({l2}); // This setlabels is important, otherwise a specific scenario does not occur
	l2->setText(QStringLiteral("Text label 2"));

	STORETEXTPROPERTIES(l1, l1InitProperties);
	STORETEXTPROPERTIES(l2, l2InitProperties);

	w.setLabels({l1, l2});
	auto cursor = w.ui.teLabel->textCursor();
	// marks character 2 to 4
	cursor.setPosition(2);
	cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
	w.ui.teLabel->setTextCursor(cursor);
	w.fontColorChanged(Qt::cyan);
	w.backgroundColorChanged(Qt::green);
	w.fontBoldChanged(true);
	w.fontItalicChanged(true);

	{
		QTextEdit te(l1->text().text);
		QTextCursor cTest = te.textCursor();
		cTest.setPosition(2);
		cTest.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
		te.setTextCursor(cTest);
		auto ccf = te.currentCharFormat();
		QCOMPARE(ccf.foreground().color(), Qt::cyan);
		QCOMPARE(ccf.background().color(), Qt::green);
		QCOMPARE(te.fontItalic(), true);
		QCOMPARE(te.fontWeight(), QFont::Bold);
		QCOMPARE(te.toPlainText(), QStringLiteral("This is the text of label 1"));
	}

	{
		QTextEdit te(l2->text().text);
		QTextCursor cTest = te.textCursor();
		cTest.setPosition(2);
		cTest.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
		te.setTextCursor(cTest);
		auto ccf = te.currentCharFormat();
		QCOMPARE(ccf.foreground().color(), Qt::cyan);
		QCOMPARE(ccf.background().color(), Qt::green);
		QCOMPARE(te.fontItalic(), true);
		QCOMPARE(te.fontWeight(), QFont::Bold);
		QCOMPARE(te.toPlainText(), QStringLiteral("Text label 2"));
	}

	w.fontBoldChanged(false);
	w.fontItalicChanged(false);

	{
		QTextEdit te(l1->text().text);
		QTextCursor cTest = te.textCursor();
		cTest.setPosition(2);
		cTest.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
		te.setTextCursor(cTest);
		auto ccf = te.currentCharFormat();
		QCOMPARE(ccf.foreground().color(), Qt::cyan);
		QCOMPARE(ccf.background().color(), Qt::green);
		QCOMPARE(te.fontItalic(), false);
		QCOMPARE(te.fontWeight(), QFont::Normal);
		QCOMPARE(te.toPlainText(), QStringLiteral("This is the text of label 1"));
	}

	{
		QTextEdit te(l2->text().text);
		QTextCursor cTest = te.textCursor();
		cTest.setPosition(2);
		cTest.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
		te.setTextCursor(cTest);
		auto ccf = te.currentCharFormat();
		QCOMPARE(ccf.foreground().color(), Qt::cyan);
		QCOMPARE(ccf.background().color(), Qt::green);
		QCOMPARE(te.fontItalic(), false);
		QCOMPARE(te.fontWeight(), QFont::Normal);
		QCOMPARE(te.toPlainText(), QStringLiteral("Text label 2"));
	}
}

QTEST_MAIN(TextLabelTest)
