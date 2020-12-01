#pragma once
#include <windows.h>
#include <aclapi.h>
#include <iostream>

class SecurityHandle
{

	PSID pEveryoneSID = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	PACL pACL = NULL;
	SECURITY_ATTRIBUTES sa;
	DWORD lasterror;

public:


	unsigned int InitSecurityAttrubuts();
	DWORD getlasterror();
	SECURITY_ATTRIBUTES& getsecurityattrebut();
};