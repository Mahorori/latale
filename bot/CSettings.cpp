#include "stdafx.h"

#include <qsettings.h>
#include "CSettings.h"

void load()
{
	// ini file
	QSettings settings("setting.ini", QSettings::IniFormat);
	// utf8 thx
	settings.setIniCodec("UTF-8");

	QList<LOGIN> logins;
	int size = settings.beginReadArray("logins");
	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);
		LOGIN login;
		login.username = settings.value("username").toString();
		login.password = settings.value("password").toString();
		logins.append(login);
	}
	settings.endArray();
}

void write()
{
	QList<LOGIN> logins;

	QSettings settings;
	settings.beginWriteArray("logins");
	for (int i = 0; i < logins.size(); i++)
	{
		settings.setArrayIndex(i);
		settings.setValue("userName", logins.at(i).username);
		settings.setValue("password", logins.at(i).password);
	}
	settings.endArray();
}