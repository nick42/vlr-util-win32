#include "pch.h"
#include "com.CoInitManager.h"

namespace vlr {

namespace util {

SPCCoInitState CCoInitManager::GetCoStateForCurrentThread()
{
	auto slMapAccess = std::scoped_lock{ m_mutexMapAcces };

	auto dwThreadID = ::GetCurrentThreadId();
	auto iterIndex = m_mapThreadIDToCoInitState.find(dwThreadID);
	if (iterIndex == m_mapThreadIDToCoInitState.end())
	{
		// No cached state
		return {};
	}

	auto wpCoInitState = iterIndex->second;
	auto spCoInitState = wpCoInitState.lock();
	if (!spCoInitState)
	{
		// References gone; can clear from map
		m_mapThreadIDToCoInitState.erase(iterIndex);
		return {};
	}

	// Outstanding reference; return instance
	return spCoInitState;
}

SPCCoInitState CCoInitManager::InitializeComForOperation(const CoInitFlags oFlags /*= {}*/)
{
	static const auto _tFailureValue = SPCCoInitState{};

	// Note: Using this construction, as we're going to have std::weak_ptr instances
	auto spCoInitState = std::shared_ptr<CCoInitState>{ new CCoInitState };
	VLR_ASSERT_NONZERO_OR_RETURN_FAILURE_VALUE(spCoInitState);

	spCoInitState->CallCoInitialize(oFlags);

	{
		auto slMapAccess = std::scoped_lock{ m_mutexMapAcces };
		m_mapThreadIDToCoInitState[spCoInitState->m_dwInitThreadID] = spCoInitState;
	}

	return spCoInitState;
}

} // namespace util

} // namespace vlr
