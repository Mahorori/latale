#pragma once

struct SPMainGameManager;
struct SPSubGameManager;
struct SPLoggin;
struct SPNetworkManager;
struct SPListItemBase;
struct COOL;
struct SPChat;

extern SPMainGameManager* SPMainGameManager__GetInstance();
extern SPSubGameManager* SPSubGameManager__GetInstance();
extern SPLoggin* SPLoggin__GetInstance();
extern SPNetworkManager* SPNetworkManager__GetInstance();
extern SPListItemBase* SPListItemBase__GetInstance();
extern COOL* COOL__GetInstance();
extern SPChat* SPChat__GetInstance();