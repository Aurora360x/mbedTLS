#pragma once

// These are the main application states
typedef enum _APPLICATION_STATE {
	APPLICATION_STATE_NONE,
	APPLICATION_STATE_DEFAULT,
	APPLICATION_STATE_LITE,
	APPLICATION_STATE_SKINUPDATER,
	APPLICATION_STATE_INSTALLER,
	APPLICATION_STATE_QUIT
} APPLICATION_STATE;

static APPLICATION_STATE g_ApplicationState;


class iAppEventObserver { 
public:
	virtual HRESULT Event_FrameUpdate( IDirect3DDevice9 * pDevice, float fApplicationTime, XINPUT_KEYSTROKE& keystroke ) = 0;
	virtual HRESULT Event_FramePreRender( IDirect3DDevice9 * pDevice, float fApplicationTime )		= 0;
	virtual HRESULT Event_FrameRender( IDirect3DDevice9 * pDevice, float fApplicationTime )			= 0;
	virtual HRESULT Event_FramePostRender( IDirect3DDevice9 * pDevice, float fApplicationTime )		= 0;
	virtual HRESULT Event_FrameOverlays( IDirect3DDevice9 * pDevice, ATG::Font& overlayFont, float fApplicationTime ) { return S_OK; };
	virtual HRESULT Event_TextureCreation( IDirect3DDevice9 * pDevice, float fApplicationTime )		= 0;
	virtual HRESULT Event_GarbageCollection( IDirect3DDevice9 * pDevice, float fApplicationTime )	= 0;
	virtual HRESULT Event_WriteDebug()														= 0;
};

class CApplication
{
public:
	
	// Constructor / Destructor
	CApplication() 	{ 
		bRunning = FALSE;
		bTerminate = FALSE;
		m_pDebugTexture = NULL;
	}

	virtual ~CApplication() { }

	virtual HRESULT RunModule( IDirect3DDevice9 * pDevice, D3DPRESENT_PARAMETERS& d3dpp );

	D3DPRESENT_PARAMETERS m_d3dpp;
	LPDIRECT3DDEVICE9 m_pd3dDevice;

	LPDIRECT3DTEXTURE9 GetDebugTexture( void )			{ return m_pDebugTexture; }
	HRESULT SuspendInput( BOOL val )					{ bSuspendInput = val; return S_OK; }

	// Registration to the RenderLoop Observer
	HRESULT AddObserver(iAppEventObserver& ref);
	HRESULT RemoveObserver(iAppEventObserver& ref);
	HRESULT ClearObservers( void );

protected:

	// Inheritable Functions used to Run A Module
	virtual HRESULT Initialize( IDirect3DDevice9 * pDevice, D3DPRESENT_PARAMETERS& d3dpp  ) { return S_OK; }
	virtual HRESULT Update( IDirect3DDevice9 * pDevice ) { return S_OK; }
	virtual HRESULT Render( IDirect3DDevice9 * pDevice ) { return S_OK; }

	// iRenderLoopObserver Notifications
	HRESULT _triggerFrameUpdate( IDirect3DDevice9 * pDevice, float fApplicationTime, XINPUT_KEYSTROKE& keystroke );
	HRESULT _triggerFramePreRender( IDirect3DDevice9 * pDevice, float fApplicationTime );
	HRESULT _triggerFrameRender( IDirect3DDevice9 * pDevice, float fApplicationTime );
	HRESULT _triggerFramePostRender( IDirect3DDevice9 * pDevice, float fApplicationTime );
	HRESULT _triggerFrameOverlays( IDirect3DDevice9 * pDevice, ATG::Font& overlayFont, float fApplicationTime );
	HRESULT _triggerTextureCreation( IDirect3DDevice9 * pDevice, float fApplicationTime );
	HRESULT _triggerGarbageCollection( IDirect3DDevice9 * pDevice, float fApplicationTime );
	HRESULT _triggerWriteDebug();

	LPDIRECT3DTEXTURE9			m_pDebugTexture;
	BOOL bSuspendInput;

private:

	// Variables
	BOOL bRunning;
	BOOL bTerminate;

	// Map to hold a list of all the registerd observers
	std::set<iAppEventObserver* const> appEventObservers;
	typedef std::set<iAppEventObserver* const> appEventItem;

};