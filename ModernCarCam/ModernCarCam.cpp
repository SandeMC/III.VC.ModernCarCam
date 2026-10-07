#include "common.h"
#include "MemoryMgr.h"
#include "GTA.h"
#include "Camera.h"
#include "Pad.h"

#include "ModuleList.hpp"
#include "GInputAPI.h"
#include "debugmenu_public.h"

#include "ModernCarCam.h"

// ---------------------------------------------------------------------------
// ModernCarCam - a universal vehicle camera for GTA III and GTA Vice City.
//
// With the shipped vanilla settings the vehicle camera is a 1:1 reproduction
// of the original camera of either game. Every additional behaviour is an
// ini option layered on top of that baseline.
//
// This file owns DllMain and the game hooks, the address wrappers for the
// engine calls, the shared buffer helper and the debug menu. The rest of the
// project is split by concern (see ModernCarCam.h):
//
//   Profiles.cpp  camera tables + applyProfile()
//   Settings.cpp  ini parsing + settings state
//   CamVanilla.cpp  authentic III/VC "on a string" and behind-boat cameras
//   CamSA.cpp       San Andreas follow-camera engine
//
// The authentic camera is based on the reversed sources of re3 / reVC
// (https://github.com/Hezkore/hez-gta-re3); see licenses/re3.txt.
// The VCS camera shake is ported from ThirteenAG's WidescreenFixesPack
// (MIT licensed, see licenses/WidescreenFixesPack.txt).
// ---------------------------------------------------------------------------

HMODULE dllModule, hDummyHandle;
int gtaversion = -1;

// Reminder: isVC() will also return true for Re:LCS
bool isReLCS = false;

int debugMenuLoaded = 0; // 1: not installed 2: installed
DebugMenuAPI gDebugMenuAPI;

// -----

void(*&RwCamera) = *AddressByVersion<void**>(0x72676C, 0, 0, 0x8100BC, 0, 0);

CCameraIII *TheCameraIII = (CCameraIII*)0x6FACF8;
CCameraVC *TheCameraVC = (CCameraVC*)0x7E4688;

// Actually static member of CCamera
bool& m_bUseMouse3rdPerson = *AddressByVersion<bool*>(0x5F03D8, 0, 0, 0xA10B4C, 0, 0);

cDMAudio &DMAudio = *AddressByVersion<cDMAudio*>(0x95CDBE, 0, 0, 0xA10B8A, 0, 0);

// These are static members of CWorld
CColPoint& ms_testSpherePoint = *AddressByVersion<CColPoint*>(0x6E64C0, 0, 0, 0x7D18C0, 0, 0);
CEntity*& pIgnoreEntity = *AddressByVersion<CEntity **>(0x8F6494, 0, 0, 0x9B6E58, 0, 0);

#pragma warning(push)
#pragma warning(disable: 4100) // the naked address wrappers forward every argument untouched

addr plosAddress = AddressByVersion<addr>(0x4AF970, 0, 0, 0x4D92D0, 0, 0);
WRAPPER bool CWorldIII::ProcessLineOfSight(const CVector& point1, const CVector& point2, CColPoint& point, CEntity*& entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects) { EAXJMP(plosAddress); }
WRAPPER bool CWorldVC::ProcessLineOfSight(const CVector& point1, const CVector& point2, CColPoint& point, CEntity*& entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects, bool sth) { EAXJMP(plosAddress); }

addr tsawAddress = AddressByVersion<addr>(0x4B4710, 0, 0, 0x4D3F40, 0, 0);
WRAPPER CEntity* CWorldIII::TestSphereAgainstWorld(CVector centre, float distance, CEntity* entityToIgnore, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSomeObjects) { EAXJMP(tsawAddress); }
WRAPPER CEntity* CWorldVC::TestSphereAgainstWorld(CVector centre, float distance, CEntity* entityToIgnore, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSomeObjects) { EAXJMP(tsawAddress); }

