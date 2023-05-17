#ifndef IMPORTERRORDIALOG_H
#define IMPORTERRORDIALOG_H

#include <QDialog>

namespace Ui {
class ImportErrorDialog;
}

class ImportErrorDialog : public QDialog {
	Q_OBJECT

public:
	explicit ImportErrorDialog(const QStringList& errors, QWidget* parent = nullptr);
	~ImportErrorDialog();

private:
	Ui::ImportErrorDialog* ui;
};

#endif // IMPORTERRORDIALOG_H
