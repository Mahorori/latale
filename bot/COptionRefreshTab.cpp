#include "stdafx.h"

#include <qwidget.h>
#include "COptionRefreshTab.hpp"
#include "functions.h"
#include "pointers.h"
#include "structs.h"
#include "oprst.h"

#define OPTION_BLANK 0xFFFF

COptionRefreshTab *pOptionRefreshTab = nullptr;

COptionRefreshTab::COptionRefreshTab(__in QWidget *pParent) : QWidget(pParent)
{
	QGroupBox *grpMain = new QGroupBox(tr("Option Refresh"));
	grpMain->setCheckable(true);
	grpMain->setChecked(false);
	connect(grpMain, SIGNAL(toggled(bool)), this, SLOT(OnToggled(bool)));

	for (int i = 0; i < 5; i++)
	{
		vLblOptions.push_back(new QLabel(tr("-")));
		vComboOptions.push_back(new QComboBox);
		vComboMinValues.push_back(new QComboBox);
		vComboMaxValues.push_back(new QComboBox);
	}

	// new op
	connect(vComboOptions[0], SIGNAL(currentIndexChanged(int)), this, SLOT(OnOption1Changed(int)));
	connect(vComboOptions[1], SIGNAL(currentIndexChanged(int)), this, SLOT(OnOption2Changed(int)));
	connect(vComboOptions[2], SIGNAL(currentIndexChanged(int)), this, SLOT(OnOption3Changed(int)));
	connect(vComboOptions[3], SIGNAL(currentIndexChanged(int)), this, SLOT(OnOption4Changed(int)));
	connect(vComboOptions[4], SIGNAL(currentIndexChanged(int)), this, SLOT(OnOption5Changed(int)));

	QCheckBox *pCheckFaster = new QCheckBox(QString::fromLocal8Bit("高速化"));
	connect(pCheckFaster, SIGNAL(stateChanged(int)), this, SLOT(OnFasterChanged(int)));

	pCheckRandOrder = new QCheckBox(QString::fromLocal8Bit("順不同"));
	connect(pCheckRandOrder, SIGNAL(stateChanged(int)), this, SLOT(OnRandOrderChanged(int)));

	QGridLayout *layoutGrp = new QGridLayout;
	layoutGrp->addWidget(new QLabel(QString::fromLocal8Bit("アイテム :")), 0, 0);
	layoutGrp->addWidget(pLblItemName = new QLabel(tr("-")), 0, 1);
	layoutGrp->addWidget(pCheckFaster, 0, 4);
	layoutGrp->addWidget(pCheckRandOrder, 0, 6);
	// labels
	for (int i = 0; i < 5; i++)
	{
		layoutGrp->addWidget(new QLabel(QString::fromLocal8Bit("現在のオプション :")), i + 1, 0);
		layoutGrp->addWidget(vLblOptions[i], i + 1, 1);
		layoutGrp->addWidget(new QLabel(QString::fromLocal8Bit("新しいオプション :")), i + 1, 2);
		layoutGrp->addWidget(vComboOptions[i], i + 1, 3);
		layoutGrp->addWidget(vComboMinValues[i], i + 1, 4);
		layoutGrp->addWidget(new QLabel(tr(" ~ ")), i + 1, 5);
		layoutGrp->addWidget(vComboMaxValues[i], i + 1, 6);
	}
	layoutGrp->setColumnStretch(0, 0);
	layoutGrp->setColumnStretch(1, 1);
	layoutGrp->setColumnStretch(2, 0);
	layoutGrp->setColumnStretch(3, 2);
	layoutGrp->setColumnStretch(4, 0);
	layoutGrp->setColumnStretch(5, 0);
	layoutGrp->setColumnStretch(6, 0);
	layoutGrp->setRowStretch(6, 1);
	grpMain->setLayout(layoutGrp);

	QGridLayout *layoutMain = new QGridLayout;
	layoutMain->addWidget(grpMain, 0, 0);
	setLayout(layoutMain);

	pOptionRefreshTab = this;
}

COptionRefreshTab::~COptionRefreshTab()
{
	ToggleOptionRstHook(FALSE);
	ToggleOptionRstFaster(FALSE);
}

void COptionRefreshTab::OnToggled(bool checked)
{
	ToggleOptionRstHook(checked);
}

