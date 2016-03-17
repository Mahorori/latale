#pragma once

#include <qwidget.h>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class CHackTab : public QWidget
{
	Q_OBJECT

public:
	explicit CHackTab(__in QWidget *pParent = 0);
	~CHackTab();

private slots:
	void OnFMLCheckBoxStateChanged(int state);
};