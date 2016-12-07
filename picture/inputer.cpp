#include "stdafx.h"
#include "inputer.h"

void WINAPI Inputer_W::SetCallbackCMD(LPCALLBACKCMD pCallback)
{
	lpcallbackcmd = pCallback;
}