addr fgz3dAddress = AddressByVersion<addr>(0x4B3AE0, 0, 0, 0x4D53A0, 0, 0);
WRAPPER float CWorldIII::FindGroundZFor3DCoord(float x, float y, float z, bool* found) { EAXJMP(fgz3dAddress); }
WRAPPER float CWorldVC::FindGroundZFor3DCoord(float x, float y, float z, bool* found) { EAXJMP(fgz3dAddress); }

addr frz3dAddress = AddressByVersion<addr>(0x4B3B50, 0, 0, 0x4D51D0, 0, 0);
WRAPPER float CWorldIII::FindRoofZFor3DCoord(float x, float y, float z, bool* found) { EAXJMP(frz3dAddress); }
WRAPPER float CWorldVC::FindRoofZFor3DCoord(float x, float y, float z, bool* found) { EAXJMP(frz3dAddress); }

addr pvlAddress = AddressByVersion<addr>(0x4B0DE0, 0, 0, 0x4D8B00, 0, 0);
WRAPPER bool CWorldIII::ProcessVerticalLine(const CVector& origin, float distance, CColPoint& point, CEntity*& entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, CStoredCollPoly* outCollPoly) { EAXJMP(pvlAddress); }
WRAPPER bool CWorldVC::ProcessVerticalLine(const CVector& origin, float distance, CColPoint& point, CEntity*& entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, CStoredCollPoly* outCollPoly) { EAXJMP(pvlAddress); }

addr loscAddress = AddressByVersion<addr>(0x4AEAA0, 0, 0, 0x4DA560, 0, 0);
WRAPPER bool CWorldIII::GetIsLineOfSightClear(const CVector& origin, const CVector& target, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects) { EAXJMP(loscAddress); }
WRAPPER bool CWorldVC::GetIsLineOfSightClear(const CVector& origin, const CVector& target, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects) { EAXJMP(loscAddress); }

addr gwlnwAddress = AddressByVersion<addr>(0x555440, 0, 0, 0x5C2BE0, 0, 0);
WRAPPER bool CWaterLevel::GetWaterLevelNoWaves(float fX, float fY, float fZ, float* pfOutLevel) { EAXJMP(gwlnwAddress); }

addr rcsncpAddress = AddressByVersion<addr>(0x5A5070, 0, 0, 0x64A860, 0, 0);
WRAPPER void* RwCameraSetNearClipPlane(void* camera, float nearClip) { EAXJMP(rcsncpAddress); }

addr posAddress = AddressByVersion<addr>(0x57C840, 0, 0, 0x5F9DA0, 0, 0);
WRAPPER void cDMAudio::PlayOneShot(int32 audioEntity, uint16 oneShot, float volume) { EAXJMP(posAddress); }

CBaseModelInfo** CModelInfo::ms_modelInfoPtrs = AddressByVersion<CBaseModelInfo **>(0x83D408, 0, 0, 0x92D4C8, 0, 0);

addr attachAddress = AddressByVersion<addr>(0x4B8DD0, 0, 0, 0x4DFA40, 0, 0);
addr updateRwAddress = AddressByVersion<addr>(0x4B8EC0, 0, 0, 0x4DF8F0, 0, 0);
WRAPPER void CMatrix::Attach(RwMatrix* matrix, bool owner) { EAXJMP(attachAddress); }
WRAPPER void CMatrix::UpdateRW(void) { EAXJMP(updateRwAddress); }
#pragma warning(pop)

addr ditbAddress = AddressByVersion<addr>(0x48BFB0, 0, 0, 0x4A4C02, 0, 0);
void (*DebugInitTextBuffer)();

// Actually static member of CVehicle
bool &m_bDisableMouseSteering = *AddressByVersion<bool*>(0x60252C, 0, 0, 0x69C610, 0, 0);

uint32 &m_snTimeInMilliseconds = *AddressByVersion<uint32*>(0x885B48, 0, 0, 0x974B2C, 0, 0);
float &ms_fTimeStep = *AddressByVersion<float*>(0x8E2CB4, 0, 0, 0x975424, 0, 0);

bool lookingRelativelyLeft = false;
bool lookingRelativelyRight = false;

void onMasterProfileChange(void) {
	applyProfile(cameraProfile, isVC());
}

