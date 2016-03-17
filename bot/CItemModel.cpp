#include "stdafx.h"

#include "CItemModel.hpp"
#include "functions.h"

CItemModel::CItemModel(__in QObject *pParent)
	:QAbstractTableModel(pParent)
{
	vItems.clear();
}

CItemModel::~CItemModel()
{

}

int CItemModel::rowCount(const QModelIndex &) const
{
	return vItems.size();
}

int CItemModel::columnCount(const QModelIndex &) const
{
	return 2;
}

QVariant CItemModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		switch (index.column())
		{
			case 0: // id
			{
				return QString("%1").arg(vItems[index.row()]);
			}
			case 1: // name
			{
				return QString::fromLocal8Bit(getItemName(vItems[index.row()]).c_str());
			}
			default:
				return QString("ERROR");
		}
	}
	return QVariant();
}

QVariant CItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			switch (section)
			{
				case 0:
					return QString("Id");
				case 1:
					return QString("Name");
			}
		}
	}
	return QVariant();
}

bool CItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::EditRole)
	{
		if (index.row() == vItems.size())
			vItems.push_back(value.toUInt());
	}
	return true;
}

void CItemModel::addItem(__in unsigned int itemId)
{
	emit layoutAboutToBeChanged();

	this->vItems.push_back(itemId);

	// changePersistentIndex;

	emit layoutChanged();
}

unsigned int CItemModel::getItemId(__in int index)
{
	if (vItems.size() > index)
		return vItems[index];

	return 0;
}