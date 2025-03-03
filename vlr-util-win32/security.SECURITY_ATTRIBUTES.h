#pragma once

#include "vlr-util/util.Result.h"

namespace vlr {

namespace util {

namespace win32 {

namespace security {

namespace detail {

struct CAutoInit_SECURITY_ATTRIBUTES
	: public SECURITY_ATTRIBUTES
{
	CAutoInit_SECURITY_ATTRIBUTES()
		: SECURITY_ATTRIBUTES{}
	{
		static_assert(sizeof(CAutoInit_SECURITY_ATTRIBUTES) == sizeof(SECURITY_ATTRIBUTES));
		nLength = sizeof(SECURITY_ATTRIBUTES);
	}
};

} // namespace detail

class CSecurityAttributes
{
protected:
	detail::CAutoInit_SECURITY_ATTRIBUTES m_stSecurityAttributes{};
	bool m_bRawValueSet_SecurityDescriptor = false;

public:
	inline decltype(auto) withSecurityDescriptor(LPVOID pvSecurityDescriptor)
	{
		m_stSecurityAttributes.lpSecurityDescriptor = pvSecurityDescriptor;
		// Note: We set this flag IFF the caller set a non-NULL pointer
		m_bRawValueSet_SecurityDescriptor = (m_stSecurityAttributes.lpSecurityDescriptor != nullptr);
		return *this;
	}
	inline decltype(auto) withInheritHandle(bool bValue)
	{
		m_stSecurityAttributes.bInheritHandle = bValue;
		return *this;
	}

public:
	LPSECURITY_ATTRIBUTES GetWin32Ptr()
	{
		return &m_stSecurityAttributes;
	}
};

} // namespace security

} // namespace win32

} // namespace util

} // namespace vlr
