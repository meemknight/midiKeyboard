#include "raylib.h"
#include <iostream>

#pragma region imgui
#include "imgui.h"
#include "rlImGui.h"
#include "imguiThemes.h"
#pragma endregion

#include <midiInit.h>
#include <keyPress.h>

#include <imfilebrowser.h>
#include <fstream>

struct Entry
{

	uint8_t  note = 0;
	std::string noteName = "";

	unsigned short keyboardMapping = 0;
	std::string keyBoardName = "";

};



int main(void)
{

	SetTraceLogLevel(LOG_NONE); // no log output to the console by raylib
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 450, "raylib [core] example - basic window");
	SetExitKey(KEY_NULL); // Disable Esc from closing window
	SetTargetFPS(120);

#pragma region imgui
	rlImGuiSetup(true);

	//you can use whatever imgui theme you like!
	//ImGui::StyleColorsDark();
	//imguiThemes::yellow();
	//imguiThemes::gray();
	imguiThemes::green();
	//imguiThemes::red();
	//imguiThemes::embraceTheDarkness();


	ImGuiIO &io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.FontGlobalScale = 3;

	ImGuiStyle &style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		//style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 0.5f;
		//style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
	}

#pragma endregion

	ImGui::FileBrowser fileDialog;
	fileDialog.SetTitle("Select File");
	fileDialog.SetTypeFilters({".txt"});

	bool openFileDialog = false;
	bool saveFileDialog = false;
	std::string resourcesPath = "";


	MidiInWinMM midiIn;
	midiIn.open();

	bool isListeningForNote = 0;
	bool isListeningForKey = 0;

	Entry entry;

	std::vector<Entry> entries;

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(RAYWHITE);

	#pragma region imgui
		rlImGuiBegin();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
		ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::PopStyleColor(2);
	#pragma endregion

	#pragma region imgui window

		// call this each frame between rlImGuiBegin()/End()
		ImGuiViewport *vp = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(vp->Pos);
		ImGui::SetNextWindowSize(vp->Size);
		ImGui::SetNextWindowViewport(vp->ID);

		// optional: edge-to-edge content
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoCollapse
			//| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoNavFocus
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoDocking;      // keeps it fixed and undockable
		// | ImGuiWindowFlags_NoBackground;    // uncomment if you want it transparent

		if (ImGui::Begin("FullScreenFixed", nullptr, flags))
		{
			ImGui::Text("Midi to keyboard");

			if (ImGui::Button("Save mappings"))
			{
				saveFileDialog = true;
				fileDialog.SetPwd(resourcesPath);
				fileDialog.Open();
			}
			ImGui::SameLine();
			if (ImGui::Button("Load mappings"))
			{
				openFileDialog = true;
				fileDialog.SetPwd(resourcesPath);
				fileDialog.Open();
			}

			fileDialog.Display();

			if (fileDialog.HasSelected())
			{
				std::string path = fileDialog.GetSelected().string();

				if (saveFileDialog)
				{
					std::ofstream out(path);
					if (out.is_open())
					{
						for (auto &e : entries)
						{
							out << (int)e.note << " " << (int)e.keyboardMapping << "\n";
						}
						out.close();
					}
					saveFileDialog = false;
				}
				else if (openFileDialog)
				{
					std::ifstream in(path);
					if (in.is_open())
					{
						entries.clear();
						int note, key;
						while (in >> note >> key)
						{
							if (note < 0 || note > 127) continue; // sanity check
							Entry e;
							e.note = (uint8_t)note;
							e.noteName = noteName(note);
							e.keyboardMapping = (unsigned short)key;
							e.keyBoardName = getKeyName(key);
							entries.push_back(e);
						}
						in.close();
					}
					openFileDialog = false;
				}

				fileDialog.ClearSelected();
			}



			if (!midiIn.isOpen())
			{
				ImGui::Text("Trying to connect...");

				static int counter = 0;
				
				if (counter <= 0)
				{
					midiIn.open();
					counter = 30;
				}
				counter--;
			}
			else
			{



				ImGui::NewLine();

				if (ImGui::Button("Disconect Midi"))
				{
					midiIn.close();
				}

				if (isListeningForKey)
				{
					ImGui::Text("Imput a key to map that midi to...");

				}else
				if (!isListeningForNote)
				{
					if (ImGui::Button("Listen for midi"))
					{
						isListeningForNote = true;
						isListeningForKey = 0;
						entry = {};
					}
				}
				else
				{
					ImGui::Text("Imput a midi key...");
				}


				ImGui::Separator();

				ImGui::BeginChild(1);

				for (int i = 0; i < entries.size(); )
				{
					ImGui::PushID(i+1); // ensure unique IDs for widgets

					ImGui::BeginChild(i+1, ImVec2(0, 180), true); // optional size/appearance
					ImGui::Text("Note: %s", entries[i].noteName.c_str());
					ImGui::Text("Map to: %s", entries[i].keyBoardName.c_str());

					if (ImGui::Button("Remove this entry"))
					{
						entries.erase(entries.begin() + i);
						ImGui::EndChild();
						ImGui::PopID();
						continue; // don't increment i, because elements shifted
					}

					ImGui::EndChild();
					ImGui::PopID();

					i++; // only increment if nothing was erased
				}

				ImGui::EndChild();
			}



		}
		ImGui::End();

		ImGui::PopStyleVar(); // if you pushed padding

	#pragma endregion


	#pragma region program logic
		
		std::vector<MidiMessage> midiMessages;
		midiIn.poll(midiMessages);

		if (isListeningForNote)
		{
		
			for (int i = 0; i < midiMessages.size(); i++)
			{

				if (midiMessages[i].isNoteOn())
				{
					entry = {};
					entry.note = midiMessages[i].note();
					entry.noteName = midiMessages[i].noteName();

					isListeningForNote = false;
					isListeningForKey = true;

					break;
				}

			}
				
		}else
		if (isListeningForKey)
		{

			unsigned short key = getKeyPressed();

			if (key)
			{
				isListeningForKey = false;
				isListeningForNote = false;
				entry.keyboardMapping = key;
				entry.keyBoardName = getKeyName(key);

				entries.push_back(entry);

			}

		}
		else
		{

			for (auto m : midiMessages)
			{

				for (auto &e : entries)
				{

					if (m.note() == e.note)
					{

						if (m.isNoteOn())
						{
							simulateKeyPress(e.keyboardMapping);
						}
						else if (m.isNoteOff())
						{
							simulateKeyRelease(e.keyboardMapping);
						}

					}
				}

			}


		}

		


	#pragma endregion


	#pragma region imgui
		rlImGuiEnd();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	#pragma endregion

		EndDrawing();
	}


#pragma region imgui
	rlImGuiShutdown();
#pragma endregion



	CloseWindow();

	return 0;
}