#pragma once
#include "stdafx.h"
#include "Application.h"

HRESULT CApplication::RunModule( IDirect3DDevice9 * pDevice, D3DPRESENT_PARAMETERS& d3dpp )
{
	// First check to make sure the pDevice is valid
	if( pDevice == NULL || bRunning == TRUE)
		return S_FALSE;

	// Set the flag to let the module know it is running
	bRunning = TRUE;
	m_pd3dDevice = pDevice;
	m_d3dpp = d3dpp;

	// Run the modules initialization routines
	if( Initialize( pDevice, d3dpp ) != S_OK )
		return S_FALSE;

	// Module Loop
	while( bTerminate == FALSE ) 
	{
		// Update the module
		Update( pDevice );
		// Render the module
		Render( pDevice );
	}

	// Rest our variables
	bRunning = FALSE;
	bTerminate = FALSE;

	// Return Successfully
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//    iAppEventObserver Functions and Implementation
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CApplication::AddObserver(iAppEventObserver& Observer) {
	// Add new observer to our observer list
	appEventObservers.insert(&Observer);

	return S_OK;
}

HRESULT CApplication::RemoveObserver(iAppEventObserver& Observer) {
	// Remove observer from our observer list
	appEventObservers.erase(&Observer);

	return S_OK;
}

HRESULT CApplication::ClearObservers( void ) {
	// Clear all of the observers from our list
	appEventObservers.clear();

	return S_OK;
}

HRESULT CApplication::_triggerFrameUpdate( IDirect3DDevice9 * pDevice, float fApplicationTime, XINPUT_KEYSTROKE& keystroke ) {
	// Let observers know that a message was triggered
	for(appEventItem::const_iterator it = appEventObservers.begin(); it != appEventObservers.end(); ++it ) 
	{
		// Obtain a pointer to the current app event
		iAppEventObserver * appEvent = (*it);
		if( appEvent ) appEvent->Event_FrameUpdate( pDevice, fApplicationTime, keystroke );
	}
	// Return Successfully
	return S_OK;
}

HRESULT CApplication::_triggerFramePreRender( IDirect3DDevice9 * pDevice, float fApplicationTime ) {
	// Let observers know that a message was triggered
	for(appEventItem::const_iterator it = appEventObservers.begin(); it != appEventObservers.end(); ++it ) 
	{
		// Obtain a pointer to the current app event
		iAppEventObserver * appEvent = (*it);
		if( appEvent ) appEvent->Event_FramePreRender( pDevice, fApplicationTime );
	}
	// Return Successfully
	return S_OK;
}

HRESULT CApplication::_triggerFrameRender( IDirect3DDevice9 * pDevice, float fApplicationTime ) {
	// Let observers know that a message was triggered
	for(appEventItem::const_iterator it = appEventObservers.begin(); it != appEventObservers.end(); ++it ) 
	{
		// Obtain a pointer to the current app event
		iAppEventObserver * appEvent = (*it);
		if( appEvent ) appEvent->Event_FrameRender( pDevice, fApplicationTime );
	}
	// Return Successfully
	return S_OK;
}

HRESULT CApplication::_triggerFramePostRender( IDirect3DDevice9 * pDevice, float fApplicationTime ) {
	// Let observers know that a message was triggered
	for(appEventItem::const_iterator it = appEventObservers.begin(); it != appEventObservers.end(); ++it ) 
	{
		// Obtain a pointer to the current app event
		iAppEventObserver * appEvent = (*it);
		if( appEvent ) appEvent->Event_FramePostRender( pDevice, fApplicationTime );
	}
	// Return Successfully
	return S_OK;
}

HRESULT CApplication::_triggerFrameOverlays( IDirect3DDevice9 * pDevice, ATG::Font& overlayFont, float fApplicationTime ) {
	// Let observers know that a message was triggered
	for(appEventItem::const_iterator it = appEventObservers.begin(); it != appEventObservers.end(); ++it ) 
	{
		// Obtain a pointer to the current app event
		iAppEventObserver * appEvent = (*it);
		if( appEvent ) appEvent->Event_FrameOverlays( pDevice, overlayFont, fApplicationTime );
	}
	// Return Successfully
	return S_OK;
}

HRESULT CApplication::_triggerTextureCreation( IDirect3DDevice9 * pDevice, float fApplicationTime ) {
	// Let observers know that a message was triggered
	for(appEventItem::const_iterator it = appEventObservers.begin(); it != appEventObservers.end(); ++it ) 
	{
		// Obtain a pointer to the current app event
		iAppEventObserver * appEvent = (*it);
		if( appEvent ) appEvent->Event_TextureCreation( pDevice, fApplicationTime );
	}
	// Return Successfully
	return S_OK;
}


HRESULT CApplication::_triggerGarbageCollection( IDirect3DDevice9 * pDevice, float fApplicationTime ) {
	// Let observers know that a message was triggered
	for(appEventItem::const_iterator it = appEventObservers.begin(); it != appEventObservers.end(); ++it ) 
	{
		// Obtain a pointer to the current app event
		iAppEventObserver * appEvent = (*it);
		if( appEvent ) appEvent->Event_GarbageCollection( pDevice, fApplicationTime );
	}
	// Return Successfully
	return S_OK;
}


HRESULT CApplication::_triggerWriteDebug() {
	// Let observers know that a message was triggered
	for(appEventItem::const_iterator it = appEventObservers.begin(); it != appEventObservers.end(); ++it ) 
	{
		// Obtain a pointer to the current app event
		iAppEventObserver * appEvent = (*it);
		if( appEvent ) appEvent->Event_WriteDebug();
	}
	// Return Successfully
	return S_OK;
}