#pragma once

#include <qwidget.h>

QT_BEGIN_NAMESPACE
class QComboBox;
class QStackedWidget;
class QTableView;
QT_END_NAMESPACE

class CBotTab : public QWidget
{
	Q_OBJECT

	enum
	{
		PAGE_PERCENT = 0,
		PAGE_X
	};

public:
	explicit CBotTab(__in QWidget *pParent = 0);
	~CBotTab();

private slots:
	void comboHPChanged(int index);
	void comboSPChanged(int index);
	void OnHPHelperClicked(bool);
	void OnSPHelperClicked(bool);
	void showTableHPMenu(QPoint pos);

private:
	QStackedWidget *pPagesHP, *pPagesSP;
	QComboBox *pComboHP, *pComboSP;
	QTableView *pTableHP, *pTableSP;
};

class PercentPage : public QWidget
{
public:
	PercentPage(__in QWidget *parent = 0);
	~PercentPage();
};

class XPage : public QWidget
{
public:
	XPage(__in QWidget *parent = 0);
	~XPage();
};