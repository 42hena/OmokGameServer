#pragma once
enum {
	en_Login = 0,
	en_LoginRequest,
	en_LoginResponse,

	en_HeartBeatRequest,
	en_HeartBeatResponse,

	en_Lobby = 100,
	en_RoomListRequest,
	en_RoomListResponse,

	en_Room = 200,
	en_CreateRoomRequest,
	en_CreateRoomResponse,

	en_EnterRoom = 300,
	en_EnterRoomRequest,
	en_EnterRoomResponse,
	en_EnterRoomStateResponse,

	en_LeaveRoom = 400,
	en_LeaveRoomRequest,
	en_LeaveRoomResponse,
	en_LeaveRoomStateResponse,

	en_Chat = 500,
	en_ChatRequest,
	en_ChatResponse,

	en_Position = 1000,
	en_ChangePositionPlayerRequest,
	en_ChangePositionSpectatorRequest,
	en_ChangePositionPlayerResponse,
	en_ChangePositionSpectatorResponse,

	en_Game = 1100,
	en_ReadyRequest,
	en_ReadyResponse,
	en_CancelReadyRequest,
	en_CancelReadyResponse,
	en_StartResponse,

	en_PlayerResponse,
	en_RecordResponse,

	en_PutStoneRequest,
	en_PutStoneResponse,

	en_GameOverRecordResponse,
	en_GameOverResponse,
};

// TODO List
// 1. 재접속시 두어진 돌들을 다 줘야함. <vector<tuple<x, y, color>>
// 2. DB 생각. (유저 들고 오기, 상점에서 아이템 사는 거, 


// ready만 끝나면 될 듯.

