#include "stdafx.h"

#include <qwidget.h>
#include "CLoginTab.hpp"
#include "CUserData.hpp"
#include "autologin.h"

static CLoginTab* pLoginTab = nullptr;

CLoginTab::CLoginTab(__in QWidget *pParent) : QWidget(pParent)
{
	//
	pLoginTab = this; // lolz

	InitializeCriticalSection(&csUserData);

	// Login Information Group
	QGroupBox *infoGroupBox = new QGroupBox(tr("Login Information"));
	QLabel *idLbl = new QLabel(tr("Username :"));
	QLabel *pwLbl = new QLabel(tr("Password :"));

	pIdEdit = new QLineEdit;
	pPwEdit = new QLineEdit;
	pPwEdit->setEchoMode(QLineEdit::EchoMode::Password);
	QPushButton *addButton = new QPushButton(tr("Add"), this);
	connect(addButton, SIGNAL(clicked(bool)), this, SLOT(OnAddAccount(bool)));

	// groupbox layout
	QGridLayout *editLayout = new QGridLayout;
	editLayout->addWidget(idLbl, 0, 0);
	editLayout->addWidget(pIdEdit, 0, 1);
	editLayout->addWidget(pwLbl, 1, 0);
	editLayout->addWidget(pPwEdit, 1, 1);
	editLayout->addWidget(addButton, 2, 1, 1, 1);
	editLayout->setColumnStretch(0, 1);
	editLayout->setColumnStretch(1, 2);
	infoGroupBox->setLayout(editLayout);

	// settings group
	QGroupBox *groupSettings = new QGroupBox(tr("Settings"));
	groupSettings->setCheckable(true);
	groupSettings->setChecked(false);
	connect(groupSettings, SIGNAL(toggled(bool)), this, SLOT(OnToggled(bool)));

	QLabel *lblLogin = new QLabel(tr("Login :"));
	QLabel *lblName = new QLabel(tr("Character Name :"));

	pComboAcc = new QComboBox;
	connect(pComboAcc, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(OnComboAccCurrentIndexChanged(const QString &)));
	pComboChars = new QComboBox;
	connect(pComboChars, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(OnComboCharCurrentIndexChanged(const QString &)));

	QPushButton *removeButton = new QPushButton(tr("Remove"), this);
	connect(removeButton, SIGNAL(clicked(bool)), this, SLOT(OnRemoveAccount(bool)));
	// groupbox layout
	QGridLayout *settingsLayout = new QGridLayout;
	settingsLayout->addWidget(lblLogin, 0, 0);
	settingsLayout->addWidget(pComboAcc, 0, 1);
	settingsLayout->addWidget(lblName, 1, 0);
	settingsLayout->addWidget(pComboChars, 1, 1);
	settingsLayout->addWidget(removeButton, 2, 1);
	settingsLayout->setColumnStretch(0, 1);
	settingsLayout->setColumnStretch(1, 2);
	groupSettings->setLayout(settingsLayout);

	// ayy
	QGroupBox *groupHelper = new QGroupBox(tr("Helper"));
	QPushButton *btnLoad = new QPushButton(tr("Load Characters (Click at char select)"), this);
	connect(btnLoad, SIGNAL(clicked(bool)), this, SLOT(OnLoadCharacters(bool)));

	QGridLayout *layoutHelper = new QGridLayout;
	layoutHelper->addWidget(btnLoad, 0, 1);
	layoutHelper->setColumnStretch(0, 1);
	layoutHelper->setColumnStretch(1, 2);
	groupHelper->setLayout(layoutHelper);

	// tab layout
	QVBoxLayout *mainLayout = new QVBoxLayout; // The QVBoxLayout class lines up widgets vertically.
	mainLayout->addWidget(infoGroupBox);
	mainLayout->addWidget(groupSettings);
	mainLayout->addWidget(groupHelper);
	mainLayout->addStretch(1);
	setLayout(mainLayout);
}

