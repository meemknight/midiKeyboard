#pragma once
// Minimal, reusable WinMM MIDI input wrapper (header-only friendly interface)

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <cstdint>
#include <vector>
#include <string>


// Returns note name like "C4", "F#3", etc.
inline std::string noteName(int note)
{
	static const char *names[12] = {
		"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
	};
	int n = note;            // 0–127
	int octave = (n / 12) - 1; // MIDI standard: 60 = C4
	return std::string(names[n % 12]) + std::to_string(octave);
}

struct MidiMessage
{
	uint8_t  status = 0;   // 0x80..0xEF (type + channel)
	uint8_t  data1 = 0;   // note / controller / etc.
	uint8_t  data2 = 0;   // velocity / value
	uint32_t timestampMs = 0;   // from backend (ms since start)

	// --- Helpers ---
	int channel() const { return status & 0x0F; }  // 0–15
	uint8_t typeByte() const { return status & 0xF0; }

	// --- Notes ---
	bool isNoteOn()  const { return (typeByte() == 0x90) && data2 > 0; }
	bool isNoteOff() const { return (typeByte() == 0x80) || ((typeByte() == 0x90) && data2 == 0); }

	int   note()      const { return data1; }              // 0–127 (60 = Middle C)
	int   velocity()  const { return data2; }              // 0–127
	float velocityNorm() const { return data2 / 127.0f; }  // 0.0–1.0

	// Returns note name like "C4", "F#3", etc.
	std::string noteName() const
	{
		return ::noteName(note());
	}

	// --- Control Change ---
	bool  isControlChange() const { return typeByte() == 0xB0; }
	int   control()       const { return data1; }               // 0–127
	int   controlValue()  const { return data2; }               // 0–127
	float controlNorm()   const { return data2 / 127.0f; }      // 0.0–1.0

	// --- Pitch Bend ---
	bool isPitchBend() const { return typeByte() == 0xE0; }
	// 14-bit raw value: 0–16383, 8192 = center
	int   pitchBendValue() const { return (data2 << 7) | data1; }
	float pitchBendNorm()  const { return pitchBendValue() / 16383.0f; } // 0.0–1.0
	float pitchBendCentered() const { return (pitchBendValue() - 8192) / 8192.0f; } // -1.0–+1.0

	// --- Program Change ---
	bool isProgramChange() const { return typeByte() == 0xC0; }
	int  programNumber()   const { return data1; }

	// --- Aftertouch (polyphonic) ---
	bool isAftertouch() const { return typeByte() == 0xA0; }
	int  aftertouchPressure() const { return data2; }
	float aftertouchNorm()    const { return data2 / 127.0f; }

	// --- Channel Pressure ---
	bool isChannelPressure() const { return typeByte() == 0xD0; }
	int  channelPressure()   const { return data1; }
	float channelPressureNorm() const { return data1 / 127.0f; }

};


class MidiInWinMM
{
public:
	MidiInWinMM();
	~MidiInWinMM();

	// Open a MIDI input device by index (0..deviceCount-1)
	bool open(uint32_t deviceIndex = 0);

	// Close if open
	void close();

	bool isOpen() const;

	// Drain messages accumulated from the callback into 'out' and clear internal queue.
	void poll(std::vector<MidiMessage> &out);

	// Device enumeration helpers
	int deviceCount() const;
	std::string deviceName(uint32_t index) const;

private:
	struct Impl;
	Impl *impl = nullptr;
};
