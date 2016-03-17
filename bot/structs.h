#pragma once

#define Padding(x) struct { unsigned char __padding##x[(x)]; };

typedef LPVOID SPWindowLogin;

// ahhh
struct SPSubGameManager;

#pragma pack(push, 1)
struct SPMainGameManager
{
	Padding(0x18);
	SPSubGameManager *pSubGameManager; // name can be wrong
};

struct SPVector // name can be wrong
{
	Padding(0x08);
	FLOAT x;
	FLOAT y;
	FLOAT prev_x; // idk
	FLOAT prev_y; // idk
	FLOAT another_x; // idk
	FLOAT another_y; // idk
};

// エンチャント用のタイプ？ エンチャントできないアイテムのタイプは0
typedef enum _ENCHANT_TYPE
{
	NONE = 0,
	GLASSES = 2,			// メガネ
	TOP = 4,				// 上
	BOTTOM = 5,				// 下
	BOOTS = 8,				// 靴
	SOCKS = 9,				// ストッキング
	BINDU = 10,				// ビンディ or マント
	ONE_HANDED_SWORD = 17,	// 片手剣
	STAFF = 19,				// 杖
	isiyumi = 24,			// 弩
	BAG = 30,				// カバン
	MANA_GUARD = 87,		// マナガード
} ENCHANT_TYPE;

// base item struct
struct SPBaseItem // name can be wrong
{
	Padding(0x14);
	std::string name; // size 0x18
	Padding(0x15C);
	unsigned long flag; // 0x188 for cooldown?
	Padding(0x5C);
	union
	{
		unsigned long uEnchantType;
		ENCHANT_TYPE enchantType; // 0x1e8
	};
};

typedef struct _EQUIP_OPTION
{
	unsigned short id;
	unsigned short value;
} EQUIP_OPTION;

struct SPItem // name can be wrong
{
	Padding(0x08);
	unsigned int	uItemId;	// 0x08
	SPBaseItem		*pBaseItem;	// 0x0c
	Padding(0x08);
	unsigned int	uQuantity;	// 0x18
	Padding(0x19);
	EQUIP_OPTION	options[5];	// 0x35
};

struct SPInventory // name can be wrong
{
	Padding(0x2C);
	std::vector<SPItem**> inventories[5];
};

struct SPStatus // name can be wrong
{
	// basic stats?
	LPVOID lpv; // baseclasses?
	LPVOID pParent; // player or monster?
	std::string name; // 0x08~0x1C
	unsigned int level; // not sure
	unsigned int hp; // 0x24
	unsigned int mp; // 0x28
	unsigned int maxhp; // 0x2c
	unsigned int maxmp; // 0x30
	Padding(0x10);
	unsigned int luk; // 0x44 幸運
	unsigned int luk_total; // 0x48 幸運
	unsigned int str; // 0x4c 筋力
	unsigned int str_total; // 0x50 筋力
	unsigned int physical; // 0x54 体力
	unsigned int physical_total; // 0x58 体力
	unsigned int _int; // 0x5c 魔法
	unsigned int _int_total; // 0x60 魔法
	unsigned short element[5]; // 0x64~0x0x6C 属性値
	unsigned short dmg_min; // 0x6e
	unsigned short dmg_max; // 0x70
	unsigned short alignment2; // 0x72
	unsigned long unk; // 0x74
};

// SPStatusの継承クラス？
struct SPPlayerStatus
{
	SPStatus baseStat;
	// player stats?
	Padding(0x14);
	unsigned long long exp;		// 0x8C current exp
	unsigned long long baseexp;	// 0x94  
	unsigned long long maxexp;	// 0x9C exp for next level?

	double getExpPercentage()
	{
		unsigned long min = exp - baseexp; // should be < INT_MAX right?
		unsigned long max = maxexp - baseexp; // should be < INT_MAX right?
		return min / max;
	}
};

struct SPPlayer
{
	Padding(0xC4);
	SPVector		*pVec;			// 0xC4
	SPPlayerStatus	*pStat;			// 0xC8
	SPInventory		*pInventory;	// 0xCC
};

// SPStatusの継承クラス？
struct SPMonsterStatus
{
	SPStatus baseStat;
	// no additional stat atm
};

struct SPMonster
{
	Padding(0x04);
	unsigned long mobid; // not sure..
	unsigned long objectid;
	Padding(0x5C);
	SPVector		*pVec;	// 0x68
	SPMonsterStatus *pStat; // 0x6C
};

struct SPSubGameManager // can be wrong name
{
	Padding(0x18);
	std::vector<SPPlayer*> vPlayers;		// 0x18 player0 = your char
	std::vector<SPMonster*> vMonsters;		// 0x24
	std::vector<SPPlayer*> vEvents;			// 0x30
	std::vector<SPPlayer*> vItems;			// 0x3c dropped items(bags)
	std::vector<SPPlayer*> vChatBoards1;	// 0x48
	std::vector<SPPlayer*> vChatBoards2;	// 0x54
	Padding(0x20);
	SPPlayer *pCharacter;					// 0x80
};

struct SPNetworkManager
{
	Padding(0x18);
	struct
	{
		Padding(0x0C);
		int socket;
		BOOL fConnected;
	} *p;
	Padding(0xF8);
	std::string strId; // 0x114 username
};

// size = 0xd7
struct LoginCharacter // prob this has equips and job too
{
	Padding(0x08);
	CHAR lpszName[0x10]; // CharacterName
	UINT uLevel;
	Padding(0xBB);
};

struct SPLoggin
{
	Padding(0x000000A0);
	std::vector<LoginCharacter> vCharacters; // why not pointer? nib

	UINT getCharacterSize()
	{
		return vCharacters.size();
	}
};

struct SPListItemBase
{
	Padding(0x0C);
	LPVOID lpECX;
	Padding(0x3C);
};

struct COOL
{
	Padding(0x04);
	LPVOID lpECX;
};

struct SPWindowOptionRst // prob
{
	Padding(0x000002D0);
	int count; // opcode count 0x2d0
	Padding(0x0C);
	int slot; // (start0) 0x2e0
	SPItem **ppItem; // 0x2e4
};

struct SPItemOption
{
	Padding(0x04);
	std::string desc1;		// ex: Lv 1. 筋力 %+d
	std::string desc2;		// ex: Lv 1. 筋力 %+d ~ %+d
	unsigned long unk1;		// 0x34
	unsigned long unk2;		// 0x38
	unsigned int min;		// 0x3c 天然
	unsigned int max;		// 0x40 天然
	unsigned int gain_min;	// 0x44 値
	unsigned int gain_max;	// 0x48 値
	unsigned long unk3;		// 0x4C always 0?
	unsigned int baseId;	// 0x50
	unsigned int level;		// 0x54
};

// size 0xD8
struct SPClientOptionArchive
{
	Padding(0x0C);
	unsigned long baseid;	// 0x0C
	unsigned long level;	// 0x10
	Padding(0x20);
	unsigned long option;	// 0x34 option
	Padding(0x38);
	unsigned long type;		// 0x70 enum
	Padding(0x68);
};

struct SPChat // uh...
{
	enum COMMAND
	{
		GREEN_CHAT = 0x00000899,
	};

	struct
	{
		Padding(0x20);
		void(__thiscall * ProcessCommand)(__in SPChat *pChat, __in COMMAND command, __in LPVOID lpvData1, __in LPVOID lpvData2);
	} *pBaseclass;
};
#pragma pack(pop)