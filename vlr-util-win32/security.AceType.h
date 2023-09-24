#pragma once

#include <vlr-util/util.includes.h>

VLR_NAMESPACE_BEGIN( vlr )

enum class AceType : unsigned int
{
	Unknown, // = ACCESS_MIN_MS_ACE_TYPE
	AccessAllowed = ACCESS_ALLOWED_ACE_TYPE,
	AccessDenies = ACCESS_DENIED_ACE_TYPE,
	SystemAudit = SYSTEM_AUDIT_ACE_TYPE,
	SystemAlarm = SYSTEM_ALARM_ACE_TYPE,
	//ACCESS_MAX_MS_V2_ACE_TYPE

//#define ACCESS_ALLOWED_COMPOUND_ACE_TYPE        (0x4)
//#define ACCESS_MAX_MS_V3_ACE_TYPE               (0x4)
//
//#define ACCESS_MIN_MS_OBJECT_ACE_TYPE           (0x5)
//#define ACCESS_ALLOWED_OBJECT_ACE_TYPE          (0x5)
//#define ACCESS_DENIED_OBJECT_ACE_TYPE           (0x6)
//#define SYSTEM_AUDIT_OBJECT_ACE_TYPE            (0x7)
//#define SYSTEM_ALARM_OBJECT_ACE_TYPE            (0x8)
//#define ACCESS_MAX_MS_OBJECT_ACE_TYPE           (0x8)
//
//#define ACCESS_MAX_MS_V4_ACE_TYPE               (0x8)
//#define ACCESS_MAX_MS_ACE_TYPE                  (0x8)
//
//#define ACCESS_ALLOWED_CALLBACK_ACE_TYPE        (0x9)
//#define ACCESS_DENIED_CALLBACK_ACE_TYPE         (0xA)
//#define ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE (0xB)
//#define ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE  (0xC)
//#define SYSTEM_AUDIT_CALLBACK_ACE_TYPE          (0xD)
//#define SYSTEM_ALARM_CALLBACK_ACE_TYPE          (0xE)
//#define SYSTEM_AUDIT_CALLBACK_OBJECT_ACE_TYPE   (0xF)
//#define SYSTEM_ALARM_CALLBACK_OBJECT_ACE_TYPE   (0x10)
//
//#define SYSTEM_MANDATORY_LABEL_ACE_TYPE         (0x11)
//#define SYSTEM_RESOURCE_ATTRIBUTE_ACE_TYPE      (0x12)
//#define SYSTEM_SCOPED_POLICY_ID_ACE_TYPE        (0x13)
//#define SYSTEM_PROCESS_TRUST_LABEL_ACE_TYPE     (0x14)
//#define SYSTEM_ACCESS_FILTER_ACE_TYPE           (0x15)
//#define ACCESS_MAX_MS_V5_ACE_TYPE               (0x15)
};

VLR_NAMESPACE_END //( vlr )
