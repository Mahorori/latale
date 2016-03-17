#pragma once

#include <qwidget.h>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QComboBox;
QT_END_NAMESPACE

class CUserData;

class CLoginTab : public QWidget
{
	Q_OBJECT

public:
	explicit CLoginTab(__in QWidget *pParent = 0);
	~CLoginTab();

public:
	CUserData* getUserData();
	CUserData* getUserDataById(__in std::string id);
	std::string getCharName() const;

private slots:
	void OnAddAccount(bool checked);
	void OnRemoveAccount(bool checked);
	void OnToggled(bool on);
	void OnLoadCharacters(bool checked);
	void OnComboAccCurrentIndexChanged(const QString & text);
	void OnComboCharCurrentIndexChanged(const QString & text);

public:
	static CLoginTab* getLoginTab();

private:
	QLineEdit *pIdEdit, *pPwEdit;
	QComboBox *pComboChars;
	QComboBox *pComboAcc;

	std::vector<CUserData*> vUserDatas;
	CUserData *pUserData;
	std::string strCharName;

	CRITICAL_SECTION csUserData;
};