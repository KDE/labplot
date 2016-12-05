#include "SlidingPanel.h"

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QApplication>
#include <KLocale>

SlidingPanel::SlidingPanel(QWidget *parent, const QString &worksheetName) : QFrame(parent) {
        setAttribute(Qt::WA_DeleteOnClose);

    m_worksheetName = new QLabel(worksheetName);
    QFont nameFont;
    nameFont.setPointSize(20);
    nameFont.setBold(true);
    m_worksheetName->setFont(nameFont);

    m_quitPresentingMode = new QPushButton(i18n("Quit presentation"));

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(m_worksheetName);
    QSpacerItem* spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlayout->addItem(spacer);
    hlayout->addWidget(m_quitPresentingMode);
    setLayout(hlayout);

    QPalette pal(palette());
    pal.setColor(QPalette::Background, Qt::gray);
    setAutoFillBackground(true);
    setPalette(pal);

    move(0, 0);
    raise();
    show();
}

SlidingPanel::~SlidingPanel() {
        delete m_worksheetName;
        delete m_quitPresentingMode;
}

void SlidingPanel::movePanel(qreal value) {
        move(0, -height() + static_cast<int>(value * height()) );
    raise();
}

QSize SlidingPanel::sizeHint() const {
    QSize sh;
    QDesktopWidget* const dw = QApplication::desktop();
    const int primaryScreenIdx = dw->primaryScreen();
    const QRect& screenSize = dw->availableGeometry(primaryScreenIdx);
    sh.setWidth(screenSize.width());

    //for the height use 1.5 times the height of the font used in the label (20 points) in pixels
    QFont font;
    font.setPointSize(20);
    QFontMetrics fm(font);
    sh.setHeight(1.5*fm.ascent());

    return sh;
}

bool SlidingPanel::shouldHide() {
        const QRect& frameRect = this->rect();
    return !(frameRect.contains(QCursor::pos()));
}
