#pragma once

#include <qabstractitemmodel.h>

class CItemModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	CItemModel(__in QObject *pParent = 0);
	~CItemModel();

public:
	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_INVOKABLE;

public:
	void addItem(__in unsigned int itemId);
	unsigned int getItemId(__in int index);

private:
	std::vector<unsigned int> vItems;
};