// ---------------------------------------------------------------------------
// Debug menu
// ---------------------------------------------------------------------------
void registerDebugMenu() {
	if (!debugMenuLoaded) {
		if (DebugMenuLoad()) {
			DebugMenuAddInt8("ModernCarCam", "Camera profile", (int8_t*)&cameraProfile, onMasterProfileChange, 1, 0, 8, profileNames);

			DebugMenuAddVarBool8("ModernCarCam", "Camera wobble", (int8*)&cameraWobble, nil);
			DebugMenuAddVarBool8("ModernCarCam", "Elastic string physics", (int8*)&elasticStringPhysics, nil);
			DebugMenuAddInt8("ModernCarCam", "Pitch slope tilt", (int8_t*)&pitchTilt, nil, 1, 0, 3, pitchTiltNames);
			DebugMenuAddVarBool8("ModernCarCam", "Dynamic speed FOV", (int8*)&dynamicSpeedFOV, nil);
			DebugMenuAddVarBool8("ModernCarCam", "VCS camera shake", (int8*)&vcsCamShake, nil);
			DebugMenuAddInt8("ModernCarCam", "Camera anchoring", (int8_t*)&cameraAnchoring, nil, 1, 0, 2, anchoringNames);
			DebugMenuAddVarBool8("ModernCarCam", "Vehicle-specific zoom", (int8*)&vehicleSpecificZoom, nil);
			DebugMenuAddVarBool8("ModernCarCam", "Modern turret control", (int8*)&modernTurretControl, nil);
			DebugMenuAddVarBool8("ModernCarCam", "Modern drive-by", (int8*)&modernDriveBy, nil);
			DebugMenuAddVarBool8("ModernCarCam", "Mouse free-look", (int8*)&mouseFreeLook, nil);
			DebugMenuAddVarBool8("ModernCarCam", "SA bikes cam raise with passenger", (int8*)&heightIncreaseOnBike, nil);
			DebugMenuAddVarBool8("ModernCarCam", "Fix Camera clipping through the model bug", (int8*)&fixTheBug, nil);
			DebugMenuAddVarBool8("ModernCarCam", "Don't keep camera over water", (int8*)&seeUnderwater, nil);
			debugMenuLoaded = 2;
		} else
			debugMenuLoaded = 1;
	}
	DebugInitTextBuffer();
}

// ---------------------------------------------------------------------------
// 360-degree drive-by: let the game treat a turret-relative "looking left or
// right" as a real look direction.
// ---------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4740) // the naked asm thunks intentionally flow around inline asm
namespace BetterDriveBy {

	__declspec(naked) static void LookingLeftOrRightIII()
	{
		if (TheCameraIII->Cams[TheCameraIII->ActiveCam].LookingLeft || lookingRelativelyLeft)
		{
			EAXJMP(0x564099)
		}
		else if (TheCameraIII->Cams[TheCameraIII->ActiveCam].LookingRight || lookingRelativelyRight)
		{
			EAXJMP(0x5640A6)
		}
		else
		{
			EAXJMP(0x5641D1)
		}
	}

	__declspec(naked) static void LookingLeftOrRightVC()
	{
		if (TheCameraVC->Cams[TheCameraVC->ActiveCam].LookingLeft || lookingRelativelyLeft)
		{
			EAXJMP(0x5C9880)
		}
		else if (TheCameraVC->Cams[TheCameraVC->ActiveCam].LookingRight || lookingRelativelyRight)
		{
			EAXJMP(0x5C988E)
		}
		else
		{
			EAXJMP(0x5C9893)
		}
	}

	__declspec(naked) static void LookingLeftOrRightBikesVC()
	{
		if (TheCameraVC->Cams[TheCameraVC->ActiveCam].LookingLeft || lookingRelativelyLeft)
		{
			EAXJMP(0x5C92B9)
		}
		else if (TheCameraVC->Cams[TheCameraVC->ActiveCam].LookingRight || lookingRelativelyRight)
		{
			EAXJMP(0x5C92C7)
		}
		else
		{
			EAXJMP(0x5C92CB)
		}
	}

