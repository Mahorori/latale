#ifndef CINSTANCE_MANAGER_HPP
#define CINSTANCE_MANAGER_HPP

#include <map>

#include "CMaplePacket.hpp"

typedef std::map<PVOID, CMaplePacket*> PACKET_INSTANCES;

class CInstanceManager
{
private:
	PACKET_INSTANCES m_Instances;
public:
	CMaplePacket* Find(__in PVOID pInstance);
	VOID Add(__in PVOID pInstance, __in CMaplePacket* Packet);
	VOID Remove(__in PVOID pInstance);
	VOID Clear();
	CMaplePacket* operator[](__in PVOID pInstance);
};

#endif // CINSTANCE_MANAGER_HPP
