#include "GameNetwork/GeneralsOnline/HTTP/HTTPManager.h"
#include "../NGMP_include.h"

HTTPManager::HTTPManager() noexcept
{
	
}

void HTTPManager::SendGETRequest(const char* szURI, EIPProtocolVersion protover, std::map<std::string, std::string>& inHeaders, std::function<void(bool bSuccess, int statusCode, std::string strBody, HTTPRequest* pReq)> completionCallback, std::function<void(size_t bytesReceived)> progressCallback, int timeoutMS)
{
	std::scoped_lock<std::recursive_mutex> lock(m_Mutex);

	HTTPRequest* pRequest = PlatformCreateRequest(EHTTPVerb::HTTP_VERB_GET, protover, szURI, inHeaders, completionCallback, progressCallback, timeoutMS);

	m_vecRequestsPendingStart.push_back(pRequest);
}

void HTTPManager::SendPOSTRequest(const char* szURI, EIPProtocolVersion protover, std::map<std::string, std::string>& inHeaders, const char* szPostData, std::function<void(bool bSuccess, int statusCode, std::string strBody, HTTPRequest* pReq)> completionCallback, std::function<void(size_t bytesReceived)> progressCallback, int timeoutMS)
{
	std::scoped_lock<std::recursive_mutex> lock(m_Mutex);

	HTTPRequest* pRequest = PlatformCreateRequest(EHTTPVerb::HTTP_VERB_POST, protover, szURI, inHeaders, completionCallback, progressCallback, timeoutMS);
	pRequest->SetPostData(szPostData);

	m_vecRequestsPendingStart.push_back(pRequest);
}

void HTTPManager::SendPUTRequest(const char* szURI, EIPProtocolVersion protover, std::map<std::string, std::string>& inHeaders, const char* szData, std::function<void(bool bSuccess, int statusCode, std::string strBody, HTTPRequest* pReq)> completionCallback, std::function<void(size_t bytesReceived)> progressCallback /*= nullptr*/, int timeoutMS)
{
	std::scoped_lock<std::recursive_mutex> lock(m_Mutex);

	HTTPRequest* pRequest = PlatformCreateRequest(EHTTPVerb::HTTP_VERB_PUT, protover, szURI, inHeaders, completionCallback, progressCallback, timeoutMS);
	pRequest->SetPostData(szData);

	m_vecRequestsPendingStart.push_back(pRequest);
}

void HTTPManager::SendDELETERequest(const char* szURI, EIPProtocolVersion protover, std::map<std::string, std::string>& inHeaders, const char* szData, std::function<void(bool bSuccess, int statusCode, std::string strBody, HTTPRequest* pReq)> completionCallback, std::function<void(size_t bytesReceived)> progressCallback /*= nullptr*/, int timeoutMS)
{
	std::scoped_lock<std::recursive_mutex> lock(m_Mutex);

	HTTPRequest* pRequest = PlatformCreateRequest(EHTTPVerb::HTTP_VERB_DELETE, protover, szURI, inHeaders, completionCallback, progressCallback, timeoutMS);
	pRequest->SetPostData(szData);

	m_vecRequestsPendingStart.push_back(pRequest);
}

void HTTPManager::Shutdown()
{
	curl_multi_cleanup(m_pCurl);
	m_pCurl = nullptr;
}

HTTPRequest* HTTPManager::PlatformCreateRequest(EHTTPVerb httpVerb, EIPProtocolVersion protover, const char* szURI, std::map<std::string, std::string>& inHeaders, std::function<void(bool bSuccess, int statusCode, std::string strBody, HTTPRequest* pReq)> completionCallback, std::function<void(size_t bytesReceived)> progressCallback /*= nullptr*/, int timeoutMS /* = -1 */) noexcept
{
	HTTPRequest* pNewRequest = new HTTPRequest(httpVerb, protover, szURI, inHeaders, completionCallback, progressCallback, timeoutMS);
	return pNewRequest;
}

HTTPManager::~HTTPManager()
{
	Shutdown();
}

void HTTPManager::Initialize()
{
	m_pCurl = curl_multi_init();
	m_bProxyEnabled = DeterminePlatformProxySettings();
}

void HTTPManager::Tick()
{
	std::scoped_lock<std::recursive_mutex> lock(m_Mutex);

	// start anything needing starting
	for (HTTPRequest* pRequest : m_vecRequestsPendingStart)
	{
		pRequest->StartRequest();
		m_vecRequestsInFlight.push_back(pRequest);
	}
	m_vecRequestsPendingStart.clear();

	// perform and poll
	int numReqs = 0;
	curl_multi_perform(m_pCurl, &numReqs);
	curl_multi_poll(m_pCurl, NULL, 0, 1, NULL);

	// are we done?
	int msgq = 0;
	CURLMsg* m = curl_multi_info_read(m_pCurl, &msgq);
	std::vector<HTTPRequest*> vecItemsToRemove = std::vector<HTTPRequest*>();
	if (m != nullptr && m->msg == CURLMSG_DONE)
	{
		CURL* pCurlHandle = m->easy_handle;

		if (pCurlHandle != nullptr)
		{
			// find the associated request
			for (HTTPRequest* pRequest : m_vecRequestsInFlight)
			{
				if (pRequest != nullptr && pRequest->EasyHandleMatches(pCurlHandle))
				{
					pRequest->Threaded_SetComplete(m->data.result);
					vecItemsToRemove.push_back(pRequest);
				}
			}
		}
	}

	// remove any completed
	for (HTTPRequest* pRequestToDestroy : vecItemsToRemove)
	{
		m_vecRequestsInFlight.erase(std::remove(m_vecRequestsInFlight.begin(), m_vecRequestsInFlight.end(), pRequestToDestroy));
		delete pRequestToDestroy;
	}
}

void HTTPManager::AddHandleToMulti(CURL* pNewHandle)
{
	std::scoped_lock<std::recursive_mutex> lock(m_Mutex);
	curl_multi_add_handle(m_pCurl, pNewHandle);
}

void HTTPManager::RemoveHandleFromMulti(CURL* pHandleToRemove)
{
	std::scoped_lock<std::recursive_mutex> lock(m_Mutex);
	curl_multi_remove_handle(m_pCurl, pHandleToRemove);
}
