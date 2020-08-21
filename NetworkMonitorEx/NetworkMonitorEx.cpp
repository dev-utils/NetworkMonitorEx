// NetworkMonitorEx.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#include <windows.h>
#include <netlistmgr.h>

#pragma comment(lib, "ole32.lib")

class MyNetWorkEvent : public INetworkListManagerEvents
{
public:
	MyNetWorkEvent() {
		m_ref = 1;
	}
	/*
	typedef enum NLM_CONNECTIVITY {
	  NLM_CONNECTIVITY_DISCONNECTED       = 0x0000, // 断开连接
	  NLM_CONNECTIVITY_IPV4_NOTRAFFIC     = 0x0001, // 不通
	  NLM_CONNECTIVITY_IPV6_NOTRAFFIC     = 0x0002,
	  NLM_CONNECTIVITY_IPV4_SUBNET        = 0x0010,
	  NLM_CONNECTIVITY_IPV4_LOCALNETWORK  = 0x0020,
	  NLM_CONNECTIVITY_IPV4_INTERNET      = 0x0040, // 有网
	  NLM_CONNECTIVITY_IPV6_SUBNET        = 0x0100,
	  NLM_CONNECTIVITY_IPV6_LOCALNETWORK  = 0x0200,
	  NLM_CONNECTIVITY_IPV6_INTERNET      = 0x0400
	} NLM_CONNECTIVITY;

	ConnectivityChanged: 0003 // 局域网，没网
	ConnectivityChanged: 0000 // 断开连接
	ConnectivityChanged: 0042 // 局域网，能上网
	*/
	virtual HRESULT STDMETHODCALLTYPE ConnectivityChanged(NLM_CONNECTIVITY newConnectivity)
	{

		printf("ConnectivityChanged: %04X\n", newConnectivity);
		return S_OK;
	}


	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		HRESULT Result = S_OK;
		if (IsEqualIID(riid, IID_IUnknown))
		{
			*ppvObject = (IUnknown *)this;
		}
		else if (IsEqualIID(riid, IID_INetworkListManagerEvents))
		{
			*ppvObject = (INetworkListManagerEvents *)this;
		}
		else
		{
			Result = E_NOINTERFACE;
		}

		return Result;
	}


	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return (ULONG)InterlockedIncrement(&m_ref);
	}


	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		LONG Result = InterlockedDecrement(&m_ref);
		if (Result == 0)
			delete this;
		return (ULONG)Result;
	}
private:
	LONG m_ref;
};


int main()
{
	CoInitialize(NULL);

	INetworkListManager *pNetworkListManager = NULL;
	HRESULT hr = CoCreateInstance(CLSID_NetworkListManager, NULL, CLSCTX_ALL, IID_INetworkListManager, (LPVOID *)&pNetworkListManager);
	printf("CLSID_NetworkListManager: %d\n", FAILED(hr));

	VARIANT_BOOL bConnected = VARIANT_FALSE;
	hr = pNetworkListManager->get_IsConnected(&bConnected);
	printf("is Connected: %d %d\n", bConnected, FAILED(hr));

	IConnectionPointContainer *pCPContainer = NULL;
	hr = pNetworkListManager->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPContainer);
	IConnectionPoint *pConnectPoint = NULL;
	hr = pCPContainer->FindConnectionPoint(IID_INetworkListManagerEvents, &pConnectPoint);

	DWORD Cookie = 0;
	MyNetWorkEvent* pMyEvent = new MyNetWorkEvent();
	pConnectPoint->Advise((IUnknown*)pMyEvent, &Cookie);
	// 必须有下面这个消息循环
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}
	pConnectPoint->Unadvise(Cookie);
	pConnectPoint->Release();

	pCPContainer->Release();
	pNetworkListManager->Release();

	system("pause");
	CoUninitialize();
	return 0;
}