	__declspec(naked) static void LookingLeftOrRightBoatsVC()
	{
		if (TheCameraVC->Cams[TheCameraVC->ActiveCam].LookingLeft || lookingRelativelyLeft)
		{
			EAXJMP(0x5C9610)
		}
		else if (TheCameraVC->Cams[TheCameraVC->ActiveCam].LookingRight || lookingRelativelyRight)
		{
			EAXJMP(0x5C961E)
		}
		else
		{
			EAXJMP(0x5C9623)
		}
	}
}
#pragma warning(pop)

void
CCamVC::GetVectorsReadyForRW(void)
{
	Front.Normalise();
	if (Front.x == 0.0f && Front.y == 0.0f) {
		Front.x = 0.0001f;
		Front.y = 0.0001f;
	}
	CVector right = CrossProduct(Front, CVector(0.0f, 0.0f, 1.0f));
	if (right.MagnitudeSqr() < 0.0001f)
		right = CVector(1.0f, 0.0f, 0.0f);
	else
		right.Normalise();

	CVector up0 = CrossProduct(right, Front);
	up0.Normalise();

	Up = up0 * cosf(f_Roll) - right * sinf(f_Roll);
	Up.Normalise();
}

void
CCamIII::GetVectorsReadyForRW(void)
{
	Front.Normalise();
	if (Front.x == 0.0f && Front.y == 0.0f) {
		Front.x = 0.0001f;
		Front.y = 0.0001f;
	}
	CVector right = CrossProduct(Front, CVector(0.0f, 0.0f, 1.0f));
	if (right.MagnitudeSqr() < 0.0001f)
		right = CVector(1.0f, 0.0f, 0.0f);
	else
		right.Normalise();

	CVector up0 = CrossProduct(right, Front);
	up0.Normalise();

	Up = up0 * cosf(f_Roll) - right * sinf(f_Roll);
	Up.Normalise();
}

// Needed for storing previous mode for some unknown alpha angle effect.
// Credits goes to The Hero - aap for reversing it (comments are belong to him)
#define currentMode (isIII() ? TheCameraIII->Cams[TheCameraIII->ActiveCam].Mode : TheCameraVC->Cams[TheCameraVC->ActiveCam].Mode)

int previousMode = 0;

void
WellBufferMe(float Target, float* CurrentValue, float* CurrentSpeed, float MaxSpeed, float Acceleration, bool IsAngle)
{
	// MaxSpeed is a limit of how fast the value is allowed to change. 1.0 = to Target in up to 1ms
	// Acceleration is how fast the speed will change to MaxSpeed. 1.0 = to MaxSpeed in 1ms

	float Delta = Target - *CurrentValue;

	if (IsAngle) {
		while (Delta >= PI) Delta -= 2 * PI;
		while (Delta < -PI) Delta += 2 * PI;
	}

	float TargetSpeed = Delta * MaxSpeed;
	*CurrentSpeed += Acceleration * (TargetSpeed - *CurrentSpeed) * ms_fTimeStep;

	// Clamp speed if we overshot
	if (TargetSpeed < 0.0f && *CurrentSpeed < TargetSpeed)
		* CurrentSpeed = TargetSpeed;
	else if (TargetSpeed > 0.0f && *CurrentSpeed > TargetSpeed)
		* CurrentSpeed = TargetSpeed;

	*CurrentValue += *CurrentSpeed * min(10.0f, ms_fTimeStep);

	previousMode = currentMode;
}

int16
CPad::FakeCarGunLeftRight(void)
{
	if (currentMode == MODE_CAMONASTRING)
		return 0;
	else
		return pad0.GetCarGunLeftRight();
}

int16
CPad::FakeCarGunUpDown(void)
{
	if (currentMode == MODE_CAMONASTRING)
		return 0;
	else
		return pad0.GetCarGunUpDown();
}
#undef currentMode