CLoginTab::~CLoginTab()
{
	for (CUserData *pElement : vUserDatas)
		delete pElement;

	LeaveCriticalSection(&csUserData);

	pLoginTab = nullptr;
}

void CLoginTab::OnAddAccount(bool checked)
{
	std::string strId = pIdEdit->text().toStdString();
	std::string strPw = pPwEdit->text().toStdString();

	if (strId.length() < 6 || strPw.length() < 6)
		return;

	EnterCriticalSection(&csUserData);

	for (CUserData *pElement : vUserDatas)
	{
		// already in list
		if (strcmp(pElement->getId().c_str(), strId.c_str()) == 0)
			return;
	}

	CUserData *pItem = new CUserData(strId, strPw);
	pComboAcc->addItem(strId.c_str(), QVariant::fromValue(pItem));
	vUserDatas.push_back(pItem);
	pComboAcc->setCurrentIndex(-1);

	LeaveCriticalSection(&csUserData);
}

void CLoginTab::OnRemoveAccount(bool checked)
{
	std::string strId = pComboAcc->currentText().toStdString();
	if (strId.length() < 6)
		return;

	QMessageBox msgBox;
	msgBox.setText(tr("Are you sure you you want to remove %1?").arg(strId.c_str()));
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);

	if (msgBox.exec() == QMessageBox::Ok)
	{
		EnterCriticalSection(&csUserData);

		// remove from list
		for (auto it = vUserDatas.begin(); it != vUserDatas.end(); it++)
		{
			CUserData *pElement = *it;
			if (strcmp(pElement->getId().c_str(), strId.c_str()) == 0)
			{
				vUserDatas.erase(it, it + 1);
				pComboAcc->removeItem(pComboAcc->currentIndex());
				delete pElement;
			}
		}
		pUserData = NULL;

		LeaveCriticalSection(&csUserData);
	}
}

void CLoginTab::OnToggled(bool on)
{
	ToggleAutoLogin(on);
	ToggleAutoSelectChar(on);
}

void CLoginTab::OnLoadCharacters(bool checked)
{
	if (pUserData)
		emit this->pComboAcc->currentIndexChanged(QString::fromStdString(pUserData->getId()));
}

void CLoginTab::OnComboAccCurrentIndexChanged(const QString & text)
{
	std::string strTemp = text.toStdString();

	EnterCriticalSection(&csUserData);

	for (CUserData *pElement : vUserDatas)
	{
		if (strcmp(pElement->getId().c_str(), strTemp.c_str()) == 0)
		{
			// remove all items
			pComboChars->clear();

			for (std::string chara : pElement->getChars())
				pComboChars->addItem(QString::fromLocal8Bit(chara.c_str()));

			pUserData = pElement;
			pComboChars->setCurrentIndex(-1);
		}
	}

	LeaveCriticalSection(&csUserData);
}

void CLoginTab::OnComboCharCurrentIndexChanged(const QString & text)
{
	EnterCriticalSection(&csUserData);

	if (!pUserData)
		return;

	strCharName = text.toStdString();

	LeaveCriticalSection(&csUserData);
}

CUserData* CLoginTab::getUserData()
{
	EnterCriticalSection(&csUserData);

	CUserData *pRet = pUserData;

	LeaveCriticalSection(&csUserData);

	return pRet;
}

CUserData* CLoginTab::getUserDataById(__in std::string id)
{
	EnterCriticalSection(&csUserData);

	CUserData *pRet = nullptr;

	for (CUserData *pElement : vUserDatas)
	{
		if (strcmp(pElement->getId().c_str(), id.c_str()) == 0)
		{
			// matched
			pRet = pElement;
			break;
		}
	}

	LeaveCriticalSection(&csUserData);

	return pRet;
}

std::string CLoginTab::getCharName() const
{
	return strCharName;
}

CLoginTab* CLoginTab::getLoginTab()
{
	return pLoginTab;
}