// Minimal, reusable WinMM MIDI input wrapper (implementation)

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "midiInit.h"
#include <windows.h>
#include <mmsystem.h>
#include <mutex>
#include <atomic>
#include <cstdio>
#pragma comment(lib, "winmm.lib")

struct MidiInWinMM::Impl
{
	HMIDIIN hIn = nullptr;
	std::mutex mtx;
	std::vector<MidiMessage> queue;
	std::atomic<bool> open{false};

	static void CALLBACK s_midiInProc(HMIDIIN hmi, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
	{
		if (!dwInstance) return;
		Impl *self = reinterpret_cast<Impl *>(dwInstance);
		if (wMsg == MIM_DATA)
		{
			// dwParam1: 0x00ddss (status in low byte)
			// dwParam2: timestamp (ms since midiInStart)
			uint32_t p1 = static_cast<uint32_t>(dwParam1);
			MidiMessage m;
			m.status = static_cast<uint8_t>(p1 & 0xFFu);
			m.data1 = static_cast<uint8_t>((p1 >> 8) & 0xFFu);
			m.data2 = static_cast<uint8_t>((p1 >> 16) & 0xFFu);
			m.timestampMs = static_cast<uint32_t>(dwParam2);

			// Only short messages 0x80..0xEF are interesting here
			uint8_t hi = m.status & 0xF0u;
			if (hi >= 0x80 && hi <= 0xE0)
			{
				std::lock_guard<std::mutex> lock(self->mtx);
				self->queue.push_back(m);
			}
		}
		else if (wMsg == MIM_LONGDATA)
		{
			// Ignore SysEx in this minimal wrapper
		}
	}
};

MidiInWinMM::MidiInWinMM(): impl(new Impl) {}
MidiInWinMM::~MidiInWinMM() { close(); delete impl; }

bool MidiInWinMM::open(uint32_t deviceIndex)
{
	close();
	UINT num = midiInGetNumDevs();
	if (num == 0 || deviceIndex >= num) return false;

	MMRESULT r = midiInOpen(&impl->hIn, deviceIndex,
		reinterpret_cast<DWORD_PTR>(&Impl::s_midiInProc),
		reinterpret_cast<DWORD_PTR>(impl),
		CALLBACK_FUNCTION);
	if (r != MMSYSERR_NOERROR)
	{
		impl->hIn = nullptr;
		return false;
	}
	midiInStart(impl->hIn);
	impl->open = true;
	return true;
}

void MidiInWinMM::close()
{
	if (impl->hIn)
	{
		midiInStop(impl->hIn);
		midiInReset(impl->hIn);
		midiInClose(impl->hIn);
		impl->hIn = nullptr;
	}
	{
		std::lock_guard<std::mutex> lock(impl->mtx);
		impl->queue.clear();
	}
	impl->open = false;
}

bool MidiInWinMM::isOpen() const { return impl->open.load(); }

void MidiInWinMM::poll(std::vector<MidiMessage> &out)
{
	std::lock_guard<std::mutex> lock(impl->mtx);
	if (!impl->queue.empty())
	{
		out.insert(out.end(), impl->queue.begin(), impl->queue.end());
		impl->queue.clear();
	}
}

int MidiInWinMM::deviceCount() const
{
	return static_cast<int>(midiInGetNumDevs());
}

std::string MidiInWinMM::deviceName(uint32_t index) const
{
	//MIDIINCAPS caps{};
	//if (midiInGetDevCaps(index, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
	//{
	//#ifdef UNICODE
	//	char nameA[256]{};
	//	WideCharToMultiByte(CP_UTF8, 0, caps.szPname, -1, nameA, 255, nullptr, nullptr);
	//	return std::string(nameA);
	//#else
	//	return std::string(caps.szPname);
	//#endif
	//}
	//return {};

	//no unicode
	MIDIINCAPSA caps{};
	if (midiInGetDevCapsA(index, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
	{
		return std::string(caps.szPname);
	}
	return {};
}
