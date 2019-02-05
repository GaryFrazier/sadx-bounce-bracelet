#include "stdafx.h"
#include <SADXModLoader.h>

#define EXPORT __declspec(dllexport)

FunctionPointer(int, _rand, (void), 0x006443BF);

static const PVMEntry BounceBraceletPVM = { const_cast<char*>("BounceBracelet"), &SONIC_TEXLIST };
static bool bouncing = false;
static int bounceCount = 0;

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
		if (GameState != 15 || MetalSonicFlag)
		{
			bouncing = false;
			return;
		}

		for (int i = 0; i < 8; i++)
		{
			EntityData1* data1 = EntityData1Ptrs[i];
			CharObj2* data2 = CharObj2Ptrs[i];
			
			if (data1 == nullptr || data1->CharID != Characters_Sonic)
			{
				continue;
			}
			bool toggle = (ControllerPointers[i]->PressedButtons & Buttons_B) != 0;
			bool action = data1->Action == 12;

			if (bouncing) {
				if (data2->Speed.y == 0) {
					if (bounceCount > 2) {
						bounceCount = 2;
					}

					data2->Speed.y = (3 + ((bounceCount - 1)));

					bouncing = false;
				}
			}
			else if (toggle && action)
			{
				NJS_VECTOR newSpeed = NJS_VECTOR();
				newSpeed.x = data2->Speed.x;
				newSpeed.y = -2.5;
				newSpeed.z = data2->Speed.z;

				bouncing = true;
				bounceCount += 1;

				Sonic_ChargeSpindash(data2, data1);
				Sonic_ReleaseSpindash(data1, data2);

				data2->Speed = newSpeed;
			}
			else if (data2->Speed.y == 0) {
				bounceCount = 0;
			}
		}

	}
}
