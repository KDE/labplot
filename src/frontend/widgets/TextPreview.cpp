/*
	File             : TextPreview.cpp
	Project          : LabPlot
	Description      : widget for text preview of FileWidget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "TextPreview.h"

#include <QFile>
#include <QPlainTextEdit>
#include <QVBoxLayout>

TextPreview::TextPreview(QWidget *parent)
		: KPreviewWidgetBase(parent) {
        textEdit = new QPlainTextEdit(this);
        textEdit->setReadOnly(true);
        textEdit->setWordWrapMode(QTextOption::WrapAnywhere);

        auto* layout = new QVBoxLayout(this);
        layout->addWidget(textEdit);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
}

void TextPreview::showPreview(const QUrl &url) {
        textEdit->clear();
        if (!url.isLocalFile()) return;

        QFile file(url.toLocalFile());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString text = in.read(1000);  // Limit size for preview
            textEdit->setPlainText(text);
        }
}

void TextPreview::clearPreview() {
        textEdit->clear();
}
