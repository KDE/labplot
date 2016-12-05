#ifndef SLIDINGPANEL_H
#define SLIDINGPANEL_H

#include <QObject>
#include <QFrame>

class QLabel;
class QPushButton;

class SlidingPanel : public QFrame {
        Q_OBJECT
public:
        explicit SlidingPanel(QWidget* parent, const QString& worksheetName);
        ~SlidingPanel();

        QLabel* m_worksheetName;
    QPushButton* m_quitPresentingMode;
    virtual QSize sizeHint() const;
    bool shouldHide();

public slots:
    void movePanel(qreal);
};

#endif // SLIDINGPANEL_H
