#include "stdafx.h"

#include "CInstanceManager.hpp"

CMaplePacket* CInstanceManager::Find(__in PVOID pInstance)
{
	PACKET_INSTANCES::const_iterator it = m_Instances.find(pInstance);

	return it != m_Instances.end() ? it->second : NULL;
}

CMaplePacket* CInstanceManager::operator[](__in PVOID pInstance)
{
	return Find(pInstance);
}

VOID CInstanceManager::Add(__in PVOID pInstance, __in CMaplePacket* Packet)
{
	m_Instances[pInstance] = Packet;
}

VOID CInstanceManager::Remove(__in PVOID pInstance)
{
	PACKET_INSTANCES::iterator it = m_Instances.find(pInstance);

	if (it != m_Instances.end())
		m_Instances.erase(it);
}

VOID CInstanceManager::Clear()
{
	m_Instances.clear();
}
