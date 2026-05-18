#pragma once

#include <vlr-util/util.AutoCleanupBase.h>
#include <vlr-util/cpp_namespace.h>

namespace vlr {

namespace util {

namespace win32 {

class CAutoCleanup_SC_HANDLE
	: public CAutoCleanupBase
{
public:
	SC_HANDLE m_hSCM = nullptr;

protected:
	virtual HRESULT DoCleanup();

public:
	CAutoCleanup_SC_HANDLE(SC_HANDLE hSCM)
		: m_hSCM{ hSCM }
	{
	}
	// Should not copy; would break cleanup
	CAutoCleanup_SC_HANDLE(const CAutoCleanup_SC_HANDLE&) = delete;
	// Move is okey though
	CAutoCleanup_SC_HANDLE(CAutoCleanup_SC_HANDLE&& other) noexcept
		: m_hSCM{ std::exchange(other.m_hSCM, nullptr) }
	{
	}
	virtual ~CAutoCleanup_SC_HANDLE()
	{
		OnDestroy_DoCleanup();
	}
};

} // namespace win32

} // namespace util

} // namespace vlr
