#pragma once
class CMonitoring {
public:


private:
	static alignas (64) long _lobbyPeopleCount;
	static alignas (64) long _roomPeopleCount;
	static long _createRoomPacketCount;
	static long _enterRoomPacketCount;
	static long _leaveRoomPacketCount;
	static long _chatRoomPacketCount;
};