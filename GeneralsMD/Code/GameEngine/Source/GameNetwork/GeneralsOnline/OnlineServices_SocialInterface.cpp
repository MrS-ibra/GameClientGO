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