void COptionRefreshTab::OnOption1Changed(int index)
{
	// clear min max items
	vComboMinValues[0]->clear();
	vComboMaxValues[0]->clear();

	SPListItemBase *pListItemBase = SPListItemBase__GetInstance();
	if (!pListItemBase)
		return; // :'(

	uint option = vComboOptions[0]->itemData(index).toUInt();
	SPItemOption *pItemOption = SPListItemBase__GetOption(pListItemBase, option);

	if (pItemOption)
	{
		for (int i = pItemOption->min; i <= pItemOption->max; i++)
		{
			vComboMinValues[0]->addItem(QString("%1").arg(i), QVariant::fromValue(i));
		}
		vComboMaxValues[0]->addItem(QString("%1").arg(pItemOption->max), QVariant::fromValue(pItemOption->max));
	}
}

void COptionRefreshTab::OnOption2Changed(int index)
{
	// clear min max items
	vComboMinValues[1]->clear();
	vComboMaxValues[1]->clear();

	SPListItemBase *pListItemBase = SPListItemBase__GetInstance();
	if (!pListItemBase)
		return; // :'(

	uint option = vComboOptions[1]->itemData(index).toUInt();
	SPItemOption *pItemOption = SPListItemBase__GetOption(pListItemBase, option);

	if (pItemOption)
	{
		for (int i = pItemOption->min; i <= pItemOption->max; i++)
		{
			vComboMinValues[1]->addItem(QString("%1").arg(i), QVariant::fromValue(i));
		}
		vComboMaxValues[1]->addItem(QString("%1").arg(pItemOption->max), QVariant::fromValue(pItemOption->max));
	}
}

void COptionRefreshTab::OnOption3Changed(int index)
{
	// clear min max items
	vComboMinValues[2]->clear();
	vComboMaxValues[2]->clear();

	SPListItemBase *pListItemBase = SPListItemBase__GetInstance();
	if (!pListItemBase)
		return; // :'(

	uint option = vComboOptions[2]->itemData(index).toUInt();
	SPItemOption *pItemOption = SPListItemBase__GetOption(pListItemBase, option);

	if (pItemOption)
	{
		for (int i = pItemOption->min; i <= pItemOption->max; i++)
		{
			vComboMinValues[2]->addItem(QString("%1").arg(i), QVariant::fromValue(i));
		}
		vComboMaxValues[2]->addItem(QString("%1").arg(pItemOption->max), QVariant::fromValue(pItemOption->max));
	}
}

void COptionRefreshTab::OnOption4Changed(int index)
{
	// clear min max items
	vComboMinValues[3]->clear();
	vComboMaxValues[3]->clear();

	SPListItemBase *pListItemBase = SPListItemBase__GetInstance();
	if (!pListItemBase)
		return; // :'(

	uint option = vComboOptions[3]->itemData(index).toUInt();
	SPItemOption *pItemOption = SPListItemBase__GetOption(pListItemBase, option);

	if (pItemOption)
	{
		for (int i = pItemOption->min; i <= pItemOption->max; i++)
		{
			vComboMinValues[3]->addItem(QString("%1").arg(i), QVariant::fromValue(i));
		}
		vComboMaxValues[3]->addItem(QString("%1").arg(pItemOption->max), QVariant::fromValue(pItemOption->max));
	}
}

void COptionRefreshTab::OnOption5Changed(int index)
{
	// clear min max items
	vComboMinValues[4]->clear();
	vComboMaxValues[4]->clear();

	SPListItemBase *pListItemBase = SPListItemBase__GetInstance();
	if (!pListItemBase)
		return; // :'(

	uint option = vComboOptions[4]->itemData(index).toUInt();
	SPItemOption *pItemOption = SPListItemBase__GetOption(pListItemBase, option);

	if (pItemOption)
	{
		for (int i = pItemOption->min; i <= pItemOption->max; i++)
		{
			vComboMinValues[4]->addItem(QString("%1").arg(i), QVariant::fromValue(i));
		}
		vComboMaxValues[4]->addItem(QString("%1").arg(pItemOption->max), QVariant::fromValue(pItemOption->max));
	}
}

void COptionRefreshTab::OnRandOrderChanged(int state)
{
	for (QComboBox *pElement : vComboMinValues)
	{
		// grey out
		pElement->setEnabled(state == Qt::CheckState::Unchecked);
	}
	for (QComboBox *pElement : vComboMaxValues)
	{
		// grey out
		pElement->setEnabled(state == Qt::CheckState::Unchecked);
	}
}

void COptionRefreshTab::OnFasterChanged(int state)
{
	ToggleOptionRstFaster(state == Qt::CheckState::Checked);
}

void COptionRefreshTab::setOptionText(__in int index, __in LPCSTR lpcszText)
{
	if (vLblOptions.size() > index)
		vLblOptions[index]->setText(QString::fromLocal8Bit(lpcszText));
}

