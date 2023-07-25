#include "ImportErrorDialog.h"
#include "ui_ImportErrorDialog.h"

ImportErrorDialog::ImportErrorDialog(const QStringList& errors, QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::ImportErrorDialog) {
	ui->setupUi(this);
	ui->lwErrors->addItems(errors);

	connect(ui->pbOk, &QPushButton::clicked, this, &QDialog::accept);
}

ImportErrorDialog::~ImportErrorDialog() {
	delete ui;
}
