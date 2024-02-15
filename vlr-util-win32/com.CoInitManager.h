#pragma once

#include <combaseapi.h>
#include <map>
#include <memory>
#include <mutex>

#include "com.CoInitState.h"

namespace vlr {

namespace util {

class CCoInitManager
{
protected:
	std::map<DWORD, std::weak_ptr<CCoInitState>> m_mapThreadIDToCoInitState;
	std::mutex m_mutexMapAcces;

public:
	SPCCoInitState GetCoStateForCurrentThread();

	SPCCoInitState InitializeComForOperation(const CoInitFlags oFlags = {});

};

} // namespace util

} // namespace vlr