void COptionRefreshTab::OnUpdateItem(__in SPItem *pItem)
{
	SPItemOption	*pItemOptionEquip, *pItemOptionAvailable;
	CHAR			szText[256];
	DWORD			dwOption;

	std::vector<SPClientOptionArchive*> vAvailableOptions;
	std::vector<uint> vAvailableBaseOptions;
	SPListItemBase *pListItemBase = SPListItemBase__GetInstance();
	if (!pListItemBase)
		return; // :'(

	pLblItemName->setText(QString::fromLocal8Bit(pItem->pBaseItem->name.c_str()));

	for (QComboBox *pComboBox : vComboOptions)
	{
		// clear all items
		pComboBox->clear();
	}

	vAvailableOptions.clear();
	vAvailableBaseOptions.clear();

	if (pItem->pBaseItem->enchantType == ENCHANT_TYPE::GLASSES)
	{
		vAvailableBaseOptions.push_back(100);	// 筋力 %+d
		vAvailableBaseOptions.push_back(300);	// 幸運 %+d
		vAvailableBaseOptions.push_back(500);	// 魔法力 %+d
		vAvailableBaseOptions.push_back(700);	// 体力 %+d
		vAvailableBaseOptions.push_back(8300);	// 物理攻撃的中率 %+d％
		vAvailableBaseOptions.push_back(8520);	// 魔法攻撃的中率 %+d％
		vAvailableBaseOptions.push_back(10300);	// 防御力 %+d
	}
	else if (pItem->pBaseItem->enchantType == ENCHANT_TYPE::SOCKS)
	{
		vAvailableBaseOptions.push_back(100);	// 筋力 %+d
		vAvailableBaseOptions.push_back(300);	// 幸運 %+d
		vAvailableBaseOptions.push_back(500);	// 魔法力 %+d
		vAvailableBaseOptions.push_back(700);	// 体力 %+d
		vAvailableBaseOptions.push_back(6100);	// オプション発生確率 %+d％
		vAvailableBaseOptions.push_back(8700);	// 物理攻撃回避率 %+d％
		vAvailableBaseOptions.push_back(8900);	// 魔法攻撃回避率 %+d％
		vAvailableBaseOptions.push_back(10300);	// 防御力 %+d
	}
	else if (pItem->pBaseItem->enchantType == ENCHANT_TYPE::BINDU)
	{
		vAvailableBaseOptions.push_back(100);	// 筋力 %+d
		vAvailableBaseOptions.push_back(300);	// 幸運 %+d
		vAvailableBaseOptions.push_back(500);	// 魔法力 %+d
		vAvailableBaseOptions.push_back(700);	// 体力 %+d
		vAvailableBaseOptions.push_back(5700);	// Ely取得 %+d％
		vAvailableBaseOptions.push_back(6100);	// オプション発生確率 %+d％
		vAvailableBaseOptions.push_back(10300);	// 防御力 %+d
	}
	else
	{
		SPListItemBase__GetAvailableOptions(pListItemBase, pItem->pBaseItem->uEnchantType, NULL, 1, &vAvailableOptions); // 1 = 一般エンチャント
		for (SPClientOptionArchive *pElement : vAvailableOptions)
		{
			vAvailableBaseOptions.push_back(pElement->baseid);
		}
	}

	for (int i = 0; i < 5; i++)
	{
		// don't wanna copy structure so no local variable here...
		if (pItem->options[i].id == 0)
		{
			setOptionText(i, "-");
			continue;
		}

		pItemOptionEquip = SPListItemBase__GetOption(pListItemBase, pItem->options[i].id);
		if (!pItemOptionEquip)
		{
			setOptionText(i, "-");
			continue;
		}

		sprintf_s(szText, pItemOptionEquip->desc1.c_str(), pItem->options[i].value);
		setOptionText(i, szText);

		vComboOptions[i]->addItem(QString::fromLocal8Bit("未指定"), QVariant::fromValue(OPTION_BLANK));

		// add all avaiable options
		for (uint baseId : vAvailableBaseOptions)
		{
			// option id = baseid + level
			dwOption = baseId + pItemOptionEquip->level;
			pItemOptionAvailable = SPListItemBase__GetOption(pListItemBase, dwOption);
			if (pItemOptionAvailable)
			{
				// add to combobox
				vComboOptions[i]->addItem(QString::fromLocal8Bit(pItemOptionAvailable->desc2.c_str()), QVariant::fromValue(dwOption));
			}
		}
	}
}

