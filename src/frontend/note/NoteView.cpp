/*
	File                 : NotesView.cpp
	Project              : LabPlot
	Description          : Notes View for taking notes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2016 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NoteView.h"
#include "backend/note/Note.h"

#include <QApplication>
#include <QButtonGroup>
#include <QEvent>
#include <QPrinter>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <KLocalizedString>

#ifdef HAVE_DISCOUNT
extern "C" {
#include <mkdio.h>
}
#endif

NoteView::NoteView(Note* note)
	: m_note(note) {
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	// Setup toolbar with mode toggle buttons
	setupToolbar();
	layout->addWidget(m_toolbar);

	// Create stacked widget for edit/preview
	m_stack = new QStackedWidget(this);

	// Edit mode widget (plain text editor)
	m_textEdit = new QTextEdit(m_stack);
	m_textEdit->setPlainText(m_note->text());
	m_textEdit->setBackgroundRole(QPalette::Base);
	m_textEdit->setFocus();
	m_textEdit->installEventFilter(this);

	QPalette palette = m_textEdit->palette();
	palette.setColor(QPalette::Base, m_note->backgroundColor());
	palette.setColor(QPalette::Text, m_note->textColor());
	m_textEdit->setPalette(palette);
	m_textEdit->setFont(m_note->textFont());

	// Preview mode widget (rendered output)
	m_preview = new QTextBrowser(m_stack);
	m_preview->setOpenExternalLinks(true);
	m_preview->setReadOnly(true);
	m_preview->viewport()->setAutoFillBackground(true);

	m_stack->addWidget(m_textEdit); // index 0 = edit
	m_stack->addWidget(m_preview); // index 1 = preview

	layout->addWidget(m_stack);

	// Initial mode
	if (m_note->mode() == Note::Mode::Markdown) {
		m_previewButton->setChecked(true);
		showPreviewMode();
	} else {
		m_editButton->setChecked(true);
		showEditMode();
	}

	// Connections
	connect(m_textEdit, &QTextEdit::textChanged, this, &NoteView::textEditChanged);

	connect(m_note, &Note::textChanged, this, &NoteView::noteTextChanged);
	connect(m_note, &Note::modeChanged, this, &NoteView::noteModeChanged);
	connect(m_note, &Note::backgroundColorChanged, this, &NoteView::noteBackgroundColorChanged);
	connect(m_note, &Note::textColorChanged, this, &NoteView::noteTextColorChanged);
	connect(m_note, &Note::textFontChanged, this, &NoteView::noteTextFontChanged);
}

void NoteView::setupToolbar() {
	m_toolbar = new QToolBar(this);
	m_toolbar->setIconSize(QSize(16, 16));
	m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	// Create mode toggle buttons
	m_editButton = new QToolButton(m_toolbar);
	m_editButton->setText(i18n("Edit"));
	m_editButton->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
	m_editButton->setCheckable(true);
	m_editButton->setChecked(true);
	m_editButton->setToolTip(i18n("Edit markdown source"));

	m_previewButton = new QToolButton(m_toolbar);
	m_previewButton->setText(i18n("Preview"));
	m_previewButton->setIcon(QIcon::fromTheme(QStringLiteral("document-preview")));
	m_previewButton->setCheckable(true);
	m_previewButton->setToolTip(i18n("Preview rendered output"));

	// Make buttons mutually exclusive
	auto* buttonGroup = new QButtonGroup(this);
	buttonGroup->addButton(m_editButton);
	buttonGroup->addButton(m_previewButton);

	m_toolbar->addWidget(m_editButton);
	m_toolbar->addWidget(m_previewButton);

	// Only show toolbar when in Markdown mode
	m_toolbar->setVisible(m_note->mode() == Note::Mode::Markdown);

	// Connect mode switching
	connect(m_editButton, &QToolButton::clicked, this, &NoteView::showEditMode);
	connect(m_previewButton, &QToolButton::clicked, this, &NoteView::showPreviewMode);
}

void NoteView::showEditMode() {
	m_stack->setCurrentIndex(0);
	m_viewMode = false;
	m_textEdit->setFocus();
}

void NoteView::showPreviewMode() {
	if (m_note->mode() == Note::Mode::Markdown) {
		updatePreview();
	}
	m_stack->setCurrentIndex(1);
	m_viewMode = true;
}

void NoteView::updatePreview() {
	QString html = renderMarkdown(m_textEdit->toPlainText());
	QFont font = m_note->textFont();
	int fontSize = font.pointSize();
	if (fontSize <= 0)
		fontSize = QApplication::font().pointSize();
	if (fontSize <= 0)
		fontSize = 10;

	// escape single quotes in the font family name for use in CSS
	QString fontFamily = font.family();
	fontFamily.replace(QLatin1Char('\''), QStringLiteral("\\'"));

	// apply styling to the QTextBrowser itself (background, text color, font)
	auto palette = m_preview->palette();
	palette.setColor(QPalette::Base, m_note->backgroundColor());
	palette.setColor(QPalette::Text, m_note->textColor());
	palette.setColor(QPalette::Window, m_note->backgroundColor());
	palette.setColor(QPalette::WindowText, m_note->textColor());
	m_preview->setPalette(palette);

	auto viewportPalette = m_preview->viewport()->palette();
	viewportPalette.setColor(QPalette::Base, m_note->backgroundColor());
	viewportPalette.setColor(QPalette::Text, m_note->textColor());
	viewportPalette.setColor(QPalette::Window, m_note->backgroundColor());
	viewportPalette.setColor(QPalette::WindowText, m_note->textColor());
	m_preview->viewport()->setPalette(viewportPalette);
	m_preview->setFont(font);

	// apply styling to the HTML content
	QString wrappedHtml = QStringLiteral(
		"<html><head><style>"
		"html, body { "
		"  margin: 0; "
		"  padding: 0; "
		"  background-color: %1; "
		"  color: %2; "
		"  font-family: '%3'; "
		"  font-size: %4pt; "
		"}"
		"</style></head><body>%5</body></html>")
		.arg(m_note->backgroundColor().name())
		.arg(m_note->textColor().name())
		.arg(fontFamily)
		.arg(fontSize)
		.arg(html);

	// update the preview's default style sheet and set the new HTML content
	QString style = QStringLiteral(
						"body { "
						"  background-color: %1; "
						"  color: %2; "
						"  font-family: '%3'; "
						"  font-size: %4pt; "
						"}")
						.arg(m_note->backgroundColor().name())
						.arg(m_note->textColor().name())
						.arg(fontFamily)
						.arg(fontSize);

	m_preview->document()->setDefaultStyleSheet(style);
	m_preview->setHtml(wrappedHtml);
}

QString NoteView::renderMarkdown(const QString& markdown) {
#ifdef HAVE_DISCOUNT
	// Use Discount library (same as TextLabel)
	QByteArray mdData = markdown.toUtf8();

#ifdef HAVE_DISCOUNT3
	MMIOT* mdHandle = mkd_string(mdData.data(), mdData.size() + 1, nullptr);
	if (!mdHandle)
		return markdown;

	mkd_flag_t* v3flags = mkd_flags();
	mkd_set_flag_num(v3flags, MKD_FENCEDCODE);
	mkd_set_flag_num(v3flags, MKD_GITHUBTAGS);

	if (!mkd_compile(mdHandle, v3flags)) {
		mkd_cleanup(mdHandle);
		return markdown;
	}
#else
	MMIOT* mdHandle = mkd_string(mdData.data(), mdData.size() + 1, 0);
	if (!mdHandle)
		return markdown;

	unsigned int flags = MKD_FENCEDCODE | MKD_GITHUBTAGS;
	if (!mkd_compile(mdHandle, flags)) {
		mkd_cleanup(mdHandle);
		return markdown;
	}
#endif

	char* html;
	int size = mkd_document(mdHandle, &html);
	QString result = QString::fromUtf8(html, size);
	mkd_cleanup(mdHandle);
	return result;
#else
	// Fallback: return plain text with line breaks
	return QStringLiteral("<pre>%1</pre>").arg(markdown.toHtmlEscaped());
#endif
}

void NoteView::print(QPrinter* printer) const {
	if (m_viewMode && m_note->mode() == Note::Mode::Markdown)
		m_preview->print(printer);
	else
		m_textEdit->print(printer);
}

void NoteView::textEditChanged() {
	if (m_initializing)
		return;

	// Debounce text changes
	if (m_textChangedTimerId != -1)
		killTimer(m_textChangedTimerId);
	m_textChangedTimerId = startTimer(1000);
}

void NoteView::timerEvent(QTimerEvent* event) {
	if (event->timerId() == m_textChangedTimerId) {
		killTimer(m_textChangedTimerId);
		m_textChangedTimerId = -1;

		m_initializing = true;
		QString text = m_textEdit->toPlainText();
		m_note->setText(text);
		m_initializing = false;

		// Update preview if in preview mode
		if (m_viewMode && m_note->mode() == Note::Mode::Markdown)
			updatePreview();
	}
}

//*************************************************************
//************ SLOTs for changes triggered in Note ************
//*************************************************************
void NoteView::noteTextChanged(const QString& text) {
	if (m_initializing)
		return;

	auto cursor = m_textEdit->textCursor();
	int cursorAnchor = cursor.anchor();
	int cursorPos = cursor.position();

	m_initializing = true;
	m_textEdit->setPlainText(text);
	m_initializing = false;

	cursor = m_textEdit->textCursor();
	cursor.setPosition(cursorAnchor);
	cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, cursorPos - cursorAnchor);
	m_textEdit->setTextCursor(cursor);

	// Update preview if in preview mode
	if (m_viewMode && m_note->mode() == Note::Mode::Markdown)
		updatePreview();
}

void NoteView::noteBackgroundColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	m_initializing = true;
	QString red = QString::number(color.red());
	QString green = QString::number(color.green());
	QString blue = QString::number(color.blue());
	m_textEdit->setStyleSheet(QStringLiteral("QTextEdit{background-color: rgb(%1, %2, %3);}").arg(red, green, blue));
	m_initializing = false;

	// Update preview styling
	if (m_viewMode && m_note->mode() == Note::Mode::Markdown)
		updatePreview();
}

void NoteView::noteTextFontChanged(const QFont& font) {
	if (m_initializing)
		return;

	m_initializing = true;
	m_textEdit->setFont(font);
	m_initializing = false;

	// Update preview styling
	if (m_viewMode && m_note->mode() == Note::Mode::Markdown)
		updatePreview();
}

void NoteView::noteTextColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	m_initializing = true;
	m_textEdit->selectAll();
	m_textEdit->setTextColor(color);
	auto cursor = m_textEdit->textCursor();
	cursor.clearSelection();
	m_textEdit->setTextCursor(cursor);
	m_initializing = false;

	// Update preview styling
	if (m_viewMode && m_note->mode() == Note::Mode::Markdown)
		updatePreview();
}

void NoteView::noteModeChanged(Note::Mode mode) {
	// Show/hide toolbar based on mode
	m_toolbar->setVisible(mode == Note::Mode::Markdown);

	// Switch to edit mode when changing to PlainText
	if (mode == Note::Mode::PlainText) {
		m_editButton->setChecked(true);
		showEditMode();
	}
}

//*************************************************************
//************ Project Explorer Sync **************************
//*************************************************************
bool NoteView::eventFilter(QObject* obj, QEvent* event) {
	if (obj == m_textEdit && event->type() == QEvent::FocusIn)
		m_note->setSelectedInView(true);

	return QWidget::eventFilter(obj, event);
}
