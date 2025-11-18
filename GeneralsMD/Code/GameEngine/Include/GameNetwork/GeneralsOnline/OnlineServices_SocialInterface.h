#pragma once

#include "NGMP_include.h"
#include "NetworkMesh.h"
#include "../RankPointValue.h"
#include "../GameSpy/PersistentStorageThread.h"

struct FriendsEntry
{
	int64_t user_id = -1;
	std::string display_name;
	bool online;
	std::string presence;
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

	void AddFriend(int64_t target_user_id);

	void AcceptPendingRequest(int64_t target_user_id);
	void RejectPendingRequest(int64_t target_user_id);

	void OnChatMessage(int64_t source_user_id, int64_t target_user_id, UnicodeString unicodeStr);

	std::vector<UnicodeString> GetChatMessagesForUser(int64_t target_user_id)
	{
		if (m_mapCachedMessages.contains(target_user_id))
		{
			return m_mapCachedMessages[target_user_id];
		}

		return std::vector<UnicodeString>();
	}

	// Callbacks
	void InvokeCallback_NewFriendRequest(std::string strDisplayName)
	{
		if (m_cbOnNewFriendRequest != nullptr)
		{
			m_cbOnNewFriendRequest(strDisplayName);
		}
	}
	void RegisterForCallback_NewFriendRequest(std::function<void(std::string strDisplayName)> cbOnNewFriendRequest)
	{
		m_cbOnNewFriendRequest = cbOnNewFriendRequest;
	}

	void RegisterForCallback_OnChatMessage(std::function<void(int64_t source_user_id, int64_t target_user_id, UnicodeString unicodeStr)> cbOnChatMessage)
	{
		m_cbOnChatMessage = cbOnChatMessage;
	}

private:
	// NOTE: We cache messages here, because the UI isn't always present, but we dont want to miss messages
	// TODO_SOCIAL: Limit this
	std::map<int64_t, std::vector<UnicodeString>> m_mapCachedMessages; // user id here is who we are talking to / target user

	std::function<void(FriendsResult friendsResult)> m_cbOnGetFriendsList = nullptr;
	std::function<void(BlockedResult blockResult)> m_cbOnGetBlockList = nullptr;

	std::function<void(std::string strDisplayName)> m_cbOnNewFriendRequest = nullptr;

	std::function<void(int64_t source_user_id, int64_t target_user_id, UnicodeString unicodeStr)> m_cbOnChatMessage = nullptr;
};
