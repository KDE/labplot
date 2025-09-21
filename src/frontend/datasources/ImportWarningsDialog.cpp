#include "ImportWarningsDialog.h"
#include "ui_importwarningsdialog.h"

ImportWarningsDialog::ImportWarningsDialog(const QStringList& errors, QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::ImportWarningsDialog) {
	ui->setupUi(this);
	ui->lwWarnings->addItems(errors);
	setAttribute(Qt::WA_DeleteOnClose);

	connect(ui->pbOk, &QPushButton::clicked, this, &QDialog::accept);
}

ImportWarningsDialog::~ImportWarningsDialog() {
	delete ui;
}
