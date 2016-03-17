#pragma once

#include <qdialog.h>
#include <qwidget.h>

#include "CItemModel.hpp"

class CInventoryWindow : public QDialog
{
	Q_OBJECT;

public:
	enum INVENTORY
	{
		EQUIP	= 0x01,
		USE		= 0x02,
		ETC		= 0x04,
		EVENT	= 0x08,
		PET		= 0x10,
		ALL = EQUIP | USE | ETC | EVENT | PET,
	};

public:
	CInventoryWindow(__inout std::vector<unsigned int> *pvItems, __in unsigned long flag, __in QWidget *pParent = 0);
	~CInventoryWindow();

private slots:
	void equipDoubleClicked(const QModelIndex & index);
	void useDoubleClicked(const QModelIndex & index);
	void etcDoubleClicked(const QModelIndex & index);
	void eventDoubleClicked(const QModelIndex & index);
	void petDoubleClicked(const QModelIndex & index);

private:
	std::vector<QTableView*> vInventories;
	std::vector<unsigned int> *m_pvItems;
	QAbstractItemModel *m_model;
};