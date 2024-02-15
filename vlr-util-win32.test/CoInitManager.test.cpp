#include "pch.h"

#include <vlr-util-win32/com.CoInitManager.h>

TEST(CoInitManager, general)
{
	auto oCoInitManager = vlr::util::CCoInitManager{};

	{
		auto spCoInitState = oCoInitManager.GetCoStateForCurrentThread();
		EXPECT_EQ(spCoInitState, nullptr);
	}

	{
		auto spCoInitState = oCoInitManager.InitializeComForOperation();
		EXPECT_NE(spCoInitState, nullptr);
		EXPECT_EQ(spCoInitState->m_srCoInitialize, S_OK);
		EXPECT_EQ(spCoInitState->m_srCoInitializeSecurity.isSet(), false);

		auto spCoInitState_Cached = oCoInitManager.GetCoStateForCurrentThread();
		EXPECT_NE(spCoInitState_Cached, nullptr);
	}

	// Should be uninitialized at this point
	{
		auto spCoInitState = oCoInitManager.GetCoStateForCurrentThread();
		EXPECT_EQ(spCoInitState, nullptr);
	}
}
