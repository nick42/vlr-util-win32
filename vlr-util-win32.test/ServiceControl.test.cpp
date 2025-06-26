#include "pch.h"
#include "vlr-util-win32/ServiceControl.h"

#include <gtest/gtest.h>

#include "vlr-util/StringCompare.h"

using namespace vlr::util;

struct TestServiceControl
	: public testing::Test
{
	vlr::win32::CServiceControl m_oServiceControl_LocalSystem;

	TestServiceControl()
	{
		m_oServiceControl_LocalSystem.Connect(vlr::ops::CNetworkTargetInfo{}, SC_MANAGER_CONNECT);
	}
};

TEST_F(TestServiceControl, SCM_QueryServiceConfig)
{
	SResult sr;

	auto hSCM = m_oServiceControl_LocalSystem.GetOpenHandle_SCM_OrNull();
	EXPECT_NE(hSCM, nullptr);

	// Test failure case: invalid handle
	{
		SC_HANDLE hInvalid{};

		std::vector<BYTE> vecServiceConfigData;
		sr = m_oServiceControl_LocalSystem.SCM_QueryServiceConfig(hInvalid, vecServiceConfigData);
		EXPECT_TRUE(!sr.isSuccess());
	}

	// Note: Using a test service which should meet the following criteria:
	// - Available on all versions of Windows
	// - Accessible to query by all user accounts
	static constexpr vlr::tzstring_view svzServiceName_Netlogon = _T("Netlogon");

	SC_HANDLE hService{};
	sr = m_oServiceControl_LocalSystem.SCM_OpenService(
		svzServiceName_Netlogon,
		SERVICE_QUERY_CONFIG,
		hService);
	EXPECT_EQ(sr, S_OK);
	EXPECT_NE(hService, nullptr);
	auto oOnDestroy_CloseService = vlr::util::win32::CAutoCleanup_SC_HANDLE{ hService };

	auto fValidateResultData = [&](const std::vector<BYTE>& vecServiceConfigData)
	{
		EXPECT_GE(vecServiceConfigData.size(), sizeof(QUERY_SERVICE_CONFIG));

		auto pServiceConfig = reinterpret_cast<const QUERY_SERVICE_CONFIG*>(vecServiceConfigData.data());
		EXPECT_TRUE(vlr::StringCompare::CI().AreEqual(pServiceConfig->lpDisplayName, svzServiceName_Netlogon));
	};

	// Test initial buffer empty
	{
		std::vector<BYTE> vecServiceConfigData;
		sr = m_oServiceControl_LocalSystem.SCM_QueryServiceConfig(hService, vecServiceConfigData);
		EXPECT_TRUE(sr.isSuccess());
		fValidateResultData(vecServiceConfigData);
	}

	// Test initial buffer too small
	{
		std::vector<BYTE> vecServiceConfigData;
		vecServiceConfigData.resize(1);
		sr = m_oServiceControl_LocalSystem.SCM_QueryServiceConfig(hService, vecServiceConfigData);
		EXPECT_TRUE(sr.isSuccess());
		fValidateResultData(vecServiceConfigData);
	}

	// Test initial buffer presumably sufficient
	{
		std::vector<BYTE> vecServiceConfigData;
		vecServiceConfigData.resize(4096);
		sr = m_oServiceControl_LocalSystem.SCM_QueryServiceConfig(hService, vecServiceConfigData);
		EXPECT_TRUE(sr.isSuccess());
		fValidateResultData(vecServiceConfigData);
	}
}