void COptionRefreshTab::OnUpdateOption(__in SPItem *pItem)
{
	SPItemOption *pItemOption;
	CHAR szText[256];

	SPListItemBase *pListItemBase = SPListItemBase__GetInstance();
	if (!pListItemBase)
		return;

	for (int i = 0; i < 5; i++)
	{
		// don't wanna copy structure so no local variable here...
		if (pItem->options[i].id == 0)
		{
			setOptionText(i, "-");
			continue;
		}

		pItemOption = SPListItemBase__GetOption(pListItemBase, pItem->options[i].id);
		if (!pItemOption)
		{
			setOptionText(i, "-");
			continue;
		}

		sprintf_s(szText, pItemOption->desc1.c_str(), pItem->options[i].value);
		setOptionText(i, szText);
	}
}

bool COptionRefreshTab::canStart(__in SPItem *pItem)
{
	SPListItemBase *pListItemBase = SPListItemBase__GetInstance();
	if (!pListItemBase)
		return false; // :'(

	QComboBox *pComboBox;
	std::vector<uint> vOptions;
	int option_count = SPItem__GetOptionCount(pItem);

	for (int i = 0; i < option_count; i++)
	{
		pComboBox = vComboOptions[i];
		if (pComboBox->currentIndex() == -1)
		{
			// printf("新しいオプションが選択されていないため、オプションリフレッシュを開始できません。");
			return false;
		}
		uint option = pComboBox->itemData(pComboBox->currentIndex()).toUInt();

		if (option == OPTION_BLANK)
		{
			if (!pCheckRandOrder->isChecked())
			{
				// 順番通りなのでブランクを入れる。
				vOptions.push_back(OPTION_BLANK);
			}
		}
		else
		{
			// 
			vOptions.push_back(option);
			if (!pCheckRandOrder->isChecked())
			{
				// 順不同でない。
				pComboBox = vComboMinValues[i];
				if (pComboBox->currentIndex() == -1)
				{
					// printf("最小値が選択されていないため、オプションリフレッシュを開始できません。");
					return false;
				}
				uint min = pComboBox->itemData(pComboBox->currentIndex()).toUInt();

				pComboBox = vComboMaxValues[i];
				if (pComboBox->currentIndex() == -1)
				{
					// printf("最大値が選択されていないため、オプションリフレッシュを開始できません。");
					return false;
				}
				uint max = pComboBox->itemData(pComboBox->currentIndex()).toUInt();

				if (min > max)
				{
					printf("最小値最大値の設定が間違っているため、オプションリフレッシュを開始できません。");
					return false;
				}
			}
		}
	}

	// 同一のオプションが無いか調べる。
	for (int x = 0; x < vOptions.size(); x++)
	{
		if (vOptions[x] == OPTION_BLANK)
			continue;

		for (int y = 0; y < vOptions.size(); y++)
		{
			if (vOptions[y] == OPTION_BLANK)
				continue;

			if (x == y)
				continue;

			if (vOptions[x] == vOptions[y])
			{
				// printf("同一のオプションが含まれているため、オプションリフレッシュを開始できません。");
				return false;
			}
		}
	}

	// matched?
	if (pCheckRandOrder->isChecked())
	{
		// 順不同
		for (uint option : vOptions)
		{
			// 選択したオプション
			SPItemOption *pItemOption = SPListItemBase__GetOption(pListItemBase, option);
			if (!pItemOption)
				return false;

			uint baseId = pItemOption->baseId;
			bool contain = false;

			for (int i = 0; i < option_count; i++)
			{
				// don't wanna copy structure so no local variable here...
				if (pItem->options[i].id == 0)
					continue;

				// 装備のオプション
				pItemOption = SPListItemBase__GetOption(pListItemBase, pItem->options[i].id);
				if (!pItemOption)
					return false;

				if (baseId == pItemOption->baseId)
				{
					// 選択したオプションとbaseIdの１つが一致
					contain = true;
				}
			}

			if (!contain)
			{
				// 一致なし。
				return true;
			}
		}
	}
	else
	{
		// 順番
		for (int i = 0; i < option_count; i++)
		{
			// don't wanna copy structure so no local variable here...
			if (pItem->options[i].id == 0)
				continue;

			if (vOptions[i] != OPTION_BLANK)
			{
				if (vOptions[i] == pItem->options[i].id)
				{
					// 選択したオプションの一つと一致
					uint min = vComboMinValues[i]->itemData(vComboMinValues[i]->currentIndex()).toUInt();
					uint max = vComboMaxValues[i]->itemData(vComboMaxValues[i]->currentIndex()).toUInt();
					if (pItem->options[i].value < min || pItem->options[i].value > max)
					{
						// 値
						return true;
					}
				}
				else
				{
					// 一致なし。
					return true;
				}
			}
		}
	}

	// 終了
	return false;
}