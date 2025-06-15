/*
	File             : TextPreview.h
	Project          : LabPlot
	Description      : widget for text preview of FileWidget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KPreviewWidgetBase>

class QPlainTextEdit;

class TextPreview : public KPreviewWidgetBase {
public:
	explicit TextPreview(QWidget *parent = nullptr);
	void showPreview(const QUrl &url) override;
	void clearPreview() override;

private:
    QPlainTextEdit* textEdit;
};
