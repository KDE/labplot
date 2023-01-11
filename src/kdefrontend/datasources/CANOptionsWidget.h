#ifndef CANOPTIONSWIDGET_H
#define CANOPTIONSWIDGET_H

#include <QWidget>

namespace Ui {
class CANOptionsWidget;
}

class VectorBLFFilter;

class CANOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit CANOptionsWidget(QWidget* parent = nullptr);
	~CANOptionsWidget();

	void applyFilterSettings(VectorBLFFilter* filter) const;

	void saveSettings() const;
	void loadSettings() const;

private:
	Ui::CANOptionsWidget* ui;
};

#endif // CANOPTIONSWIDGET_H
