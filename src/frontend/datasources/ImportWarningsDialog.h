#ifndef IMPORTWARNINGSDIALOG_H
#define IMPORTWARNINGSDIALOG_H

#include <QDialog>

namespace Ui {
class ImportWarningsDialog;
}

class ImportWarningsDialog : public QDialog {
	Q_OBJECT

public:
	explicit ImportWarningsDialog(const QStringList& errors, QWidget* parent = nullptr);
	~ImportWarningsDialog();

private:
	Ui::ImportWarningsDialog* ui;
};

#endif // IMPORTWARNINGSDIALOG_H
