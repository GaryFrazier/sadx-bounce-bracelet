#include "stdafx.h"
#include <SADXModLoader.h>

#define EXPORT __declspec(dllexport)

FunctionPointer(int, _rand, (void), 0x006443BF);

static int   ring_timer = 0;
static int   super_count = 0; // Dirty hack for multitap mod compatibility
static short last_level = 0;
static short last_act = 0;
static int   level_song = 0;
static Uint8 last_action[8] = {};

static const int clips[] = {
	402,
	508,
	874,
	1427,
	1461
};

static void __cdecl BounceBracelet_Main(ObjectMaster* _this)
{

	// HACK: Result screen disables P1 control. There's probably a nicer way to do this, we just need to find it.
	if (IsControllerEnabled(0))
	{
		++ring_timer %= 60;

		if (!ring_timer)
		{
			AddRings(-1);
		}
	}
}

static void BounceBracelet_Bounce()
{
	PhysicsArray->AirAccel = 500;
	PhysicsArray->JumpSpeed = 500;
}

static const PVMEntry BounceBraceletPVM = { const_cast<char*>("BounceBracelet"), &SONIC_TEXLIST };

extern "C"
{
	EXPORT ModInfo SADXModInfo = { ModLoaderVer, nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0 };

	void EXPORT Init(const char* path, HelperFunctions* helper)
	{
		helper->RegisterCharacterPVM(Characters_Sonic, BounceBraceletPVM);

		// Fixes vertical offset when completing a stage
		WriteData<7>(reinterpret_cast<Uint8*>(0x00494E13), 0x90i8);

		// Fixes upside down water plane in Emerald Coast 2
		auto ec2_mesh = reinterpret_cast<LandTable*>(0x01039E9C);
		NJS_OBJECT* obj = ec2_mesh->Col[1].Model;
		obj->ang[0] = 32768;
		obj->pos[1] = -3.0f;
		obj->pos[2] = -5850.0f;

		// Always initialize Super Sonic weld data.
		WriteData<2>(reinterpret_cast<Uint8*>(0x0049AC6A), 0x90i8);
	}

	void EXPORT OnFrame()
	{

		for (int i = 0; i < 8; i++)
		{
			EntityData1* data1 = EntityData1Ptrs[i];
			CharObj2* data2 = CharObj2Ptrs[i];

			if (data1 == nullptr || data1->CharID != Characters_Sonic)
			{
				continue;
			}

			bool toggle = (ControllerPointers[i]->PressedButtons & ButtonBits_Y) != 0;
			//bool action = !is_super ? (last_action[i] == 8 && data1->Action == 12) : (last_action[i] == 82 && data1->Action == 78);

			if (toggle /*&& action*/)
			{
				BounceBracelet_Bounce();
			}
			
		}

	}
}
