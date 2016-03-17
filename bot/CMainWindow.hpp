#pragma once

#include <qmainwindow.h>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QTabWidget;
QT_END_NAMESPACE

class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit CMainWindow(__in QWidget *pParent = 0);
	~CMainWindow();

	private slots:
	void OnOpen();
	void OnAbout();

private:
	QTextEdit *pTextEdit;
	QTabWidget *pTabWidget;
};