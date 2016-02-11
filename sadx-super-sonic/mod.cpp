#include "stdafx.h"
#include <SADXModLoader.h>

#define EXPORT __declspec(dllexport)

DataPointer(NJS_TEXLIST, SUPERSONIC_TEXLIST, 0x0142272C);
DataPointer(bool, SuperSonicFlag, 0x03C55D45);
DataPointer(int, CurrentSong, 0x00912698);
DataPointer(int, LastSong, 0x0091269C);
DataPointer(Bool, Music_Enabled, 0x0091268C);

FunctionPointer(void, ForcePlayerAction, (Uint8 playerNum, Uint8 action), 0x00441260);

static int ring_timer = 0;
static int super_count = 0;	// Dirty hack for multitap mod compatibility
static Uint8 last_action[8] = {};

void __cdecl Sonic_SuperPhysics_Delete(ObjectMaster* _this)
{
	char index = *(char*)_this->UnknownB_ptr;
	CharObj2* data2 = CharObj2Ptrs[index];

	if (data2 != nullptr)
		data2->PhysicsData = PhysicsArray[Characters_Sonic];
}

// TODO: Figure out why this runs on the result screen
void __cdecl SuperSonicManager_Main(ObjectMaster* _this)
{
	if (super_count < 1)
	{
		DeleteObject_(_this);
		return;
	}

	if (Music_Enabled && CurrentSong != 86)
		CurrentSong = 86;

	if (IsControllerEnabled(0))
		++ring_timer %= 60;

	if (!ring_timer)
		AddRings(-1);
}
void __cdecl SuperSonicManager_Delete(ObjectMaster* _this)
{
	super_count = 0;
	ring_timer = 0;

	if (Music_Enabled)
		CurrentSong = LastSong;
}
void SuperSonicManager_Load()
{
	ObjectMaster* obj = LoadObject(0, 2, SuperSonicManager_Main);
	if (obj)
		obj->DeleteSub = SuperSonicManager_Delete;
}

static const PVMEntry SuperSonicPVM = { "SUPERSONIC", &SUPERSONIC_TEXLIST };

extern "C"
{
	void EXPORT Init(const char* path, HelperFunctions* helper)
	{
		helper->RegisterCharacterPVM(Characters_Sonic, SuperSonicPVM);
		WriteData((void**)0x004943C2, (void*)Sonic_SuperPhysics_Delete);
	}

	void EXPORT OnFrame()
	{
		if (GameState != 15)
			return;

		for (Uint8 i = 0; i < 8; i++)
		{
			CharObj1* data1 = CharObj1Ptrs[i];
			CharObj2* data2 = CharObj2Ptrs[i];

			if (data1 == nullptr || data1->CharID != Characters_Sonic)
				continue;

			bool isSuper = (data2->Upgrades & Upgrades_SuperSonic) != 0;
			bool toggle = Controllers[i].PressedButtons & Buttons_B;

			if (!isSuper)
			{
				if (toggle && last_action[i] == 8 && data1->Action == 12 && Rings >= 50)
				{
					// Transform into Super Sonic
					ForcePlayerAction(i, 46);
					data2->Upgrades |= Upgrades_SuperSonic;
					PlayVoice(396);

					if (!super_count++)
						SuperSonicManager_Load();
				}
			}
			else if (toggle && last_action[i] == 82 && data1->Action == 78 || !Rings)
			{
				// Change back to normal Sonic
				ForcePlayerAction(i, 47);
				data2->Upgrades &= ~Upgrades_SuperSonic;
				--super_count;
			}

			last_action[i] = data1->Action;
		}

		SuperSonicFlag = super_count > 0;
	}

	EXPORT ModInfo SADXModInfo = { ModLoaderVer };
}
