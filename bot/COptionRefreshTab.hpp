#pragma once

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
QT_END_NAMESPACE

struct SPItem;

class COptionRefreshTab : public QWidget
{
	Q_OBJECT

public:
	explicit COptionRefreshTab(__in QWidget *pParent = 0);
	~COptionRefreshTab();

private slots:
	void OnToggled(bool checked);
	void OnOption1Changed(int index);
	void OnOption2Changed(int index);
	void OnOption3Changed(int index);
	void OnOption4Changed(int index);
	void OnOption5Changed(int index);
	void OnRandOrderChanged(int state);
	void OnFasterChanged(int state);

public:
	void setOptionText(__in int index, __in LPCSTR lpcszText);
	void OnUpdateItem(__in SPItem *pItem);
	void OnUpdateOption(__in SPItem *pItem);
	bool canStart(__in SPItem *pItem);

private:
	QLabel *pLblItemName;
	QCheckBox *pCheckRandOrder;
	std::vector<QLabel*> vLblOptions;
	std::vector<QComboBox*> vComboOptions;
	std::vector<QComboBox*> vComboMinValues;
	std::vector<QComboBox*> vComboMaxValues;
};

extern COptionRefreshTab *pOptionRefreshTab;