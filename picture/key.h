#pragma once

#define KEYSTATEMASK_DOWN				0x8000
#define ISKEYDOWN(KEY)					((GetAsyncKeyState(KEY) & KEYSTATEMASK_DOWN) != 0)

#define ISSHIFTDOWN						ISKEYDOWN(VK_SHIFT)
#define ISALTDOWN						ISKEYDOWN(VK_MENU)
#define ISCONTROLDOWN					ISKEYDOWN(VK_CONTROL)

#define CAPSLOCK_ISON					((bool)(GetKeyState(VK_CAPITAL) & 0x01))
#define CAPS_STATUS_ON					(CAPSLOCK_ISON ^ ISSHIFTDOWN)



// °´¼ü×´Ì¬
#define ISCHAR(C)						(((C) <= 'Z' && (C) >= 'A') || (C) <= ('z' && (C) >= 'a'))
#define ISUPPERCHAR(C)					((C) <= 'Z' && (C) >= 'A')
#define ISLOWERCHAR(C)					((C) <= 'z' && (C) >= 'a')
#define TOUPPERCHAR_UNSAFE(C)			(C) += 'A' - 'a';
#define TOLOWERCHAR_UNSAFE(C)			(C) += 'a' - 'A';