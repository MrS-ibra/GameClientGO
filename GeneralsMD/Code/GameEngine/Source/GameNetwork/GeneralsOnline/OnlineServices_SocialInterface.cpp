#include "GameNetwork/GeneralsOnline/json.hpp"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"
#include "../../GameSpy/PersistentStorageThread.h"
#include "../../RankPointValue.h"
#include "../OnlineServices_Init.h"
#include "../HTTP/HTTPManager.h"

NGMP_OnlineServices_SocialInterface::NGMP_OnlineServices_SocialInterface()
{

}

NGMP_OnlineServices_SocialInterface::~NGMP_OnlineServices_SocialInterface()
{

}

void NGMP_OnlineServices_SocialInterface::GetFriendsList(std::function<void(FriendsResult friendsResult)> cb)
{
	m_cbOnGetFriendsList = cb;

	std::string strURI = NGMP_OnlineServicesManager::GetAPIEndpoint("Social/Friends");
	std::map<std::string, std::string> mapHeaders;

	NGMP_OnlineServicesManager::GetInstance()->GetHTTPManager()->SendGETRequest(strURI.c_str(), EIPProtocolVersion::DONT_CARE, mapHeaders, [=](bool bSuccess, int statusCode, std::string strBody, HTTPRequest* pReq)
		{
			FriendsResult friendsResult;

			try
			{
				nlohmann::json jsonObject = nlohmann::json::parse(strBody);

				// friends
				for (const auto& friendEntryIter : jsonObject["friends"])
				{
					FriendsEntry newFriend;

					friendEntryIter["user_id"].get_to(newFriend.user_id);
					friendEntryIter["display_name"].get_to(newFriend.display_name);
					friendEntryIter["online"].get_to(newFriend.online);
					friendEntryIter["presence"].get_to(newFriend.presence);


					friendsResult.vecFriends.push_back(newFriend);
				}

				// pending requests
				for (const auto& friendEntryIter : jsonObject["pending_requests"])
				{
					FriendsEntry newEntry;

					friendEntryIter["user_id"].get_to(newEntry.user_id);
					friendEntryIter["display_name"].get_to(newEntry.display_name);


					friendsResult.vecPendingRequests.push_back(newEntry);
				}
			}
			catch (...)
			{

			}

			if (m_cbOnGetFriendsList != nullptr)
			{
				// TODO_SOCIAL: Clean this up on exit etc
				m_cbOnGetFriendsList(friendsResult);
				m_cbOnGetFriendsList = nullptr;
			}
		});
}

void NGMP_OnlineServices_SocialInterface::GetBlockList(std::function<void(BlockedResult blockResult)> cb)
{
	m_cbOnGetBlockList = cb;

	std::string strURI = NGMP_OnlineServicesManager::GetAPIEndpoint("Social/Blocked");
	std::map<std::string, std::string> mapHeaders;

	NGMP_OnlineServicesManager::GetInstance()->GetHTTPManager()->SendGETRequest(strURI.c_str(), EIPProtocolVersion::DONT_CARE, mapHeaders, [=](bool bSuccess, int statusCode, std::string strBody, HTTPRequest* pReq)
		{
			BlockedResult blockedResult;

			try
			{
				nlohmann::json jsonObject = nlohmann::json::parse(strBody);

				for (const auto& blockedEntryIter : jsonObject["blocked"])
				{
					FriendsEntry newEntry;

					blockedEntryIter["user_id"].get_to(newEntry.user_id);
					blockedEntryIter["display_name"].get_to(newEntry.display_name);


					blockedResult.vecBlocked.push_back(newEntry);
				}
			}
			catch (...)
			{

			}

			if (m_cbOnGetBlockList != nullptr)
			{
				// TODO_SOCIAL: Clean this up on exit etc
				m_cbOnGetBlockList(blockedResult);
				m_cbOnGetBlockList = nullptr;
			}
		});
}

void NGMP_OnlineServices_SocialInterface::AddFriend(int64_t target_user_id)
{
	std::string strURI = std::format("{}/{}", NGMP_OnlineServicesManager::GetAPIEndpoint("Social/Friends/Requests"), target_user_id);
	std::map<std::string, std::string> mapHeaders;

	NGMP_OnlineServicesManager::GetInstance()->GetHTTPManager()->SendPUTRequest(strURI.c_str(), EIPProtocolVersion::DONT_CARE, mapHeaders, "", [=](bool bSuccess, int statusCode, std::string strBody, HTTPRequest* pReq)
		{

		});
}

void NGMP_OnlineServices_SocialInterface::AcceptPendingRequest(int64_t target_user_id)
{
	std::string strURI = std::format("{}/{}", NGMP_OnlineServicesManager::GetAPIEndpoint("Social/Friends/Requests"), target_user_id);
	std::map<std::string, std::string> mapHeaders;

	NGMP_OnlineServicesManager::GetInstance()->GetHTTPManager()->SendPOSTRequest(strURI.c_str(), EIPProtocolVersion::DONT_CARE, mapHeaders, "", [=](bool bSuccess, int statusCode, std::string strBody, HTTPRequest* pReq)
		{

		});
}

void NGMP_OnlineServices_SocialInterface::RejectPendingRequest(int64_t target_user_id)
{
	std::string strURI = std::format("{}/{}", NGMP_OnlineServicesManager::GetAPIEndpoint("Social/Friends/Requests"), target_user_id);
	std::map<std::string, std::string> mapHeaders;

	NGMP_OnlineServicesManager::GetInstance()->GetHTTPManager()->SendDELETERequest(strURI.c_str(), EIPProtocolVersion::DONT_CARE, mapHeaders, "", [=](bool bSuccess, int statusCode, std::string strBody, HTTPRequest* pReq)
		{

		});
}

void NGMP_OnlineServices_SocialInterface::OnChatMessage(int64_t source_user_id, int64_t target_user_id, UnicodeString unicodeStr)
{
	if (m_cbOnChatMessage != nullptr)
	{
		m_cbOnChatMessage(source_user_id, target_user_id, unicodeStr);
	}

	// also cache it incase UI isnt visible
	NGMP_OnlineServices_AuthInterface* pAuthInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_AuthInterface>();
	if (pAuthInterface != nullptr)
	{
		int64_t user_id_to_store = -1;

		// was it me chatting?
		if (pAuthInterface->GetUserID() == source_user_id)
		{
			// cache under target user
			user_id_to_store = target_user_id;
		}
		else
		{
			// cache under source user
			user_id_to_store = source_user_id;
		}

		// does it exist yet?
		if (!m_mapCachedMessages.contains(target_user_id))
		{
			m_mapCachedMessages[target_user_id] = std::vector<UnicodeString>();
		}

		m_mapCachedMessages[user_id_to_store].push_back(unicodeStr);
	}
}
