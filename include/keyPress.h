#pragma once
#include <string>

void simulateKeyPressAndRelease(unsigned short vk);

void simulateKeyPress(unsigned short vk);

void simulateKeyRelease(unsigned short vk);


unsigned short getKeyPressed();

std::string getKeyName(unsigned short k);
