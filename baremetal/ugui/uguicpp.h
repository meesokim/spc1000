//
// uguicpp.h
//
// C++ wrapper for uGUI with mouse and touch screen support
//
#ifndef _ugui_uguicpp_h
#define _ugui_uguicpp_h

#ifdef __cplusplus
extern "C" {
#endif
	#include <ugui/ugui.h>
#ifdef __cplusplus
}
#endif

#include "../screen8.h"
#include <circle/usb/usbmouse.h>
#include <circle/input/touchscreen.h>

#if defined (DEPTH) && (DEPTH != 8)
#if defined (DEPTH) && (DEPTH != 16)
	#error DEPTH must be set to 16 or 8 in include/circle/screen.h!
#endif 
#endif

class CUGUI
{
public:
	CUGUI (CScreenDevice8 *pScreen);
	~CUGUI (void);

	boolean Initialize (void);

	void Update (void);

private:
	static void SetPixel (UG_S16 sPosX, UG_S16 sPosY, UG_COLOR Color);

	void MouseEventHandler (TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY);
	static void MouseEventStub (TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY);

	void TouchScreenEventHandler (TTouchScreenEvent Event,
				      unsigned nID, unsigned nPosX, unsigned nPosY);
	static void TouchScreenEventStub (TTouchScreenEvent Event,
					  unsigned nID, unsigned nPosX, unsigned nPosY);

private:
	CScreenDevice8 *m_pScreen;

	UG_GUI m_GUI;

	CUSBMouseDevice *m_pMouseDevice;

	CTouchScreenDevice *m_pTouchScreen;
	unsigned m_nLastUpdate;

	static CUGUI *s_pThis;
};

#endif
