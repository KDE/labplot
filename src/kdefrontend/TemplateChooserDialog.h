#ifndef TEMPLATECHOOSERDIALOG_H
#define TEMPLATECHOOSERDIALOG_H

#include <QDialog>

class Project;
class Worksheet;

namespace Ui {
class TemplateChooserDialog;
}

class TemplateChooserDialog : public QDialog
{
	Q_OBJECT

public:
	explicit TemplateChooserDialog(QWidget *parent = nullptr);
	void updateErrorMessage(const QString& message);
	~TemplateChooserDialog();

	QString templatePath() const;
	void showPreview();
	static const QString format;

private:
	void chooseTemplate();

private:
	Ui::TemplateChooserDialog *ui;
	Project* m_project; // TODO: use smart pointer!
	Worksheet* m_worksheet;
	QWidget* m_worksheetView;
};

#endif // TEMPLATECHOOSERDIALOG_H
