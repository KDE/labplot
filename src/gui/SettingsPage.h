#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QtGui/QWidget>

/**
 * @brief Base class for the settings pages of the Dolphin settings dialog.
 *
 */
class SettingsPage : public QWidget{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent);
    virtual ~SettingsPage();

    virtual void applySettings() = 0;
    virtual void restoreDefaults() = 0;
};

#endif
