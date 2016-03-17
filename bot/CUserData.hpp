#pragma once

struct LoginCharacter;

class CUserData : public QWidget
{
	Q_OBJECT

public:
	explicit CUserData(__in std::string id, __in std::string pw, __in QWidget *parent = 0);
	~CUserData();

public:
	void loadChars(__in std::vector<LoginCharacter> *pvChars);
	std::vector<std::string> getChars() const;
	int getIndexByName(__in std::string strName) const;

public:
	std::string getId() const;
	std::string getPw() const;

private:
	std::string m_id, m_pw, m_char;
	std::vector<std::string> m_vchars;
};