#pragma once

enum en_PACKET_TYPE : int
{
	en_PACKET_CS_CHAT_SERVER			= 0,
	en_PACKET_CS_CHAT_REQ_LOGIN			= 1,
	en_PACKET_CS_CHAT_RES_LOGIN			= 2,
	en_PACKET_CS_CHAT_REQ_SECTOR_MOVE	= 3,
	en_PACKET_CS_CHAT_RES_SECTOR_MOVE	= 4,
	en_PACKET_CS_CHAT_REQ_MESSAGE		= 5,
	en_PACKET_CS_CHAT_RES_MESSAGE		= 6,
	en_PACKET_CS_CHAT_REQ_HEARTBEAT		= 7,
};

#pragma pack(push, 1)

struct SLoginReq
{
	INT64 AccountNo;
	WCHAR ID[20];
	WCHAR Nickname[20];
	char SessionKey[64];
};

struct SLoginRes
{
	WORD type;
// -----

	BYTE Status;
	INT64 AccountNo;
};

struct SSectorReq
{
	INT64	AccountNo;
	WORD	SectorX;
	WORD	SectorY;
};

struct SSectorRes
{
	WORD type;
// -----

	INT64	AccountNo;
	WORD	SectorX;
	WORD	SectorY;
};

struct SChatReq
{
	INT64	AccountNo;
	WORD	MessageLen;
	WCHAR	Message[0];		// null 미포함
};

struct SChatRes
{
	WORD	type;
// -----

	INT64	AccountNo;
	WCHAR	ID[20];			// null 포함
	WCHAR	Nickname[20];
	WORD	MessageLen;
	WCHAR	Message[0];		// null 미포함
};

struct SHeartBeat
{
	WORD	type;
};

#pragma pack(pop)
