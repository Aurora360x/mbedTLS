#pragma once
#include "Application/Base/Application.h"
#include "HttpManager/HttpClient/HTTPEndpoint.h"

typedef struct _APPTIME_INFO
{    
    LARGE_INTEGER qwTime;    
    LARGE_INTEGER qwAppTime;   

    float fAppTime;    
    float fElapsedTime;    
	float fTicksPerSec;
    float fSecsPerTick;    
} APPTIME_INFO;

#define OVERLAY_FONTCOLORA			0xFFFFFF00
#define OVERLAY_FONTCOLORB			0xFFFFFF00
#define OVERLAY_FONTCOLORC			0xFFFFFF00

#define APPLICATION_RESOLUTION_X		1280
#define APPLICATION_RESOLUTION_Y		720
#define APPLICATION_INTERNAL			D3DPRESENT_INTERVAL_ONE

#define XUIAPP_MSAA						D3DMULTISAMPLE_NONE
#define AVATAR_MSAA						D3DMULTISAMPLE_NONE
#define COVERFLOW_MSAA					D3DMULTISAMPLE_4_SAMPLES
#define SYSTEMMENU_MSAA					D3DMULTISAMPLE_4_SAMPLES

static const D3DVECTOR4 g_vClearColor = { 0.25f, 0.25f, 0.25f, 0.0f };
static const DWORD g_dwTileWidth   = APPLICATION_RESOLUTION_X;
static const DWORD g_dwTileHeight  = 256;
static const DWORD g_dwFrameWidth  = APPLICATION_RESOLUTION_X;
static const DWORD g_dwFrameHeight = APPLICATION_RESOLUTION_Y;

#define APP_TILE_COUNT	3

static const D3DRECT g_dwTiles[APP_TILE_COUNT] = 
{
    {             0,			0,						g_dwTileWidth,			g_dwTileHeight },
    {             0,			g_dwTileHeight,			g_dwTileWidth,		    g_dwTileHeight * 2 },
    {             0,			g_dwTileHeight * 2,		g_dwTileWidth,			g_dwFrameHeight },
};

class CHTTPTestApp : public CApplication
{
public:
	CHTTPTestApp() {m_dwCurFrontBuffer = 0;};								// Constructor
	~CHTTPTestApp() {}							// Destructor
protected:

	// Rendering surfaces and textures
    D3DSurface*                 m_pBackBuffer;
    D3DSurface*                 m_pDepthBuffer;
    D3DTexture*                 m_pFrontBuffer[2];
    DWORD                       m_dwCurFrontBuffer;
	APPTIME_INFO				g_Time;
	ATG::Font m_OverlayFont;

	// Application interface
	HRESULT Update(IDirect3DDevice9 * pDevice);
	HRESULT Initialize(IDirect3DDevice9 * pDevice, D3DPRESENT_PARAMETERS& d3dpp );
	HRESULT Render(IDirect3DDevice9 * pDevice);

	// ATG Functionality for font and timer
	ATG::Timer m_FrameRateTimer;
	float m_fRenderTime;
	DWORD m_dwFrameCounter;
	// Variable to hold MemoryStatus
	volatile DWORD m_dwAvailableRam;

private:
	HRESULT CreateRenderTargets( void );
	HRESULT InitTime();
	HRESULT UpdateTime();
	HRESULT RenderUI(IDirect3DDevice9 * pDevice);
	HRESULT RenderOverlays( IDirect3DDevice9 * pDevice, FLOAT fApplicationTime );
public:
	HRESULT StartHTTP();
	DWORD RunThread( VOID );
	HRESULT Run( VOID );
};

