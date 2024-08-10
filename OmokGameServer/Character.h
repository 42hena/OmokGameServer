#pragma once

struct SCharacter
{
public:
	SCharacter()
		: accountNo(0), sx(-1), sy(-1), flag(0)
	{
		id[0] = 0;
		nickName[0] = 0;
		sessionKey[0] = 0;
		sessionID = -1;
		lastUpdateTime = timeGetTime();
	}

	~SCharacter()
	{}

public:
	INT64	accountNo;
	WCHAR	id[20];
	WCHAR	nickName[20];
	char	sessionKey[64];
	WORD	sx;
	WORD	sy;
	uintptr_t	sessionID;
	DWORD	lastUpdateTime;
	BYTE	flag;
};
