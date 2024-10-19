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
	en_RoomMembers,

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

	en_GracefulShutdownRequest = 59000,
	en_GracefulShutdownResponse = 59001,


};

// TODO List
// 1. 재접속시 두어진 돌들을 다 줘야함. <vector<tuple<x, y, color>>
// 2. DB 생각. (유저 들고 오기, 상점에서 아이템 사는 거, 


// ready만 끝나면 될 듯.





// 최종
// Login C->S			[1]
// | type(2) | accountNo(8) nickLen(1) nickName(20)

// Login S->C			[2]
// | type(2) | accountNo(8) nickLen(1) nickName(20) status(1)

// Create C->S			[201]
// | type(2) | accountNo(8) roomLen(1) roomName(20)

// Create S->C(User)	[202]
// | type(2) | accountNo(8) roomNo(2) roomLen(1) roomName(20) status(1)


// Enter C->S			[301]
// | type(2) | accountNo(8) roomNo(2)

// Enter S->C(User)		[302]
// | type(2) | accountNo(8) roomNo(2) roomLen(1) roomName(max 20) status(1)

// Enter S->C(BroadCast)	[303]
// | type(2) | roomNo(2) nickLen(1) nickName(max 20)

// Enter S->C(User) [304]
// | type(2) | accountNo(8) numOfPeople(2) | [nickLen(1) nickName(max 20)]


// Leave C->S [401]
// | type(2) | accountNo(8) roomNo(2) roomLen(1) roomName(max 20)

// Leave S->C(User) [402]
// | type(2) | accountNo(8) roomNo(2) roomLen(1) roomName(max 20) status(1)

// Leave C->S(BroadCast) [403]
// | type(2) | roomNo(2) nickLen(1) nickName(max 20)


// Chat C->S			[501]
// | type(2) | accountNo(8) roomNo(2) chatLen(1) chatting(max 255)

// Chat S->C(BroadCast) [502] (O)
// | type(2) | accountNo(8) roomNo(2) nickLen(1) nickName(max 20) chatLen(1) chatName(max 255)







// len(5) | type(2) roomNo(2) from(1) to(1) nicoLen(1) nickName(nickLen max 19)
//en_ChangePositionPlayerRequest,[1001]
// 
// 
// 
//en_ChangePositionSpectatorRequest,[1002]
//en_ChangePositionPlayerResponse,[1003]
// *pPacket << type << pUser->_accountNo << roomNo << flag << from << to;
//en_ChangePositionSpectatorResponse,[1004]



// Echo C->S			[60000]
// | type(2) | data(8)

// Echo S->C			[60000]
// | type(2) | data(8)
