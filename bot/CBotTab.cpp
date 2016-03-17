#include "stdafx.h"

#include "CBotTab.hpp"
#include "CItemModel.hpp"
#include "CInventoryWindow.hpp"

CBotTab::CBotTab(__in QWidget *pParent) : QWidget(pParent)
{
	// hppot
	QGroupBox *groupHP = new QGroupBox(tr("HP"));
	pComboHP = new QComboBox;
	pComboHP->addItem(tr("%"), QVariant(PAGE_PERCENT));
	pComboHP->addItem(tr("X"), QVariant(PAGE_X));

	pPagesHP = new QStackedWidget;
	pPagesHP->addWidget(new PercentPage);
	pPagesHP->addWidget(new XPage);
	connect(pComboHP, SIGNAL(currentIndexChanged(int)), this, SLOT(comboHPChanged(int)));

	QPushButton *buttonHPHelper = new QPushButton(tr("Helper"));
	connect(buttonHPHelper, SIGNAL(clicked(bool)), this, SLOT(OnHPHelperClicked(bool)));
	pTableHP = new QTableView;
	pTableHP->setSelectionBehavior(QAbstractItemView::SelectRows);
	pTableHP->setModel(new CItemModel);
	pTableHP->horizontalHeader()->setStretchLastSection(true);
	pTableHP->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(pTableHP, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showTableHPMenu(QPoint)));

	// grouphppot layout
	QGridLayout *groupHPLayout = new QGridLayout;
	groupHPLayout->addWidget(pComboHP, 0, 0);
	groupHPLayout->addWidget(pPagesHP, 1, 0);
	groupHPLayout->addWidget(buttonHPHelper, 2, 0);
	groupHPLayout->addWidget(pTableHP, 0, 1, 3, 2);
	groupHPLayout->setColumnStretch(0, 1);
	groupHPLayout->setColumnStretch(1, 2);
	groupHP->setLayout(groupHPLayout);

	// sppot
	QGroupBox *groupSP = new QGroupBox(tr("SP"));
	pComboSP = new QComboBox;
	pComboSP->addItem(tr("%"), QVariant(PAGE_PERCENT));
	pComboSP->addItem(tr("X"), QVariant(PAGE_X));

	pPagesSP = new QStackedWidget;
	pPagesSP->addWidget(new PercentPage);
	pPagesSP->addWidget(new XPage);

	QPushButton *buttonSPHelper = new QPushButton(tr("Helper"));
	connect(buttonSPHelper, SIGNAL(clicked(bool)), this, SLOT(OnSPHelperClicked(bool)));
	pTableSP = new QTableView;
	pTableSP->setSelectionBehavior(QAbstractItemView::SelectRows);
	pTableSP->setModel(new CItemModel);
	pTableSP->horizontalHeader()->setStretchLastSection(true);

	// grouphppot layout
	QGridLayout *groupSPLayout = new QGridLayout;
	groupSPLayout->addWidget(pComboSP, 0, 0);
	groupSPLayout->addWidget(pPagesSP, 1, 0);
	groupSPLayout->addWidget(buttonSPHelper, 2, 0);
	groupSPLayout->addWidget(pTableSP, 0, 1, 3, 2);
	groupSPLayout->setColumnStretch(0, 1);
	groupSPLayout->setColumnStretch(1, 2);
	groupSP->setLayout(groupSPLayout);

	// mainlayout
	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(groupHP, 0, 0);
	mainLayout->addWidget(groupSP, 1, 0);

	setLayout(mainLayout);
}

CBotTab::~CBotTab()
{

}

void CBotTab::comboHPChanged(int index)
{
	uint a = pComboHP->itemData(index).toUInt();

	if (a == PAGE_PERCENT)
	{
		pPagesHP->setCurrentIndex(PAGE_PERCENT);
	}
	else if (a == PAGE_X)
	{
		pPagesHP->setCurrentIndex(PAGE_X);
	}
	else
	{
		// ?
	}
}

void CBotTab::comboSPChanged(int index)
{
	uint a = pComboSP->itemData(index).toUInt();

	if (a == PAGE_PERCENT)
	{
		pPagesSP->setCurrentIndex(PAGE_PERCENT);
	}
	else if (a == PAGE_X)
	{
		pPagesSP->setCurrentIndex(PAGE_X);
	}
	else
	{
		// ?
	}
}

void CBotTab::OnHPHelperClicked(bool)
{
	std::vector<unsigned int> vItems;
	CInventoryWindow window(&vItems, CInventoryWindow::INVENTORY::USE);
	window.show();
	int result = window.exec();

	CItemModel *model = dynamic_cast<CItemModel*>(pTableHP->model());
	for (unsigned int itemId : vItems)
		model->addItem(itemId);

	// アイテム名の大きさにカラムのサイズを合わせる。
	pTableHP->resizeColumnToContents(1);
}

void CBotTab::OnSPHelperClicked(bool)
{
	std::vector<unsigned int> vItems;
	CInventoryWindow window(&vItems, CInventoryWindow::INVENTORY::USE);
	window.show();
	int result = window.exec();

	CItemModel *model = dynamic_cast<CItemModel*>(pTableSP->model());
	for (unsigned int itemId : vItems)
		model->addItem(itemId);

	// アイテム名の大きさにカラムのサイズを合わせる。
	pTableSP->resizeColumnToContents(1);
}

void CBotTab::showTableHPMenu(QPoint pos)
{
	QModelIndex index = pTableHP->indexAt(pos);

	QMenu *menu = new QMenu(this);

	if (index.isValid())
	{
		menu->addAction(tr("up"), this, SLOT(close()));
		menu->addAction(tr("down"), this, SLOT(close()));
		menu->addSeparator();
		menu->addAction(tr("remove"), this, SLOT(close()));
	}

	menu->addAction(tr("clear"), this, SLOT(close()));
	menu->popup(pTableHP->viewport()->mapToGlobal(pos));
}

PercentPage::PercentPage(__in QWidget *pParent) : QWidget(pParent)
{
	QSlider *slider = new QSlider;
	slider->setOrientation(Qt::Orientation::Horizontal);
	slider->setTickInterval(100);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(slider);

	setLayout(layout);
}

PercentPage::~PercentPage()
{

}

XPage::XPage(__in QWidget *pParent) : QWidget(pParent)
{
	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);
}

XPage::~XPage()
{

}