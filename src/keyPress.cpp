#include <Windows.h>
#include "keyPress.h"


void simulateKeyPressAndRelease(unsigned short vk)
{
	INPUT input[2] = {};

	// Key down
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = vk;

	// Key up
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = vk;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(2, input, sizeof(INPUT));
}



void simulateKeyPress(unsigned short vk)
{
	INPUT input[1] = {};

	// Key down
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = vk;

	SendInput(1, input, sizeof(INPUT));
}

void simulateKeyRelease(unsigned short vk)
{
	INPUT input[1] = {};

	// Key up
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = vk;
	input[0].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(1, input, sizeof(INPUT));
}


// Returns the first currently pressed key (virtual keycode), or 0 if none
unsigned short getKeyPressed()
{
	for (unsigned short vk = 1; vk < 254; ++vk)
	{
		if (GetAsyncKeyState(vk) & 0x8000)
		{
			return vk;
		}
	}
	return 0;
}


// Converts a virtual keycode into a readable key name
std::string getKeyName(unsigned short vk)
{
	UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
	LONG lParam = (scanCode << 16);

	char name[128] = {};
	int len = GetKeyNameTextA(lParam, name, sizeof(name));
	if (len > 0)
	{
		return std::string(name, len);
	}
	return {};
}