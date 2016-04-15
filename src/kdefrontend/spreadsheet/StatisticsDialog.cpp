#include "StatisticsDialog.h"
#include "backend/core/column/Column.h"
#include "math.h"

#include <QTextEdit>
#include <QDesktopWidget>

StatisticsDialog::StatisticsDialog(const QString& title, QWidget* parent) :
	KDialog(parent) {

	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	setMainWidget(mainWidget);

	ui.tw_statisticsTabs->removeTab(0);
	ui.tw_statisticsTabs->removeTab(0);

	setWindowTitle(title);
	setButtons(KDialog::Ok);
	setButtonText(KDialog::Ok, i18n("&Ok"));

	m_htmlText = QString("<table border=0 width=100%>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=#D1D1D1><b><big>"
	                     + i18n("Location measures")+
	                     "</big><b></td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td width=70%><b>"
	                     + i18n("Minimum")+
	                     "<b></td>"
	                     "<td>%1</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Maximum")+
	                     "<b></td>"
	                     "<td>%2</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Arithmetic mean")+
	                     "<b></td>"
	                     "<td>%3</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Geometric mean")+
	                     "<b></td>"
	                     "<td>%4</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Harmonic mean")+
	                     "<b></td>"
	                     "<td>%5</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Contraharmonic mean")+
	                     "<b></td>"
	                     "<td>%6</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Median")+
	                     "<b></td>"
	                     "<td>%7</td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=#D1D1D1><b><big>"
	                     + i18n("Dispersion measures")+
	                     "</big></b></td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Variance")+
	                     "<b></td>"
	                     "<td>%8</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Standard deviation")+
	                     "<b></td>"
	                     "<td>%9</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Mean absolute deviation around mean")+
	                     "<b></td>"
	                     "<td>%10</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Mean absolute deviation around median")+
	                     "<b></td>"
	                     "<td>%11</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Median absolute deviation")+
	                     "<b></td>"
	                     "<td>%12</td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td colspan=2 align=center bgcolor=#D1D1D1><b><big>"
	                     + i18n("Shape measures")+
	                     "</big></b></td>"
	                     "</tr>"
	                     "<tr></tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Skewness")+
	                     "<b></td>"
	                     "<td>%13</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Kurtosis")+
	                     "<b></td>"
	                     "<td>%14</td>"
	                     "</tr>"
	                     "<tr>"
	                     "<td><b>"
	                     + i18n("Entropy")+
	                     "<b></td>"
	                     "<td>%15</td>"
	                     "</tr>"
	                     "</table>");

	move(QApplication::desktop()->screen()->rect().center() - rect().center());
    connect(ui.tw_statisticsTabs, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
	connect(this, SIGNAL(okClicked()), this, SLOT(close()));
}

void StatisticsDialog::setColumns(const QList<Column*>& columns) {
	if (!columns.size())
		return;

	m_columns = columns;

	for (int i = 0; i < m_columns.size(); ++i) {
		QTextEdit* textEdit = new QTextEdit;
		textEdit->setReadOnly(true);
		ui.tw_statisticsTabs->addTab(textEdit, m_columns[i]->name());
	}
    currentTabChanged(0);
}

const QString StatisticsDialog::isNanValue(const double value) {
	return (isnan(value) ? i18n("The value couldn't be calculated.") : QString::number(value,'g', 10));
}

QSize StatisticsDialog::sizeHint() const {
	return QSize(490, 520);
}

void StatisticsDialog::currentTabChanged(int index) {

	if(!m_columns[index]->statisticsAvailable()) {
		WAIT_CURSOR;
		m_columns[index]->calculateStatistics();
		RESET_CURSOR;
	}
	QTextEdit* textEdit = static_cast<QTextEdit*>(ui.tw_statisticsTabs->currentWidget());
	textEdit->setHtml(m_htmlText.arg(isNanValue(m_columns[index]->statistics().minimum)).
	                  arg(isNanValue(m_columns[index]->statistics().maximum)).
	                  arg(isNanValue(m_columns[index]->statistics().arithmeticMean)).
	                  arg(isNanValue(m_columns[index]->statistics().geometricMean)).
	                  arg(isNanValue(m_columns[index]->statistics().harmonicMean)).
	                  arg(isNanValue(m_columns[index]->statistics().contraharmonicMean)).
	                  arg(isNanValue(m_columns[index]->statistics().median)).
	                  arg(isNanValue(m_columns[index]->statistics().variance)).
	                  arg(isNanValue(m_columns[index]->statistics().standardDeviation)).
	                  arg(isNanValue(m_columns[index]->statistics().meanDeviation)).
	                  arg(isNanValue(m_columns[index]->statistics().meanDeviationAroundMedian)).
	                  arg(isNanValue(m_columns[index]->statistics().medianDeviation)).
	                  arg(isNanValue(m_columns[index]->statistics().skewness)).
	                  arg(isNanValue(m_columns[index]->statistics().kurtosis)).
	                  arg(isNanValue(m_columns[index]->statistics().entropy)));
}
