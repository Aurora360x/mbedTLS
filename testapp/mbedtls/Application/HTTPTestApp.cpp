#pragma once
#include "stdafx.h"
#include "HTTPTestApp.h"
#include "TestClass/TestClass.h"
#include "HttpManager/HttpManager.h"

HRESULT CHTTPTestApp::Initialize(IDirect3DDevice9 * pDevice, D3DPRESENT_PARAMETERS& d3dpp )
{
	// Initialize our Time 
	InitTime(); 

	// Let's create our back buffer, depth stencil, PT Tiles, and double front buffers
	if( CreateRenderTargets() == S_FALSE ) {
		return S_FALSE;
	}

	if( m_OverlayFont.Create( "game:\\Media\\Fonts\\Arial_16.xpr" ) == S_OK ) {
		// Set our font window to full resolution
		m_OverlayFont.SetWindow(0, 0, 1280, 720);
	}

	RunTests();

	// Return successfully
	return S_OK;
}
HRESULT CHTTPTestApp::Update(IDirect3DDevice9 * pDevice)
{
	// Process Queued APCs
	SleepEx( 0, TRUE );

	// Update our Time
	UpdateTime();

	// Update user input
	XINPUT_KEYSTROKE keystroke;
	XInputGetKeystroke( XUSER_INDEX_ANY, XINPUT_FLAG_ANYDEVICE, &keystroke );

	// Update our external modules
	_triggerFrameUpdate( pDevice, g_Time.fElapsedTime, keystroke );  

	// Return Successfully
    return S_OK;
}
HRESULT CHTTPTestApp::Render(IDirect3DDevice9 * pDevice)
{
	// Render modules prior to XUI Rendering 
	_triggerFramePreRender( pDevice, g_Time.fElapsedTime );

	// Render modules prior to XUI Rendering 
	_triggerFrameRender( pDevice, g_Time.fElapsedTime );

	// Render our main XUI Scene
	RenderUI( pDevice );

	// Render modules after XUI Rendering
	_triggerFramePostRender( pDevice, g_Time.fElapsedTime );

	// With remaining time in frame, create required textures
	_triggerTextureCreation( pDevice, g_Time.fElapsedTime );


	// Synchronize the buffer to the presentation interval to avoid screen tearings
	m_pd3dDevice->SynchronizeToPresentationInterval();

	// Present the back buffer and swap frame buffers to continue rendering
	m_pd3dDevice->Swap( m_pFrontBuffer[ m_dwCurFrontBuffer ], NULL );
	m_dwCurFrontBuffer = ( m_dwCurFrontBuffer + 1 ) % 2;

	// Clean up after device is free of rendering
	_triggerGarbageCollection( pDevice, g_Time.fElapsedTime );

	// write Out Debug Messages
	_triggerWriteDebug();

	// Return Successfully
    return S_OK;
}
HRESULT CHTTPTestApp::RenderUI(IDirect3DDevice9 * pDevice)
{	 
	PIXBeginNamedEvent( 0, "RenderUI" );

	// Set our render target and depth buffer 
	m_pd3dDevice->SetRenderTarget( 0, m_pBackBuffer );
	m_pd3dDevice->SetDepthStencilSurface( m_pDepthBuffer );

	// Begin Rendering our Scene and all of it's components
	m_pd3dDevice->BeginTiling( 0, ARRAYSIZE(g_dwTiles), g_dwTiles, &g_vClearColor, 1, 0 );
	{
		// Render our XUI Scene
		//AppUIHelper().RenderUI( pDevice, State.HorizontalOverscan, State.VeritcalOverscan, 1280, 720 );

		// Render our Debug Overlays
		RenderOverlays( pDevice, g_Time.fElapsedTime );
	}
	m_pd3dDevice->EndTiling( 0, NULL, m_pFrontBuffer[m_dwCurFrontBuffer], NULL, 1, 0, NULL );

	PIXEndNamedEvent();

	// Return Successfully
    return S_OK;
}
HRESULT CHTTPTestApp::RenderOverlays( IDirect3DDevice9 * pDevice, FLOAT fApplicationTime )
{	
	// Mark current statistics for calculations
	//float frameRate = 1.0f / g_Time.fElapsedTime;
	m_FrameRateTimer.MarkFrame();

	// Store available ram for display
	MEMORYSTATUS memStat;
	GlobalMemoryStatus(&memStat);
	m_dwAvailableRam = memStat.dwAvailPhys; 
	
	// Begin Rendering Font
	m_OverlayFont.Begin(); 

	WCHAR szText[100];
	FLOAT x = 1240.0f, y = 60.0f;
	m_OverlayFont.SetScaleFactors( 0.75f, 0.75f );
	//swprintf_s(szText, L"Frame Rate:  %0.4f fps", frameRate );
	m_OverlayFont.DrawText( x, y, OVERLAY_FONTCOLORB, m_FrameRateTimer.GetFrameRate(), ATGFONT_RIGHT );
	
	m_OverlayFont.SetScaleFactors( 0.75f, 0.75f ); 
	swprintf_s(szText, L"Elapsed Time:  %0.4f ms", g_Time.fElapsedTime);
	m_OverlayFont.DrawText( x, y + 15.0f, OVERLAY_FONTCOLORB, szText, ATGFONT_RIGHT );

	m_OverlayFont.SetScaleFactors( 0.75f, 0.75f );
	swprintf_s(szText, L"Run Time:  %8.2f seconds", g_Time.fAppTime );
	m_OverlayFont.DrawText( x, y + 30.0f, OVERLAY_FONTCOLORB, szText, ATGFONT_RIGHT ); 

	m_OverlayFont.SetScaleFactors( 0.75f, 0.75f );
	swprintf_s(szText, L"Free Mem:  %d bytes", m_dwAvailableRam );
	m_OverlayFont.DrawText( x, y + 45.0f, OVERLAY_FONTCOLORB, szText, ATGFONT_RIGHT );

	_triggerFrameOverlays( pDevice, m_OverlayFont, 0.0f );

	m_OverlayFont.End();
	return S_OK;
}
HRESULT CHTTPTestApp::CreateRenderTargets( void )
{
	HRESULT retVal;  
	
	// Initialize D3DSurface Parameters
    D3DSURFACE_PARAMETERS ddsParams; 
    memset( &ddsParams, 0, sizeof( D3DSURFACE_PARAMETERS ) );
	ddsParams.Base = 0;

	// Create RenderTarget for use with Predicated Tiling
	retVal = m_pd3dDevice->CreateRenderTarget( g_dwTileWidth, g_dwTileHeight, D3DFMT_X8R8G8B8, XUIAPP_MSAA, 0, 0, &m_pBackBuffer, &ddsParams );
	if(retVal != D3D_OK) {
		//LOG::Write( LOG::Error, "CAuroraApp", "Predicated Tiling Render Target could not be created.  HRESULT = %X", retVal);
		return S_FALSE;
	}
			
	// Set up the surface size parameters
	ddsParams.Base = m_pBackBuffer->Size / GPU_EDRAM_TILE_SIZE;
	ddsParams.HierarchicalZBase = 0;

	// Create Depth Stencil Surface
	retVal = m_pd3dDevice->CreateDepthStencilSurface( g_dwTileWidth, g_dwTileHeight, D3DFMT_D24S8, XUIAPP_MSAA, 0, 0, &m_pDepthBuffer, &ddsParams );
	if(retVal != D3D_OK) {
		///LOG::Write( LOG::Error, "CAuroraApp", "Depth Stencil Surface could not be created. HRESULT = %X", retVal);
		return S_FALSE;
	}

	// Create First Frame Buffer
	retVal = m_pd3dDevice->CreateTexture( g_dwFrameWidth, g_dwFrameHeight, 1, 0, D3DFMT_LE_X8R8G8B8, 0, &m_pFrontBuffer[0], NULL );
	if(retVal != D3D_OK) {
		//LOG::Write( LOG::Error, "CAuroraApp", "Frame Buffer #1 could not be created.  HRESULT = %X", retVal);
		return S_FALSE;
	}

	// Create Second Frame Buffer
	retVal = m_pd3dDevice->CreateTexture( g_dwFrameWidth, g_dwFrameHeight, 1, 0, D3DFMT_LE_X8R8G8B8, 0, &m_pFrontBuffer[1], NULL );
	if(retVal != D3D_OK) {
		//LOG::Write( LOG::Error, "CAuroraApp", "Frame Buffer #2 could not be created.  HRESULT = %X", retVal);
		return S_FALSE;
	}
	
	// Return Successfully
	return S_OK;
}
HRESULT CHTTPTestApp::InitTime()
{    
    // Get the frequency of the timer
    LARGE_INTEGER qwTicksPerSec;
    QueryPerformanceFrequency( &qwTicksPerSec );
	g_Time.fTicksPerSec = (float)qwTicksPerSec.QuadPart;
    g_Time.fSecsPerTick = 1.0f / (float)qwTicksPerSec.QuadPart;

    // Save the start time
    QueryPerformanceCounter( &g_Time.qwTime );
    
    // Zero out the elapsed and total time
    g_Time.qwAppTime.QuadPart = 0;
    g_Time.fAppTime = 0.0f; 
    g_Time.fElapsedTime = 0.0f;    

	return S_OK;
}
HRESULT CHTTPTestApp::UpdateTime()
{
    LARGE_INTEGER qwNewTime; 
    LARGE_INTEGER qwDeltaTime;
    
    QueryPerformanceCounter( &qwNewTime );    
    qwDeltaTime.QuadPart = qwNewTime.QuadPart - g_Time.qwTime.QuadPart;

    g_Time.qwAppTime.QuadPart += qwDeltaTime.QuadPart;    
    g_Time.qwTime.QuadPart     = qwNewTime.QuadPart;

	static float timeScale = 1.0f;
    
	g_Time.fElapsedTime		 = ((((float) qwDeltaTime.QuadPart) / ((float) g_Time.fTicksPerSec))*1000.0f) * timeScale;
	if( g_Time.fElapsedTime > 100.0f ) g_Time.fElapsedTime = 100.0f;
    g_Time.fAppTime          = g_Time.fSecsPerTick * ((FLOAT)(g_Time.qwAppTime.QuadPart)) * timeScale;

	return S_OK;
}