// ---------------------------------------------------------------------------
// DllMain: detect the running game and install the hooks.
// ---------------------------------------------------------------------------
BOOL WINAPI
DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH) {
		dllModule = hInst;

		LoadSettings();

		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)& DllMain, &hDummyHandle);

		// III
		if (*(DWORD*)0x5C1E70 == 0x53E58955) {
			InjectHook(0x456F40, &WellBufferMe, PATCH_JUMP);
			InjectHook(0x459A16, &CCamIII::Process_FollowCar_SA_III, PATCH_NOTHING);
			InjectHook(0x459A54, &CCamIII::Process_FollowCar_SA_III, PATCH_NOTHING);
			InjectHook(0x459B36, &CCamIII::Process_FollowCar_SA_III, PATCH_NOTHING);

			// Prevent game from overwriting cam->FOV to 70.0f every frame (allows dynamic FOV and custom base FOV)
			Nop(0x45b52e, 10);

			if (modernTurretControl) {
				// To block original rhino-firetruck turret movement
				InjectHook(0x52260E, &CPad::FakeCarGunUpDown, PATCH_NOTHING);
				InjectHook(0x53D628, &CPad::FakeCarGunLeftRight, PATCH_NOTHING);
				InjectHook(0x5225D2, &CPad::FakeCarGunLeftRight, PATCH_NOTHING);
			}
			if (modernDriveBy) {
				InjectHook(0x56409D, 0x5640AB, PATCH_JUMP);
				InjectHook(0x564090, BetterDriveBy::LookingLeftOrRightIII, PATCH_JUMP);
			}
		// VC
		} else if (*(DWORD*)0x667BF5 == 0xB85548EC) {

			// checks for settings file name (gta_lcs.set in this case)
			isReLCS = *(DWORD*)0x68CFCC == 0x2E73636C;

			InjectHook(0x4864E3, &WellBufferMe, PATCH_JUMP);
			InjectHook(0x483B3B, &CCamVC::Process_FollowCar_SA_VC, PATCH_NOTHING);
			InjectHook(0x483B79, &CCamVC::Process_FollowCar_SA_VC, PATCH_NOTHING);
			InjectHook(0x483C3C, &CCamVC::Process_FollowCar_SA_VC, PATCH_NOTHING);

			// Prevent game from overwriting cam->FOV to 70.0f every frame (allows dynamic FOV and custom base FOV)
			Nop(0x47fb22, 10);

			if (modernDriveBy) {
				InjectHook(0x5C9885, 0x5C9893, PATCH_JUMP);
				InjectHook(0x5C9877, BetterDriveBy::LookingLeftOrRightVC, PATCH_JUMP);
				InjectHook(0x5C92BE, 0x5C92CB, PATCH_JUMP);
				InjectHook(0x5C92B0, BetterDriveBy::LookingLeftOrRightBikesVC, PATCH_JUMP);
				InjectHook(0x5C9615, 0x5C9623, PATCH_JUMP);
				InjectHook(0x5C9607, BetterDriveBy::LookingLeftOrRightBoatsVC, PATCH_JUMP);
			}

			if (modernTurretControl) {
				// To block original rhino-firetruck turret movement
				InjectHook(0x57ABAE, &CPad::FakeCarGunUpDown, PATCH_NOTHING);
				InjectHook(0x57AB72, &CPad::FakeCarGunLeftRight, PATCH_NOTHING);
				InjectHook(0x5865B8, &CPad::FakeCarGunLeftRight, PATCH_NOTHING);
			}

			// Vehicle-specific zoom tables are only patched for non-vanilla
			// distance profiles. The vanilla profile leaves the game's table
			// untouched so the Widescreen Fix keeps control of it and the two
			// mods do not fight over the same values.
			if (distanceProfile != PROFILE_VANILLA) {
				const float* zoomTable = (distanceProfile == PROFILE_CUSTOM) ? CarZoomModesCustom :
					((distanceProfile == PROFILE_LCS) ? CarZoomModesLCS : CarZoomModesSA);
				for (int i = 0; i < 15; i++) {
					addr a = 0x68AB70 + i * sizeof(float);
					Patch(a, zoomTable[i]);
				}
			}
		}
		else return FALSE;

		InterceptCall(&DebugInitTextBuffer, registerDebugMenu, ditbAddress);
	}
	return TRUE;
}
