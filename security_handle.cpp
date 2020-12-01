#include "security_handle.h"

/// --- инициализация дескриптора безопасности ---///
/*
 0 - ошибка AllocateAndInitializeSid
 1 - SetEntriesInAclA
 2 - LocalAlloc
 3 - InitializeSecurityDescriptor
 4 - SetSecurityDescriptorDacl
*/
unsigned int SecurityHandle::InitSecurityAttrubuts()
{
	std::string messeng;
	unsigned result = 0;
	DWORD res = 0;

	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
	{
		lasterror = GetLastError();
		result|= 1;
		return result;
	}

	ZeroMemory(&ea, 1 * sizeof(EXPLICIT_ACCESS));
	ea.grfAccessPermissions = KEY_ALL_ACCESS | MUTEX_ALL_ACCESS;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = NO_INHERITANCE;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName = (LPTSTR)pEveryoneSID;

	res = SetEntriesInAclA(1, (PEXPLICIT_ACCESSA)&ea, NULL, &pACL);
	if (res != ERROR_SUCCESS)
	{
		lasterror = GetLastError();
		result|= 2;
		return result;
	}

	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (pSD == NULL)
	{
		lasterror = GetLastError();
		result |=4;
		return result;
	}

	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
	{
		lasterror = GetLastError();
		result |= 8;
		return result;
	}

	if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
	{
		lasterror = GetLastError();
		result |= 16;
		return result;
	}

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;

	return result;
}

DWORD SecurityHandle::getlasterror()
{
	return lasterror;
}

SECURITY_ATTRIBUTES& SecurityHandle::getsecurityattrebut()
{
	return sa;
}