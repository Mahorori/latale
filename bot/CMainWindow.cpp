#include "stdafx.h"

#undef QT_NO_TRANSLATION

#include <time.h>
#include "CMainWindow.hpp"
#include "CLoginTab.hpp"
#include "CBotTab.hpp"
#include "CHackTab.hpp"
#include "COptionRefreshTab.hpp"

CMainWindow::CMainWindow(__in QWidget *pParent) : QMainWindow(pParent)
{
	setObjectName("Object");
	setWindowTitle(tr("Latale Bot by KOTOKIRA"));

	// window サイズ
	// setMaximumSize(1024, 768);
	setMinimumSize(512, 384);

	// メニュー
	QMenu *fileMenu = menuBar()->addMenu(tr("&File(&F)"));
	QAction *pAction = fileMenu->addAction(tr("&Open(&O)"));
	connect(pAction, SIGNAL(triggered()), this, SLOT(OnOpen()));

	QMenu *helpMenu = menuBar()->addMenu(tr("&Help(&H)"));
	QAction *aboutAction = helpMenu->addAction(tr("&About(&A)"));
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(OnAbout()));

	pTabWidget = new QTabWidget;
	pTabWidget->setTabPosition(QTabWidget::TabPosition::West); // 左側
	pTabWidget->addTab(new CLoginTab(), tr("Login"));
	pTabWidget->addTab(new CBotTab(), tr("Bot"));
	pTabWidget->addTab(new CHackTab(), tr("Hack"));
	pTabWidget->addTab(new COptionRefreshTab(), tr("OptRst"));

	setCentralWidget(pTabWidget);
	show();
}

CMainWindow::~CMainWindow()
{
}

void CMainWindow::OnOpen()
{

}

void CMainWindow::OnAbout()
{

}