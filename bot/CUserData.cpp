#include "stdafx.h"

#include <qwidget.h>
#include "CUserData.hpp"
#include "structs.h"

CUserData::CUserData(__in std::string id, __in std::string pw, __in QWidget *parent) : QWidget(parent)
{
	m_id = id;
	m_pw = pw;
}

CUserData::~CUserData()
{
}

std::string CUserData::getId() const
{
	return m_id;
}

std::string CUserData::getPw() const
{
	return m_pw;
}

void CUserData::loadChars(__in std::vector<LoginCharacter> *pvChars)
{
	m_vchars.clear();

	for (LoginCharacter chara : *pvChars)
		m_vchars.push_back(std::string(chara.lpszName));
}

std::vector<std::string> CUserData::getChars() const
{
	return m_vchars;
}

int CUserData::getIndexByName(__in std::string strName) const
{
	for (int i = 0; i < m_vchars.size(); i++)
	{
		if (strcmp(m_vchars[i].c_str(), strName.c_str()) == 0)
			return i;
	}

	return -1;
}