/*
	File                 : ExportSpreadsheetDialog.h
	Project              : LabPlot
	Description          : export spreadsheet dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXPORTSPREADSHEETDIALOG_H
#define EXPORTSPREADSHEETDIALOG_H

#include <QDialog>
#include <QLocale>

namespace Ui {
class ExportSpreadsheetWidget;
}

class QPushButton;
class QAbstractButton;

class ExportSpreadsheetDialog : public QDialog {
	Q_OBJECT

public:
	explicit ExportSpreadsheetDialog(QWidget*);
	~ExportSpreadsheetDialog() override;

	QString path() const;
	void setProjectFileName(const QString&);
	void setFileName(const QString&);
	void setMatrixMode(bool);
	void setExportSelection(bool);
	bool exportHeader() const;
	bool exportLatexHeader() const;
	bool gridLines() const;
	bool captions() const;
	bool skipEmptyRows() const;
	bool exportSelection() const;
	bool entireSpreadheet() const;
	bool matrixVerticalHeader() const;
	bool matrixHorizontalHeader() const;
	QString separator() const;
	QLocale::Language numberFormat() const;
	int exportToFits() const;
	bool commentsAsUnitsFits() const;
	void setExportTo(const QStringList& to);
	void setExportToImage(bool possible);
	void onCompressionToggled(bool checked);

	std::pair<int, int> getMcapSettings();

	enum class Format { ASCII, LaTeX, XLSX, SQLite, MCAP, FITS };

	Format format() const;

private:
	Ui::ExportSpreadsheetWidget* ui;
	bool m_showOptions{true};
	bool m_matrixMode{false};
	QString m_projectPath;

	QPushButton* m_showOptionsButton;
	QPushButton* m_okButton;
	QPushButton* m_cancelButton;

private Q_SLOTS:
	void slotButtonClicked(QAbstractButton*);
	void okClicked();
	void toggleOptions();
	void selectFile();
	void formatChanged(int);
	void fileNameChanged(const QString&);
	void fitsExportToChanged(int);
};

#endif
