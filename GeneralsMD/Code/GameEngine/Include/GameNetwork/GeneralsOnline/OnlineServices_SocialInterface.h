#pragma once

#include "NGMP_include.h"
#include "NetworkMesh.h"
#include "../RankPointValue.h"
#include "../GameSpy/PersistentStorageThread.h"

struct FriendsEntry
{
	int64_t user_id = -1;
	std::string display_name;
};

struct FriendsResult
{
	std::vector<FriendsEntry> vecFriends;
	std::vector<FriendsEntry> vecPendingRequests;
};

struct BlockedResult
{
	std::vector<FriendsEntry> vecBlocked;
};

class NGMP_OnlineServices_SocialInterface
{
public:
	NGMP_OnlineServices_SocialInterface();
	~NGMP_OnlineServices_SocialInterface();

	void GetFriendsList(std::function<void(FriendsResult friendsResult)> cb);

	void GetBlockList(std::function<void(BlockedResult blockResult)> cb);

private:
	std::function<void(FriendsResult friendsResult)> m_cbOnGetFriendsList = nullptr;
	std::function<void(BlockedResult blockResult)> m_cbOnGetBlockList = nullptr;
};
