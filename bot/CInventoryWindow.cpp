#include "stdafx.h"

#include "CInventoryWindow.hpp"
#include "CItemModel.hpp"
#include "pointers.h"
#include "structs.h"

CInventoryWindow::CInventoryWindow(__inout std::vector<unsigned int> *pvItems, __in unsigned long flag, QWidget *pParent) : QDialog(pParent)
{
	// popup window
	setWindowFlags(Qt::Popup | windowFlags());
	setWindowTitle(tr("Inventory Items"));

	// window ƒTƒCƒY
	// setMaximumSize(1024, 768);
	setMinimumSize(512, 384);

	vInventories.clear();
	pvItems->clear();
	m_pvItems = pvItems;

	for (int i = 0; i < 5; i++)
	{
		if ((1 << i) & flag)
		{
			QTableView *table = new QTableView;
			table->setSelectionBehavior(QAbstractItemView::SelectRows);
			table->horizontalHeader()->setStretchLastSection(true);
			CItemModel *model = new CItemModel();
			table->setModel(model);
			vInventories.push_back(table);

			// additem
			SPSubGameManager *pSubGameManager = SPSubGameManager__GetInstance();
			if (!pSubGameManager)
				continue;

			SPPlayer *pCharacter = pSubGameManager->pCharacter;
			if (!pCharacter)
				continue;

			SPInventory *pInventory = pCharacter->pInventory;
			if (!pInventory)
				continue;

			for (SPItem **ppItem : pInventory->inventories[i])
			{
				SPItem *pItem = *ppItem;
				if (pItem)
					if (pItem->uItemId != 0)
						model->addItem(pItem->uItemId);
			}
		}
		else
		{
			vInventories.push_back(nullptr);
		}
	}

	QTabWidget *tabWidget = new QTabWidget;
	tabWidget->setTabPosition(QTabWidget::TabPosition::West); // ¶‘¤

	if (flag & INVENTORY::EQUIP)
	{
		tabWidget->addTab(vInventories[0], tr("Eqp"));
		connect(vInventories[0], SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(equipDoubleClicked(const QModelIndex &)));
	}

	if (flag & INVENTORY::USE)
	{
		tabWidget->addTab(vInventories[1], tr("Use"));
		connect(vInventories[1], SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(useDoubleClicked(const QModelIndex &)));
	}

	if (flag & INVENTORY::ETC)
	{
		tabWidget->addTab(vInventories[2], tr("Etc"));
		connect(vInventories[2], SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(etcDoubleClicked(const QModelIndex &)));
	}

	if (flag & INVENTORY::EVENT)
	{
		tabWidget->addTab(vInventories[3], tr("Event"));
		connect(vInventories[3], SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(eventDoubleClicked(const QModelIndex &)));
	}

	if (flag & INVENTORY::PET)
	{
		tabWidget->addTab(vInventories[4], tr("Pet"));
		connect(vInventories[4], SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(petDoubleClicked(const QModelIndex &)));
	}

	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(tabWidget);

	setLayout(layout);
}

CInventoryWindow::~CInventoryWindow()
{

}
void CInventoryWindow::equipDoubleClicked(const QModelIndex & index)
{
	CItemModel *model = dynamic_cast<CItemModel*>(this->vInventories[0]->model());
	m_pvItems->push_back(model->getItemId(index.row()));
	close();
}

void CInventoryWindow::useDoubleClicked(const QModelIndex & index)
{
	CItemModel *model = dynamic_cast<CItemModel*>(this->vInventories[1]->model());
	m_pvItems->push_back(model->getItemId(index.row()));
	close();
}

void CInventoryWindow::etcDoubleClicked(const QModelIndex & index)
{
	CItemModel *model = dynamic_cast<CItemModel*>(this->vInventories[2]->model());
	m_pvItems->push_back(model->getItemId(index.row()));
	close();
}

void CInventoryWindow::eventDoubleClicked(const QModelIndex & index)
{
	CItemModel *model = dynamic_cast<CItemModel*>(this->vInventories[3]->model());
	m_pvItems->push_back(model->getItemId(index.row()));
	close();
}

void CInventoryWindow::petDoubleClicked(const QModelIndex & index)
{
	CItemModel *model = dynamic_cast<CItemModel*>(this->vInventories[4]->model());
	m_pvItems->push_back(model->getItemId(index.row()));
	close();
}