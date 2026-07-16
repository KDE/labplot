#ifndef WARNINGSTYLE_H
#define WARNINGSTYLE_H

// "red" warning color in formula inputs, etc.
#define SET_WARNING_STYLE(elem)                                                                                                                                \
	{                                                                                                                                                          \
		QPalette p;                                                                                                                                            \
		if (qGray(p.color(QPalette::Base).rgb()) > 160) /* light */                                                                                            \
			elem->setStyleSheet(QLatin1String("background: rgb(255, 200, 200);"));                                                                             \
		else /* dark */                                                                                                                                        \
			elem->setStyleSheet(QLatin1String("background: rgb(128, 0, 0);"));                                                                                 \
	}

#define SET_WARNING_PALETTE                                                                                                                                    \
	{                                                                                                                                                          \
		auto p = palette();                                                                                                                                    \
		if (qGray(p.color(QPalette::Base).rgb()) > 160) /* light */                                                                                            \
			p.setColor(QPalette::Text, QColor(255, 200, 200));                                                                                                 \
		else /* dark */                                                                                                                                        \
			p.setColor(QPalette::Text, QColor(128, 0, 0));                                                                                                     \
		setPalette(p);                                                                                                                                         \
	}

#define SET_WARNING_BACKGROUND(elem)                                                                                                                           \
	{                                                                                                                                                          \
		auto p = palette();                                                                                                                                    \
		if (qGray(p.color(QPalette::Base).rgb()) > 160) /* light */                                                                                            \
			elem->setBackground(QColor(255, 200, 200));                                                                                                        \
		else /* dark */                                                                                                                                        \
			elem->setBackground(QColor(128, 0, 0));                                                                                                            \
	}

#endif // WARNINGSTYLE_H
