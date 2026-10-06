#include "common.h"
#include "MemoryMgr.h"
#include "GTA.h"
#include "Camera.h"
#include "Pad.h"

#include "ModuleList.hpp"
#include "GInputAPI.h"
#include "debugmenu_public.h"

// ---------------------------------------------------------------------------
// ModernCarCam - a universal vehicle camera for GTA III and GTA Vice City.
//
// With the shipped vanilla settings the vehicle camera is a 1:1 reproduction
// of the original camera of either game. Every additional behaviour is an
// ini option layered on top of that baseline.
//
// The authentic camera is based on the reversed sources of re3 / reVC.
// The VCS camera shake is ported from ThirteenAG's WidescreenFixesPack
// (MIT licensed, see licenses/WidescreenFixesPack.txt).
// ---------------------------------------------------------------------------

// Defined by project configuration (e.g. ReleaseLCS defines LCS_CAM).
// If defined, it compiles as LCS vehicle camera (LCSCarCam); otherwise as SA vehicle camera (SACarCam).
//#define LCS_CAM

#define DefaultFOV 70.0f
#define DefaultNearClip 0.9f

HMODULE dllModule, hDummyHandle;
int gtaversion = -1;

// Reminder: isVC() will also return true for Re:LCS
bool isReLCS = false;

#define MI_III_YARDIE 135
#define MI_III_RCBANDIT 131
#define MI_III_RHINO 122
#define MI_III_DODO 126
#define MI_III_FIRETRUCK 97

#define MI_VC_VOODOO 142
#define MI_VC_FIRETRUCK 137
#define MI_VC_RHINO 162
#define MI_VC_RCBANDIT 171
#define MI_VC_RCBARON 194
#define MI_VC_RCRAIDER 195
#define MI_VC_RCGOBLIN 231

#define MI_RELCS_FIRETRUCK 138
#define MI_RELCS_RHINO 162
#define MI_RELCS_DODO 164
#define MI_RELCS_RCBANDIT 169 // original LCS didn't use this though, RC vehicles have same camera distance as normal veh - Ryadica926
#define MI_RELCS_YARDIE 173
#define MI_RELCS_RCGOBLIN 211 // same as rc bandit
#define MI_RELCS_RCRAIDER 212 // same as above

#define Tank (isIII() ? MI_III_RHINO : (isReLCS ? MI_RELCS_RHINO : MI_VC_RHINO))
#define FireTruk (isIII() ? MI_III_FIRETRUCK : (isReLCS ? MI_RELCS_FIRETRUCK : MI_VC_FIRETRUCK))
#define CarWithHydraulics (isIII() ? MI_III_YARDIE : (isReLCS ? MI_RELCS_YARDIE : MI_VC_VOODOO))
#define RcBandit (isIII() ? MI_III_RCBANDIT : (isReLCS ? MI_RELCS_RCBANDIT : MI_VC_RCBANDIT))

// These are being used only if isVC() is true
#define RcGoblin (isReLCS ? MI_RELCS_RCGOBLIN : MI_VC_RCGOBLIN)
#define RcRaider (isReLCS ? MI_RELCS_RCRAIDER : MI_VC_RCRAIDER)

IGInputPad* ginputPad;
int ginputLoaded = 0; // 1: not installed 2: installed
int debugMenuLoaded = 0; // 1: not installed 2: installed
GINPUT_PAD_SETTINGS padSettings = {};
DebugMenuAPI gDebugMenuAPI;

// Original values from SA / LCS / VC / III
const float CarZoomModesSA[] = {
	-1.0f, -0.2f, -3.2f, 0.05f, -2.41f, // near
	1.0f, 1.4f, 0.65f, 1.9f, 6.49f, // mid
	6.0f, 6.0f, 15.9f, 15.9f, 15.0f // far
};

const float CarZoomModesLCS[] = {
	-1.0f, -0.2f, -3.2f, 0.05f, -2.41f, // near
	2.0f, 2.2f, 1.65f, 2.9f, 6.49f, // mid
	6.0f, 6.0f, 15.9f, 15.9f, 15.0f // far
};

const float CarZoomModesVC[] = {
	-0.6f, 0.05f, -3.2f, 0.05f, -2.41f, // near
	1.9f, 1.4f, 0.65f, 1.9f, 6.49f,     // mid
	15.9f, 15.9f, 15.9f, 15.9f, 25.25f  // far
};

const float CarZoomModesIII[] = {
	0.05f, 0.05f, 0.05f, 0.05f, 0.05f, // near
	1.9f, 1.9f, 1.9f, 1.9f, 1.9f,       // mid
	3.9f, 3.9f, 3.9f, 3.9f, 3.9f        // far
};

// Alpha angles (up-down)
const float ZmOneAlphaOffset[] = { 0.08f, 0.08f, 0.15f, 0.08f, 0.08f };
const float ZmTwoAlphaOffset[] = { 0.07f, 0.08f, 0.3f, 0.08f, 0.08f };
const float ZmThreeAlphaOffset[] = { 0.055f, 0.05f, 0.15f, 0.06f, 0.08f };

const float ZmOneAlphaOffsetLCS[] = { 0.12f, 0.08f, 0.15f, 0.08f, 0.08f };
const float ZmTwoAlphaOffsetLCS[] = { 0.1f, 0.08f, 0.3f, 0.08f, 0.08f };
const float ZmThreeAlphaOffsetLCS[] = { 0.065f, 0.05f, 0.15f, 0.06f, 0.08f };

const float ZmOneAlphaOffsetVC[]   = { -0.01f, 0.10f, 0.125f, -0.10f, -0.06f }; // near (authentic Vice City)
const float ZmTwoAlphaOffsetVC[]   = {  0.045f, 0.12f, 0.045f,  0.045f, -0.035f }; // mid (authentic Vice City)
const float ZmThreeAlphaOffsetVC[] = {  0.005f, 0.005f, 0.15f,  0.005f,  0.12f }; // far (authentic Vice City)

const float ZmOneAlphaOffsetIII[]   = { -0.01f, 0.10f, 0.125f, -0.10f, -0.06f }; // near (authentic GTA III)
const float ZmTwoAlphaOffsetIII[]   = {  0.045f, 0.12f, 0.045f,  0.045f, -0.035f }; // mid (authentic GTA III)
const float ZmThreeAlphaOffsetIII[] = {  0.005f, 0.005f, 0.15f,  0.005f,  0.12f }; // far (authentic GTA III)

const float CARCAM_SET_SA[][15] = {
	{1.3f, 1.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.8f, 0.785398f, 1.5533431f},
	{1.1f, 1.0f, 0.1f, 10.0f, 11.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.75f, 0.78539819f, 1.5533431f},
	{1.1f, 1.0f, 0.2f, 10.0f, 15.0f, 0.05f, 0.05f, 0.0f, 0.9f, 0.05f, 0.01f, 0.05f, 1.0f, 0.17453294f, 1.2217305f},
	{1.1f, 3.5f, 0.2f, 10.0f, 25.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 1.5533431f, 1.5533431f},
	{1.3f, 1.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 0.0f, 0.9f, 0.05f, 0.005f, 0.05f, 1.0f, 0.34906587f, 1.2217305f},
	{1.1f, 1.0f, 0.2f, 10.0f, 5.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 0.78539819f, 1.5533431f}, // rc cars
	{1.1f, 1.0f, 0.2f, 10.0f, 5.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 0.34906587f, 1.2217305f}, // rc heli/planes
	{1.3f, 1.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.8f, 0.785398f, 1.5533431f}  // firetruck
};

const float CARCAM_SET_LCS[][15] = {
	{1.3f, 1.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.8f, 0.7854f, 1.5533f}, // cars
	{1.1f, 1.0f, 0.1f, 10.0f, 11.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.75f, 0.7854f, 1.5533f}, // bike
	{1.1f, 1.0f, 0.2f, 10.0f, 15.0f, 0.05f, 0.05f, 0.0f, 0.9f, 0.05f, 0.01f, 0.05f, 1.0f, 0.17453294f, 1.2217305f}, // heli (SA values)
	{1.1f, 3.5f, 0.2f, 10.0f, 25.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 1.5533431f, 1.5533431f}, // plane (SA values)
	{0.9f, 1.0f, 0.1f, 10.0f, 15.0f, 0.5f, 1.0f, 0.0f, 0.9f, 0.05f, 0.005f, 0.05f, 1.0f, -0.2f, 1.2217305f}, // boat
	{1.1f, 1.0f, 0.2f, 10.0f, 5.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 0.7854f, 1.5533f}, // rc cars
	{1.1f, 1.0f, 0.2f, 10.0f, 5.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 0.34906587f, 1.2217305f}, // rc heli/planes
	{1.3f, 1.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.8f, -0.18f, 0.7f}, // firetruck...
};

const float CARCAM_SET_VANILLA[][15] = {
	{1.3f, 0.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.8f, 0.785398f, 1.5533431f}, // cars
	{1.1f, 0.0f, 0.1f, 10.0f, 11.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.75f, 0.785398f, 1.5533431f}, // bikes
	{1.1f, 0.0f, 0.2f, 10.0f, 15.0f, 0.05f, 0.05f, 0.0f, 0.9f, 0.05f, 0.01f, 0.05f, 1.0f, 0.17453294f, 1.2217305f}, // helis
	{1.1f, 0.0f, 0.2f, 10.0f, 25.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 1.5533431f, 1.5533431f}, // planes
	{1.3f, 0.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 0.0f, 0.9f, 0.05f, 0.005f, 0.05f, 1.0f, 0.34906587f, 1.2217305f}, // boats
	{1.1f, 0.0f, 0.2f, 10.0f, 5.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 0.78539819f, 1.5533431f}, // rc cars
	{1.1f, 0.0f, 0.2f, 10.0f, 5.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 0.34906587f, 1.2217305f}, // rc heli
	{1.3f, 0.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.8f, 0.785398f, 1.5533431f}  // firetruck
};

float CARCAM_SET_CUSTOM[8][15] = {
	{1.3f, 0.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.8f, 0.785398f, 1.5533431f}, // cars
	{1.1f, 0.0f, 0.1f, 10.0f, 11.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.75f, 0.785398f, 1.5533431f}, // bikes
	{1.1f, 0.0f, 0.2f, 10.0f, 15.0f, 0.05f, 0.05f, 0.0f, 0.9f, 0.05f, 0.01f, 0.05f, 1.0f, 0.17453294f, 1.2217305f}, // helis
	{1.1f, 0.0f, 0.2f, 10.0f, 25.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 1.5533431f, 1.5533431f}, // planes
	{1.3f, 0.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 0.0f, 0.9f, 0.05f, 0.005f, 0.05f, 1.0f, 0.34906587f, 1.2217305f}, // boats
	{1.1f, 0.0f, 0.2f, 10.0f, 5.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 0.78539819f, 1.5533431f}, // rc cars
	{1.1f, 0.0f, 0.2f, 10.0f, 5.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, 0.34906587f, 1.2217305f}, // rc heli
	{1.3f, 0.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.8f, 0.785398f, 1.5533431f}  // firetruck
};

float CarZoomModesCustom[15] = {
	0.05f, 0.05f, 0.05f, 0.05f, 0.05f, // near
	1.9f, 1.9f, 1.9f, 1.9f, 1.9f,       // mid
	3.9f, 3.9f, 3.9f, 3.9f, 3.9f        // far
};

float ZmOneAlphaOffsetCustom[5]   = { -0.01f, 0.10f, 0.125f, -0.10f, -0.06f };
float ZmTwoAlphaOffsetCustom[5]   = {  0.045f, 0.12f, 0.045f,  0.045f, -0.035f };
float ZmThreeAlphaOffsetCustom[5] = {  0.005f, 0.005f, 0.15f,  0.005f,  0.12f };

enum CameraProfileType : int8_t {
	PROFILE_SA = 0,
	PROFILE_LCS = 1,
	PROFILE_VANILLA = 2,
	PROFILE_CUSTOM = 3
};

// Feature and debug menu toggles
CameraProfileType masterProfile = PROFILE_VANILLA;
CameraProfileType distanceProfile = PROFILE_VANILLA;
CameraProfileType fovProfile = PROFILE_VANILLA;
CameraProfileType anglesProfile = PROFILE_VANILLA;

bool cameraWobble = true;
bool elasticStringPhysics = true;
int  pitchTilt = 2; // 0 = disabled (authentic III), 1 = authentic VC (downhill only), 2 = match game (1 in VC, 0 in III), 3 = full symmetric tilt
bool dynamicSpeedFOV = false;
bool vcsCamShake = false;
int  cameraAnchoring = 2; // 0 = SA velocity follow, 1 = authentic III/VC rigid anchor, 2 = match profile
float cameraStiffness = -1.0f; // -1: match profile (1.0 for Vanilla, 0.25 for SA, 0.4 for LCS)
bool vehicleSpecificZoom = true;
bool modernTurretControl = true;
bool modernDriveBy = true;
bool mouseFreeLook = true;
bool heightIncreaseOnBike = true;
bool fixTheBug = true;
bool seeUnderwater = false;

// Custom profile parameters (loaded from [CustomProfile] in ini)
float customDistNear = 0.05f;
float customDistMid = 1.9f;
float customDistFar = 3.9f;
float customDistOffset = 0.0f;
float customMinDistance = 10.0f;

float customBaseFOV = 70.0f;
float customDynamicFOVMax = 30.0f;
float customDynamicFOVStartSpeed = 0.4f;

float customAngleNear = -0.01f;
float customAngleMid = 0.045f;
float customAngleFar = 0.005f;
float customMaxElevationAngle = 0.785398f;
float customMinElevationAngle = 1.5533431f;

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

addr ditbAddress = AddressByVersion<addr>(0x48BFB0, 0, 0, 0x4A4C02, 0, 0);
void (*DebugInitTextBuffer)();

// Actually static member of CVehicle
bool &m_bDisableMouseSteering = *AddressByVersion<bool*>(0x60252C, 0, 0, 0x69C610, 0, 0);

uint32 &m_snTimeInMilliseconds = *AddressByVersion<uint32*>(0x885B48, 0, 0, 0x974B2C, 0, 0);
float &ms_fTimeStep = *AddressByVersion<float*>(0x8E2CB4, 0, 0, 0x975424, 0, 0);

#define RwFrameGetMatrix(frame) (RwMatrix*)((addr)frame + 0x10)
#define GetVehicleComponent(car, comp) *(void**)((addr)car + (isIII() ? 0x37C : 0x394) + comp*4) // In CAutomobile. normally returns RwFrame*

#define GetDisablePlayerControls(pad) *((uint8*)((addr)pad + (isIII() ? 0xDF : 0xF0)))
#define GetHandlingFlags(veh) *((uint32*)((addr)veh->pHandling + (isIII() ? 0xC8 : 0xCC)))
#define GetWheelsOnGround(veh) *((uint8*)((addr)veh + (isIII() ? 0x590 : 0x5C4))) // In CAutomobile
#define GetMysteriousWheelRelatedThingBike(veh) *((uint8*)((addr)veh + 0x4DC)) // In CBike, VC
#define GetDoomAnglePtrLR(veh) (float*)((addr)veh + (isIII() ? 0x580 : 0x5B0)) // In CAutomobile
#define GetDoomAnglePtrUD(veh) (float*)((addr)veh + (isIII() ? 0x584 : 0x5B4)) // In CAutomobile
#define GetPedObjective(ped) *((uint32*)((addr)ped + (isIII() ? 0x164 : 0x160)))
#define GetNearPlane() *(float*)((addr)RwCamera + 0x80)

// Virtual func. in GTA
#define GetHeightAboveRoad(veh, classToCast) (veh->IsCar() ? *((float*)((addr)veh + (isIII() ? 0x50C : 0x530))) : \
															-1.0f * ((classToCast*)veh->GetColModel())->boundingBox.min.z)

float aspectRatio;
bool gotTheAR = false;
float GetAspectRatio() {
	if (isVC())
		return *(float*)0x94DD38; // CDraw::ms_fAspectRatio
	else {
		if (!gotTheAR) {
			gotTheAR = true;

			RsGlobalType& RsGlobal = *(RsGlobalType*)0x8F4360;
			return aspectRatio = ((float)RsGlobal.width) / ((float)RsGlobal.height);
		} else
			return aspectRatio;
	}
}

float GetMouseAccel(CCameraVC *camera) {
	return *(float*)0x94DBD0; // CCamera::m_fMouseAccelHorzntl
}

float GetMouseAccel(CCameraIII *camera) {
	return camera->m_fMouseAccelHorzntl;
}

float GetATanOfXY(float x, float y) {
	if (x == 0.0f && y == 0.0f)
		return 0.0f;

	float xabs = fabsf(x);
	float yabs = fabsf(y);

	if (xabs < yabs) {
		if (y > 0.0f) {
			if (x > 0.0f)
				return 0.5f * PI - atan2f(x / y, 1.0f);
			else
				return 0.5f * PI + atan2f(-x / y, 1.0f);
		}
		else {
			if (x > 0.0f)
				return 1.5f * PI + atan2f(x / -y, 1.0f);
			else
				return 1.5f * PI - atan2f(-x / -y, 1.0f);
		}
	}
	else {
		if (y > 0.0f) {
			if (x > 0.0f)
				return atan2f(y / x, 1.0f);
			else
				return PI - atan2f(y / -x, 1.0f);
		}
		else {
			if (x > 0.0f)
				return 2.0f * PI - atan2f(-y / x, 1.0f);
			else
				return PI + atan2f(-y / -x, 1.0f);
		}
	}
}

const CVector
Multiply3x3(const CMatrix& mat, const CVector& vec)
{
	return CVector(
		mat.m_matrix.right.x * vec.x + mat.m_matrix.up.x * vec.y + mat.m_matrix.at.x * vec.z,
		mat.m_matrix.right.y * vec.x + mat.m_matrix.up.y * vec.y + mat.m_matrix.at.y * vec.z,
		mat.m_matrix.right.z * vec.x + mat.m_matrix.up.z * vec.y + mat.m_matrix.at.z * vec.z);
}

const CVector
Multiply3x3(const CVector& vec, const CMatrix& mat)
{
	return CVector(
		mat.m_matrix.right.x * vec.x + mat.m_matrix.right.y * vec.y + mat.m_matrix.right.z * vec.z,
		mat.m_matrix.up.x * vec.x + mat.m_matrix.up.y * vec.y + mat.m_matrix.up.z * vec.z,
		mat.m_matrix.at.x * vec.x + mat.m_matrix.at.y * vec.y + mat.m_matrix.at.z * vec.z);
}

void WellBufferMe(float Target, float* CurrentValue, float* CurrentSpeed, float MaxSpeed, float Acceleration, bool IsAngle);

int previousMode = 0;

static void OnGInputSettingsReload()
{
	padSettings.cbSize = sizeof(padSettings);
	ginputPad->SendConstEvent(GINPUT_EVENT_FETCH_PAD_SETTINGS, &padSettings);
}

void LoadSettings()
{
	char iniPath[MAX_PATH];
	GetModuleFileNameA(dllModule, iniPath, MAX_PATH);
	char* dot = strrchr(iniPath, '.');
	if (dot) strcpy(dot, ".ini");

	if (GetFileAttributesA(iniPath) == INVALID_FILE_ATTRIBUTES) {
		if (GetFileAttributesA(".\\ModernCarCam.ini") != INVALID_FILE_ATTRIBUTES) {
			strcpy(iniPath, ".\\ModernCarCam.ini");
		} else if (GetFileAttributesA(".\\scripts\\ModernCarCam.ini") != INVALID_FILE_ATTRIBUTES) {
			strcpy(iniPath, ".\\scripts\\ModernCarCam.ini");
		} else if (GetFileAttributesA(".\\SACarCam.ini") != INVALID_FILE_ATTRIBUTES) {
			strcpy(iniPath, ".\\SACarCam.ini");
		} else if (GetFileAttributesA(".\\scripts\\SACarCam.ini") != INVALID_FILE_ATTRIBUTES) {
			strcpy(iniPath, ".\\scripts\\SACarCam.ini");
		}
	}

	char profile[32] = { 0 };
	GetPrivateProfileStringA("General", "CameraProfile", "Vanilla", profile, sizeof(profile), iniPath);
	if (_stricmp(profile, "LCS") == 0) {
		masterProfile = PROFILE_LCS;
	} else if (_stricmp(profile, "SA") == 0) {
		masterProfile = PROFILE_SA;
	} else if (_stricmp(profile, "Custom") == 0) {
		masterProfile = PROFILE_CUSTOM;
	} else if (_stricmp(profile, "Vanilla") == 0 || _stricmp(profile, "Original") == 0 || _stricmp(profile, "VC") == 0 || _stricmp(profile, "III") == 0) {
		masterProfile = PROFILE_VANILLA;
	} else {
		char moduleName[MAX_PATH];
		GetModuleFileNameA(dllModule, moduleName, MAX_PATH);
		if (strstr(moduleName, "LCS") != nullptr || strstr(moduleName, "lcs") != nullptr) {
			masterProfile = PROFILE_LCS;
		} else if (strstr(moduleName, "SA") != nullptr || strstr(moduleName, "sa") != nullptr) {
			masterProfile = PROFILE_SA;
		} else {
			masterProfile = PROFILE_VANILLA;
		}
	}

	distanceProfile = masterProfile;
	fovProfile = masterProfile;
	anglesProfile = masterProfile;

	auto ParseProfileString = [](const char* str, CameraProfileType defaultProfile) -> CameraProfileType {
		if (!str || !*str) return defaultProfile;
		if (_stricmp(str, "SA") == 0 || strcmp(str, "0") == 0)
			return PROFILE_SA;
		if (_stricmp(str, "LCS") == 0 || strcmp(str, "1") == 0)
			return PROFILE_LCS;
		if (_stricmp(str, "Custom") == 0 || strcmp(str, "3") == 0)
			return PROFILE_CUSTOM;
		if (_stricmp(str, "Vanilla") == 0 || _stricmp(str, "Original") == 0 || 
		    _stricmp(str, "VC") == 0 || _stricmp(str, "III") == 0 || strcmp(str, "2") == 0)
			return PROFILE_VANILLA;
		return defaultProfile;
	};

	char distBuf[32] = { 0 };
	GetPrivateProfileStringA("General", "DistanceProfile", "", distBuf, sizeof(distBuf), iniPath);
	if (!distBuf[0]) GetPrivateProfileStringA("General", "Distance", "", distBuf, sizeof(distBuf), iniPath);
	if (!distBuf[0]) GetPrivateProfileStringA("Features", "DistanceProfile", "", distBuf, sizeof(distBuf), iniPath);
	if (!distBuf[0]) GetPrivateProfileStringA("Features", "Distance", "", distBuf, sizeof(distBuf), iniPath);
	distanceProfile = ParseProfileString(distBuf, masterProfile);

	char fovBuf[32] = { 0 };
	GetPrivateProfileStringA("General", "FOVProfile", "", fovBuf, sizeof(fovBuf), iniPath);
	if (!fovBuf[0]) GetPrivateProfileStringA("General", "FOV", "", fovBuf, sizeof(fovBuf), iniPath);
	if (!fovBuf[0]) GetPrivateProfileStringA("Features", "FOVProfile", "", fovBuf, sizeof(fovBuf), iniPath);
	if (!fovBuf[0]) GetPrivateProfileStringA("Features", "FOV", "", fovBuf, sizeof(fovBuf), iniPath);
	fovProfile = ParseProfileString(fovBuf, masterProfile);

	char anglesBuf[32] = { 0 };
	GetPrivateProfileStringA("General", "AnglesProfile", "", anglesBuf, sizeof(anglesBuf), iniPath);
	if (!anglesBuf[0]) GetPrivateProfileStringA("General", "Angles", "", anglesBuf, sizeof(anglesBuf), iniPath);
	if (!anglesBuf[0]) GetPrivateProfileStringA("Features", "AnglesProfile", "", anglesBuf, sizeof(anglesBuf), iniPath);
	if (!anglesBuf[0]) GetPrivateProfileStringA("Features", "Angles", "", anglesBuf, sizeof(anglesBuf), iniPath);
	anglesProfile = ParseProfileString(anglesBuf, masterProfile);

	// Backward compatibility with legacy ini keys
	int vanCam = GetPrivateProfileIntA("Features", "VanillaCamera", -1, iniPath);
	if (vanCam == 1) {
		distanceProfile = PROFILE_VANILLA;
		fovProfile = PROFILE_VANILLA;
		anglesProfile = PROFILE_VANILLA;
	}
	int vDist = GetPrivateProfileIntA("Features", "VanillaDistance", -1, iniPath);
	if (vDist == 1) distanceProfile = PROFILE_VANILLA;
	int vFOV = GetPrivateProfileIntA("Features", "VanillaFOV", -1, iniPath);
	if (vFOV == 1) fovProfile = PROFILE_VANILLA;
	int vAngles = GetPrivateProfileIntA("Features", "VanillaAngles", -1, iniPath);
	if (vAngles == 1) anglesProfile = PROFILE_VANILLA;

	auto ReadFeature = [&](const char* key, int defaultVal, bool vcFit, bool iiiFit) -> bool {
		int val = GetPrivateProfileIntA("Features", key, defaultVal, iniPath);
		if (val == 2) {
			return isVC() ? vcFit : iiiFit;
		}
		return val != 0;
	};

	auto ReadFloat = [&](const char* sec, const char* key, float def) -> float {
		char buf[32] = { 0 };
		GetPrivateProfileStringA(sec, key, "", buf, sizeof(buf), iniPath);
		return buf[0] ? (float)atof(buf) : def;
	};

	// Custom profile parameters
	customDistNear = ReadFloat("CustomProfile", "CustomDistanceNear", 0.05f);
	customDistMid  = ReadFloat("CustomProfile", "CustomDistanceMid", 1.9f);
	customDistFar  = ReadFloat("CustomProfile", "CustomDistanceFar", 3.9f);
	customDistOffset = ReadFloat("CustomProfile", "CustomDistanceOffset", 0.0f);
	customMinDistance = ReadFloat("CustomProfile", "CustomMinDistance", 10.0f);

	customBaseFOV = ReadFloat("CustomProfile", "CustomBaseFOV", 70.0f);
	customDynamicFOVMax = ReadFloat("CustomProfile", "CustomMaxDynamicFOV", 30.0f);
	customDynamicFOVStartSpeed = ReadFloat("CustomProfile", "CustomDynamicFOVStartSpeed", 0.4f);

	customAngleNear = ReadFloat("CustomProfile", "CustomAngleNear", -0.01f);
	customAngleMid  = ReadFloat("CustomProfile", "CustomAngleMid", 0.045f);
	customAngleFar  = ReadFloat("CustomProfile", "CustomAngleFar", 0.005f);
	customMaxElevationAngle = ReadFloat("CustomProfile", "CustomMaxElevationAngle", 0.785398f);
	customMinElevationAngle = ReadFloat("CustomProfile", "CustomMinElevationAngle", 1.5533431f);

	for (int i = 0; i < 8; i++) {
		CARCAM_SET_CUSTOM[i][1] = customDistOffset;
		CARCAM_SET_CUSTOM[i][4] = customMinDistance;
		CARCAM_SET_CUSTOM[i][13] = customMaxElevationAngle;
		CARCAM_SET_CUSTOM[i][14] = customMinElevationAngle;
	}
	for (int i = 0; i < 5; i++) {
		CarZoomModesCustom[i] = customDistNear;
		CarZoomModesCustom[i + 5] = customDistMid;
		CarZoomModesCustom[i + 10] = customDistFar;
		ZmOneAlphaOffsetCustom[i] = customAngleNear;
		ZmTwoAlphaOffsetCustom[i] = customAngleMid;
		ZmThreeAlphaOffsetCustom[i] = customAngleFar;
	}

	// Game-specific features default to "match game" (2) so that the shipped
	// vanilla profile reproduces the original camera exactly: enabled in Vice
	// City, disabled in GTA III where the original did not have them.
	cameraWobble = ReadFeature("CameraWobble", 2, true, false);
	elasticStringPhysics = ReadFeature("ElasticStringPhysics", 0, true, true);
	pitchTilt = GetPrivateProfileIntA("Features", "PitchTilt", 2, iniPath);
	dynamicSpeedFOV = ReadFeature("DynamicSpeedFOV", 0, false, false);
	vcsCamShake = ReadFeature("VCSCamShake", 0, false, false);
	cameraAnchoring = GetPrivateProfileIntA("Features", "CameraAnchoring", 2, iniPath);
	cameraStiffness = ReadFloat("Features", "CameraStiffness", -1.0f);
	vehicleSpecificZoom = ReadFeature("VehicleSpecificZoom", 2, true, false);
	modernTurretControl = ReadFeature("ModernTurretControl", 1, true, true);
	modernDriveBy = ReadFeature("ModernDriveBy", 1, true, true);
	mouseFreeLook = ReadFeature("MouseFreeLook", 1, true, true);
	fixTheBug = ReadFeature("FixCameraClip", 1, true, true);

	int keepWater = GetPrivateProfileIntA("Features", "KeepCameraOverWater", 1, iniPath);
	seeUnderwater = (keepWater == 0);

	heightIncreaseOnBike = ReadFeature("BikesHeightIncrease", 2, true, false);
}

void onMasterProfileChange(void) {
	distanceProfile = masterProfile;
	fovProfile = masterProfile;
	anglesProfile = masterProfile;
}

const char *profileNames[] = { "SA", "LCS", "Vanilla", "Custom" };
const char *anchoringNames[] = { "Disabled (SA float)", "Enabled (Rigid anchor)", "Match profile" };
const char *pitchTiltNames[] = { "Disabled (Flat/III)", "Authentic VC (Downhill)", "Match game (Auto)", "Full symmetric" };

void registerDebugMenu() {
	if (!debugMenuLoaded) {
		if (DebugMenuLoad()) {
			DebugMenuAddInt8("ModernCarCam", "Master camera profile", (int8_t*)&masterProfile, onMasterProfileChange, 1, 0, 3, profileNames);
			DebugMenuAddInt8("ModernCarCam", "Distance profile", (int8_t*)&distanceProfile, nil, 1, 0, 3, profileNames);
			DebugMenuAddInt8("ModernCarCam", "FOV profile", (int8_t*)&fovProfile, nil, 1, 0, 3, profileNames);
			DebugMenuAddInt8("ModernCarCam", "Angles profile", (int8_t*)&anglesProfile, nil, 1, 0, 3, profileNames);

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

bool lookingRelativelyLeft = false;
bool lookingRelativelyRight = false;

const float TiltOverShoot[] = { 1.05f, 1.05f, 0.0f, 0.0f, 1.0f };
const float TiltTopSpeed[]  = { 0.035f, 0.035f, 0.001f, 0.005f, 0.035f };
const float TiltSpeedStep[] = { 0.016f, 0.016f, 0.0002f, 0.0014f, 0.016f };

inline float LimitRadianAngle(float angle) {
	while (angle >= PI) angle -= TWOPI;
	while (angle < -PI) angle += TWOPI;
	return angle;
}

static bool IsVehicleSuspensionHigh(CCameraVC* camera) { return camera->m_bVehicleSuspenHigh; }
static bool IsVehicleSuspensionHigh(CCameraIII*) { return false; }

// ---------------------------------------------------------------------------
// Vanilla "behind boat" camera (CCam::Process_BehindBoat).
// GTA III and Vice City share this implementation, so it is reproduced here
// as part of the vanilla preset.
// ---------------------------------------------------------------------------
template<class CamClass, class CameraClass, class VehicleClass, class WorldClass>
void
Process_BehindBoat_Vanilla(CameraClass* TheCamera, CamClass* cam, VehicleClass* car, const CVector& CameraTarget, float TargetOrientation)
{
	static CColPoint colPoint;
	static float TargetWhenChecksWereOn = 0.0f;
	static float CenterObscuredWhenChecksWereOn = 0.0f;
	static const float WaterZAddition = 2.75f;
	static const float FixerForGoingBelowGround = 0.4f;
	static const float AmountUp = 2.2f;

	if (!car->IsVehicle()) {
		cam->ResetStatics = false;
		return;
	}

	CVector TargetCoors = CameraTarget;
	float DeltaBeta = 0.0f;
	float WaterLevel = 0.0f;
	float s, c;

	cam->Beta = GetATanOfXY(TargetCoors.x - cam->Source.x, TargetCoors.y - cam->Source.y);
	cam->FOV = DefaultFOV;

	if (cam->ResetStatics) {
		CenterObscuredWhenChecksWereOn = 0.0f;
		TargetWhenChecksWereOn = 0.0f;
		cam->Beta = TargetOrientation + PI;
	}

	CWaterLevel::GetWaterLevelNoWaves(TargetCoors.x, TargetCoors.y, TargetCoors.z, &WaterLevel);
	WaterLevel += WaterZAddition;
	if (-FixerForGoingBelowGround < TargetCoors.z - WaterLevel)
		WaterLevel += TargetCoors.z - WaterLevel - FixerForGoingBelowGround;

	bool obscured;
	if (cam->m_bCollisionChecksOn || cam->ResetStatics) {
		const float zoom = TheCamera->CarZoomValueSmooth;
		CVector testPoint;

		c = cosf(TargetOrientation); s = sinf(TargetOrientation);
		testPoint = zoom * CVector(-c, -s, 0.0f) + (zoom + 7.0f) * CVector(-c, -s, 0.0f) + TargetCoors;
		testPoint.z = WaterLevel + zoom;
		const bool test1 = WorldClass::GetIsLineOfSightClear(testPoint, TargetCoors, true, false, false, true, false, true, true);

		c = cosf(TargetOrientation + 0.8f); s = sinf(TargetOrientation + DEGTORAD(40.0f));
		testPoint = zoom * CVector(-c, -s, 0.0f) + (zoom + 7.0f) * CVector(-c, -s, 0.0f) + TargetCoors;
		testPoint.z = WaterLevel + zoom;
		const bool test2 = WorldClass::GetIsLineOfSightClear(testPoint, TargetCoors, true, false, false, true, false, true, true);

		c = cosf(TargetOrientation - 0.8f); s = sinf(TargetOrientation - DEGTORAD(40.0f));
		testPoint = zoom * CVector(-c, -s, 0.0f) + (zoom + 7.0f) * CVector(-c, -s, 0.0f) + TargetCoors;
		testPoint.z = WaterLevel + zoom;
		const bool test3 = WorldClass::GetIsLineOfSightClear(testPoint, TargetCoors, true, false, false, true, false, true, true);

		if (!test2) {
			DeltaBeta = TargetOrientation - cam->Beta - DEGTORAD(40.0f);
			if (cam->ResetStatics)
				cam->Beta = TargetOrientation - DEGTORAD(40.0f);
		} else if (!test3) {
			DeltaBeta = TargetOrientation - cam->Beta + DEGTORAD(40.0f);
			if (cam->ResetStatics)
				cam->Beta = TargetOrientation + DEGTORAD(40.0f);
		} else if (!test1) {
			DeltaBeta = 0.0f;
		} else {
			if (cam->ResetStatics)
				cam->Beta = TargetOrientation;
			DeltaBeta = TargetOrientation - cam->Beta;
		}

		c = cosf(cam->Beta); s = sinf(cam->Beta);
		testPoint.x = zoom * -c + (zoom + 7.0f) * -c + TargetCoors.x;
		testPoint.y = zoom * -s + (zoom + 7.0f) * -s + TargetCoors.y;
		testPoint.z = WaterLevel + zoom;
		CEntity* entity = nil;
		obscured = WorldClass::ProcessLineOfSight(testPoint, TargetCoors, colPoint, entity, true, false, false, true, false, true, true);
		CenterObscuredWhenChecksWereOn = obscured ? 1.0f : 0.0f;

		TargetWhenChecksWereOn = DeltaBeta + cam->Beta;
	} else {
		obscured = CenterObscuredWhenChecksWereOn != 0.0f;
	}

	if (obscured) {
		CEntity* entity = nil;
		WorldClass::ProcessLineOfSight(cam->Source, TargetCoors, colPoint, entity, true, false, false, true, false, true, true);
		cam->Source = colPoint.point;
	} else {
		WellBufferMe(TargetWhenChecksWereOn, &cam->Beta, &cam->BetaSpeed, 0.07f, 0.015f, true);

		const float zoom = TheCamera->CarZoomValueSmooth;
		s = sinf(cam->Beta); c = cosf(cam->Beta);
		cam->Source = zoom * CVector(-c, -s, 0.0f) + (zoom + 7.0f) * CVector(-c, -s, 0.0f) + TargetCoors;
		cam->Source.z = WaterLevel + zoom;
	}

	if (TheCamera->CarZoomValueSmooth < 0.05f)
		TargetCoors.z += AmountUp * (0.0f - TheCamera->CarZoomValueSmooth);
	TargetCoors.z += TheCamera->CarZoomValueSmooth + 0.5f;

	cam->m_cvecTargetCoorsForFudgeInter = TargetCoors;
	cam->Front = TargetCoors - cam->Source;
	cam->GetVectorsReadyForRW();
	cam->ResetStatics = false;
}


// ---------------------------------------------------------------------------
// Vice City "behind boat" camera (reVC CCam::Process_BehindBoat).
// The Vice City boat camera differs from the GTA III one, so it is a
// separate implementation.
// ---------------------------------------------------------------------------
template<class CamClass, class CameraClass, class VehicleClass, class WorldClass, class ColModelClass>
void
Process_BehindBoat_VC(CameraClass* TheCamera, CamClass* cam, VehicleClass* car, const CVector& CameraTarget, float TargetOrientation)
{
	const float MAX_HEIGHT_UP = 15.0f;
	const float WATER_Z_ADDITION = 2.75f;
	const float WATER_Z_ADDITION_MIN = 1.5f;
	const float SMALLBOAT_CLOSE_ALPHA_MINUS = 0.2f;
	const float afBoatBetaDiffMult[3] = { 0.15f, 0.07f, 0.01f };
	const float afBoatBetaSpeedDiffMult[3] = { 0.02f, 0.015f, 0.005f };

	static float TargetWhenChecksWereOn = 0.0f;
	static float CenterObscuredWhenChecksWereOn = 0.0f;
	static float WaterLevelBuffered = 0.0f;
	static float WaterLevelSpeed = 0.0f;

	if (!car->IsVehicle()) {
		cam->ResetStatics = false;
		return;
	}

	CVector TargetCoors = CameraTarget;
	float WaterLevel = 0.0f;
	float BetaDiffMult = 0.0f;
	float BetaSpeedDiffMult = 0.0f;

	cam->Beta = GetATanOfXY(TargetCoors.x - cam->Source.x, TargetCoors.y - cam->Source.y);
	cam->FOV = DefaultFOV;
	float targetAlpha = 0.0f;

	if (cam->ResetStatics) {
		CenterObscuredWhenChecksWereOn = 0.0f;
		TargetWhenChecksWereOn = 0.0f;
	} else if (cam->DirectionWasLooking != LOOKING_FORWARD) {
		cam->Beta = TargetOrientation;
	}

	if (!CWaterLevel::GetWaterLevelNoWaves(TargetCoors.x, TargetCoors.y, TargetCoors.z, &WaterLevel))
		WaterLevel = TargetCoors.z - 0.5f;
	if (cam->ResetStatics) {
		WaterLevelBuffered = WaterLevel;
		WaterLevelSpeed = 0.0f;
	}
	WellBufferMe(WaterLevel, &WaterLevelBuffered, &WaterLevelSpeed, 0.2f, 0.07f, false);

	const float fixerForGoingBelowGround = 0.4f;
	if (-fixerForGoingBelowGround < TargetCoors.z - WaterLevelBuffered + WATER_Z_ADDITION)
		WaterLevelBuffered += TargetCoors.z - WaterLevelBuffered + WATER_Z_ADDITION - fixerForGoingBelowGround;

	ColModelClass* carCol = (ColModelClass*)car->GetColModel();
	const CVector boatDimensions = carCol->boundingBox.max - carCol->boundingBox.min;
	float boatSize = boatDimensions.Magnitude2D();

	const bool isHeli = (GetHandlingFlags(car) & 0x20000) != 0;
	const bool isBike = (GetHandlingFlags(car) & 0x10000) != 0 || car->IsBike();
	const bool isPlane = (isIII() && car->m_modelIndex == MI_III_DODO) || (GetHandlingFlags(car) & 0x40000);
	const bool isCar = car->IsCar() && !isHeli && !isBike && !isPlane;
	const int index = isCar ? 0 : (isBike ? 1 : (isHeli ? 2 : (isPlane ? 3 : 4)));

	if ((int)TheCamera->CarZoomIndicator == 1) {
		targetAlpha = ZmOneAlphaOffsetVC[index];
		BetaDiffMult = afBoatBetaDiffMult[0];
		BetaSpeedDiffMult = afBoatBetaSpeedDiffMult[0];
	} else if ((int)TheCamera->CarZoomIndicator == 2) {
		targetAlpha = ZmTwoAlphaOffsetVC[index];
		BetaDiffMult = afBoatBetaDiffMult[1];
		BetaSpeedDiffMult = afBoatBetaSpeedDiffMult[1];
	} else if ((int)TheCamera->CarZoomIndicator == 3) {
		targetAlpha = ZmThreeAlphaOffsetVC[index];
		BetaDiffMult = afBoatBetaDiffMult[2];
		BetaSpeedDiffMult = afBoatBetaSpeedDiffMult[2];
	}
	if ((int)TheCamera->CarZoomIndicator == 1 && boatSize < 10.0f) {
		targetAlpha -= SMALLBOAT_CLOSE_ALPHA_MINUS;
		boatSize = 10.0f;
	}

	if (cam->ResetStatics) {
		cam->Alpha = targetAlpha;
		cam->AlphaSpeed = 0.0f;
	}
	WellBufferMe(targetAlpha, &cam->Alpha, &cam->AlphaSpeed, 0.15f, 0.07f, true);

	if (cam->ResetStatics) {
		cam->Beta = TargetOrientation;
		cam->BetaSpeed = 0.0f;
	}
	WellBufferMe(TargetOrientation, &cam->Beta, &cam->BetaSpeed,
		BetaDiffMult * car->m_vecMoveSpeed.Magnitude(), BetaSpeedDiffMult, true);

	cam->Source = (TheCamera->CarZoomValueSmooth + boatSize) * CVector(-cosf(cam->Beta), -sinf(cam->Beta), 0.0f) + TargetCoors;
	cam->Source.z = WaterLevelBuffered + WATER_Z_ADDITION + (boatDimensions.z / 2.0f + MAX_HEIGHT_UP) * sinf(cam->Alpha);

	cam->m_cvecTargetCoorsForFudgeInter = TargetCoors;

	// AvoidTheGeometry: keep the camera out of the world (approximated with a
	// line-of-sight clip, as in the mod's other camera paths).
	{
		pIgnoreEntity = (CEntity*)car;
		CColPoint colPoint;
		CEntity* entity = nil;
		if (WorldClass::ProcessLineOfSight(TargetCoors, cam->Source, colPoint, entity, true, false, false, true, false, false, true))
			cam->Source = colPoint.point;
		pIgnoreEntity = nil;
	}

	cam->Front = TargetCoors - cam->Source;
	cam->Front.Normalise();

	// Steering roll (Vice City vanilla)
	float targetRoll = 0.0f;
	if (cameraWobble) {
		float fwdSpeed = 180.0f * DotProduct(car->m_vecMoveSpeed, car->GetForward());
		if (fwdSpeed > 210.0f)
			fwdSpeed = 210.0f;
		const float steer = (float)pad0.GetSteeringLeftRight() / 128.0f;
		CVector fwdTarget = car->GetForward();
		fwdTarget.Normalise();
		const float angleDiff = acosf(clamp(fabsf(DotProduct(fwdTarget, cam->Front)), 0.0f, 1.0f));
		targetRoll = steer * (fwdSpeed / 210.0f) * (DEGTORAD(10.0f) * TiltOverShoot[index] + cam->f_max_role_angle) * sinf(angleDiff);
	}
	WellBufferMe(targetRoll, &cam->f_Roll, &cam->f_rollSpeed, 0.15f, 0.07f, false);

	cam->GetVectorsReadyForRW();
	cam->ResetStatics = false;
}


// ---------------------------------------------------------------------------
// Vanilla "camera on a string" vehicle camera.
//
// This reproduces the original vehicle camera of both games. GTA III and
// Vice City implement CCam::Process_Cam_On_A_String differently, so the two
// algorithms are kept separate here; the mod's ini-driven features are
// layered on top of the authentic behaviour.
//
// The implementation follows the reversed sources of re3 (GTA III) and
// reVC (Vice City).
// ---------------------------------------------------------------------------
template<class CamClass, class CameraClass, class VehicleClass, class WorldClass, class ColModelClass>
void
Process_Cam_On_A_String_Vanilla(CameraClass* TheCamera, CamClass* cam, VehicleClass* car, const CVector& CameraTarget, float TargetOrientation)
{
	static float AlphaOffset = 0.0f;
	static float AlphaOffsetSpeed = 0.0f;
	static float LastTargetAlphaWithCollisionOn = 0.0f;
	static float LastTopAlphaSpeed = 0.15f;
	static float LastAlphaSpeedStep = 0.015f;
	static float HeightFixerCarsObscuring = 0.0f;
	static float HeightFixerCarsObscuringSpeed = 0.0f;
	static float heliTilt = 0.0f;
	static float heliTiltSpeed = 0.0f;
	static float stepsLeftToChangeBetaByMouse = 0.0f;
	static float heightIncreaseMult = 0.0f;
	static bool PreviousNearCheckNearClipSmall = false;

	if (!car->IsVehicle())
		return;

	CPad* pad = &pad0;
	const bool vc = isVC();
	const bool isHeli = (GetHandlingFlags(car) & 0x20000) != 0;
	const bool isBike = (GetHandlingFlags(car) & 0x10000) != 0 || car->IsBike();
	const bool isPlane = (isIII() && car->m_modelIndex == MI_III_DODO) || (GetHandlingFlags(car) & 0x40000);
	const bool isCar = car->IsCar() && !isHeli && !isBike && !isPlane;
	const int index = isCar ? 0 : (isBike ? 1 : (isHeli ? 2 : (isPlane ? 3 : 4)));

	ColModelClass* carCol = (ColModelClass*)car->GetColModel();
	CVector Dimensions = carCol->boundingBox.max - carCol->boundingBox.min;

	const uint8 nextDirectionIsForward =
		!(pad->GetLookBehindForCar() || pad->GetLookBehindForPed() || pad->GetLookLeft() || pad->GetLookRight()) &&
		cam->DirectionWasLooking == LOOKING_FORWARD;

	// ---- Field of view (vanilla base value plus optional dynamic expansion) ----
	const float baseFOV = (fovProfile == PROFILE_CUSTOM) ? customBaseFOV : DefaultFOV;
	const float maxFOVAdd = (fovProfile == PROFILE_CUSTOM) ? customDynamicFOVMax : 30.0f;
	const float fovStartSpeed = (fovProfile == PROFILE_CUSTOM) ? customDynamicFOVStartSpeed : 0.4f;
	if (cam->ResetStatics) {
		cam->FOV = baseFOV;
	} else if (dynamicSpeedFOV && (isCar || isBike)) {
		float forwardSpeed = DotProduct(car->GetForward(), car->m_vecMoveSpeed);
		if (forwardSpeed > fovStartSpeed)
			cam->FOV += (forwardSpeed - fovStartSpeed) * ms_fTimeStep;
		if (cam->FOV > baseFOV)
			cam->FOV = powf(0.98f, ms_fTimeStep) * (cam->FOV - baseFOV) + baseFOV;
		cam->FOV = clamp(cam->FOV, baseFOV, baseFOV + maxFOVAdd);
	} else {
		cam->FOV = baseFOV;
	}

	// ---- Target position and base distance ----
	CVector TargetCoors = CameraTarget;
	float BaseDist = vc ? Dimensions.Magnitude() : Dimensions.Magnitude2D();
	if (vc && isBike)
		BaseDist *= 1.45f;

	if (isBike && heightIncreaseOnBike) {
		if (car->pPassengers[0])
			heightIncreaseMult = min(1.0f, ms_fTimeStep * 0.02f + heightIncreaseMult);
		else
			heightIncreaseMult = max(0.0f, heightIncreaseMult - ms_fTimeStep * 0.02f);
		Dimensions.z += 0.4f * heightIncreaseMult;
	} else {
		heightIncreaseMult = 0.0f;
	}

	if (vc && isHeli && car->m_status != STATUS_PLAYER_REMOTE)
		TargetCoors += 0.6f * car->GetUp() * Dimensions.z;
	else
		TargetCoors.z += vc ? 0.8f * Dimensions.z : (Dimensions.z - 0.1f);

	// ---- Vehicle-specific zoom ----
	// Vice City scales the zoom per vehicle type itself. For the vanilla
	// profile we therefore leave CarZoomValueSmooth untouched, which also
	// keeps the Widescreen Fix (patching the same table) in control.
	float zoomValue = TheCamera->CarZoomValueSmooth;
	{
		const float* zoomModes = nullptr;
		if (distanceProfile == PROFILE_SA)
			zoomModes = CarZoomModesSA;
		else if (distanceProfile == PROFILE_LCS)
			zoomModes = CarZoomModesLCS;
		else if (distanceProfile == PROFILE_CUSTOM)
			zoomModes = CarZoomModesCustom;
		else if (!vc && vehicleSpecificZoom)
			zoomModes = CarZoomModesVC; // GTA III has no per-vehicle table; emulate Vice City's

		if (zoomModes) {
			int ind = (int)TheCamera->CarZoomIndicator;
			if (ind == 3)
				zoomValue = zoomModes[index + 10];
			else if (ind == 2)
				zoomValue = zoomModes[index + 5] +
					(zoomValue - 1.9f) * (zoomModes[index + 10] - zoomModes[index + 5]) / (3.9f - 1.9f);
			else if (ind == 1)
				zoomValue = zoomModes[index] +
					(zoomValue - 0.05f) * (zoomModes[index + 5] - zoomModes[index]) / (1.9f - 0.05f);
			if (zoomValue < zoomModes[index])
				zoomValue = zoomModes[index];
		}
	}

	// ---- Elastic string stretch (adds SA-style speed stretch on top) ----
	float extraDist = 0.0f;
	if (elasticStringPhysics && (isCar || isBike || car->IsBoat())) {
		float forwardSpeed = DotProduct(car->m_vecMoveSpeed, car->GetForward()) * 180.0f;
		extraDist += clamp(forwardSpeed * (2.0f / 210.0f), -1.0f, 2.0f);
	}

	// ---- Vice City RC vehicles need extra distance / angle ----
	float extraAlpha = 0.0f;
	if (vc) {
		if (car->m_modelIndex == RcRaider || car->m_modelIndex == RcGoblin) {
			extraDist += 6.0f;
			extraAlpha = 0.2f;
		} else if (!isReLCS && car->m_modelIndex == MI_VC_RCBARON) {
			extraDist += 9.5f;
			extraAlpha = 0.295f;
		}
	}

	const float distOffset = (distanceProfile == PROFILE_CUSTOM) ? customDistOffset : 0.0f;
	cam->CA_MAX_DISTANCE = BaseDist + 0.1f + zoomValue + extraDist + distOffset;
	cam->CA_MIN_DISTANCE = min(BaseDist * 0.6f, 3.5f);
	if (cam->CA_MIN_DISTANCE > cam->CA_MAX_DISTANCE)
		cam->CA_MIN_DISTANCE = cam->CA_MAX_DISTANCE - 0.05f;

	// ---- Reset statics when entering the vehicle ----
	if (cam->ResetStatics) {
		cam->AlphaSpeed = 0.0f;
		cam->BetaSpeed = 0.0f;
		cam->f_Roll = 0.0f;
		cam->f_rollSpeed = 0.0f;
		heliTilt = 0.0f;
		heliTiltSpeed = 0.0f;
		AlphaOffset = 0.0f;
		AlphaOffsetSpeed = 0.0f;
		LastTargetAlphaWithCollisionOn = 0.0f;
		LastTopAlphaSpeed = 0.15f;
		LastAlphaSpeedStep = 0.015f;
		HeightFixerCarsObscuring = 0.0f;
		HeightFixerCarsObscuringSpeed = 0.0f;
		stepsLeftToChangeBetaByMouse = 0.0f;
		heightIncreaseMult = 0.0f;

		if (TheCamera->m_bIdleOn)
			TheCamera->m_uiTimeWeEnteredIdle = m_snTimeInMilliseconds;

		cam->Beta = LimitRadianAngle(TargetOrientation + (TheCamera->m_bCamDirectlyInFront ? PI : 0.0f));
		cam->Alpha = 0.0f;

		cam->Source.x = TargetCoors.x - cosf(cam->Beta) * cam->CA_MAX_DISTANCE;
		cam->Source.y = TargetCoors.y - sinf(cam->Beta) * cam->CA_MAX_DISTANCE;
		cam->Source.z = TargetCoors.z;
	}

	// ---- Looking behind / returning from looking around ----
	if (pad->GetLookBehindForCar()) {
		if (cam->DirectionWasLooking == LOOKING_FORWARD || !cam->LookingBehind)
			TheCamera->m_bCamDirectlyInFront = true;
	}
	if (!(pad->GetLookBehindForCar() || pad->GetLookBehindForPed() || pad->GetLookLeft() || pad->GetLookRight())) {
		if (cam->DirectionWasLooking != LOOKING_FORWARD)
			TheCamera->m_bCamDirectlyBehind = true;
	}

	// ---- Basic string constraint (Cam_On_A_String_Unobscured) ----
	if (cam->ResetStatics) {
		CVector d0 = cam->Source - TargetCoors;
		cam->Source = TargetCoors + d0 * (cam->CA_MAX_DISTANCE + 1.0f);
	}

	CVector stringDist = cam->Source - TargetCoors;
	float stringLength = stringDist.Magnitude2D();
	if (stringLength < 0.001f) {
		CVector fwd = car->GetForward();
		fwd.z = 0.0f;
		fwd.Normalise();
		cam->Source = TargetCoors - fwd * cam->CA_MAX_DISTANCE;
		stringDist = cam->Source - TargetCoors;
		stringLength = stringDist.Magnitude2D();
	}
	if (stringLength > cam->CA_MAX_DISTANCE) {
		cam->Source.x = TargetCoors.x + stringDist.x / stringLength * cam->CA_MAX_DISTANCE;
		cam->Source.y = TargetCoors.y + stringDist.y / stringLength * cam->CA_MAX_DISTANCE;
	} else if (stringLength < cam->CA_MIN_DISTANCE) {
		cam->Source.x = TargetCoors.x + stringDist.x / stringLength * cam->CA_MIN_DISTANCE;
		cam->Source.y = TargetCoors.y + stringDist.y / stringLength * cam->CA_MIN_DISTANCE;
	}

	cam->Beta = GetATanOfXY(TargetCoors.x - cam->Source.x, TargetCoors.y - cam->Source.y);
	cam->Alpha = LimitRadianAngle(cam->Alpha);
	cam->Beta = LimitRadianAngle(cam->Beta);

	// Vice City: when stationary and firing the Firetruck water cannon, the camera
	// follows the turret so the player can aim. (Process_Cam_On_A_String)
	if (vc && car->m_modelIndex == FireTruk && pad->GetCarGunFired() && car->m_vecMoveSpeed.Magnitude2D() < 0.01f) {
		float targetBeta = LimitRadianAngle(car->GetForward().Heading() - *GetDoomAnglePtrLR(car) + HALFPI);
		float firetruckDeltaBeta = LimitRadianAngle(targetBeta - cam->Beta);
		float dist = (TargetCoors - cam->Source).Magnitude();
		dist = 0.1f * dist * clamp(firetruckDeltaBeta, -0.8f, 0.8f);
		cam->Source += dist * CrossProduct(cam->Front, CVector(0.0f, 0.0f, 1.0f));
	}

	cam->m_fDistanceBeforeChanges = (cam->Source - TargetCoors).Magnitude2D();

	// ---- Mouse free-look (ported from the San Andreas camera) ----
	bool mouseChangesBeta = false;
	if (mouseFreeLook && m_bUseMouse3rdPerson && !GetDisablePlayerControls(pad) && nextDirectionIsForward) {
		float mouseY = CPad::NewMouseControllerState.y * 2.0f;
		float mouseX = CPad::NewMouseControllerState.x * -2.0f;
		if ((mouseX != 0.0f || mouseY != 0.0f) && m_bDisableMouseSteering) {
			float v113 = cam->FOV * 0.0125f;
			cam->Beta += mouseX * v113 * GetMouseAccel(TheCamera);
			cam->Alpha += mouseY * v113 * GetMouseAccel(TheCamera);
			cam->BetaSpeed = 0.0f;
			cam->AlphaSpeed = 0.0f;
			stepsLeftToChangeBetaByMouse = 50.0f;
			mouseChangesBeta = true;
		} else if (stepsLeftToChangeBetaByMouse > 0.0f) {
			cam->BetaSpeed = 0.0f;
			cam->AlphaSpeed = 0.0f;
			stepsLeftToChangeBetaByMouse = max(0.0f, stepsLeftToChangeBetaByMouse - ms_fTimeStep);
			mouseChangesBeta = true;
		}
	}

	// ---- Alpha offset: the vertical angle for each zoom level ----
	{
		const int zoomIndicator = (int)TheCamera->CarZoomIndicator;
		if (vc) {
			const float* off1 = ZmOneAlphaOffsetVC;
			const float* off2 = ZmTwoAlphaOffsetVC;
			const float* off3 = ZmThreeAlphaOffsetVC;
			if (anglesProfile == PROFILE_CUSTOM) {
				off1 = ZmOneAlphaOffsetCustom; off2 = ZmTwoAlphaOffsetCustom; off3 = ZmThreeAlphaOffsetCustom;
			} else if (anglesProfile == PROFILE_SA) {
				off1 = ZmOneAlphaOffset; off2 = ZmTwoAlphaOffset; off3 = ZmThreeAlphaOffset;
			} else if (anglesProfile == PROFILE_LCS) {
				off1 = ZmOneAlphaOffsetLCS; off2 = ZmTwoAlphaOffsetLCS; off3 = ZmThreeAlphaOffsetLCS;
			}
			float targetAlphaOffset = 0.0f;
			if (zoomIndicator == 1) targetAlphaOffset = off1[index] + extraAlpha;
			else if (zoomIndicator == 2) targetAlphaOffset = off2[index] + extraAlpha;
			else if (zoomIndicator == 3) targetAlphaOffset = off3[index] + extraAlpha;
			if (cam->ResetStatics)
				AlphaOffset = targetAlphaOffset;
			WellBufferMe(targetAlphaOffset, &AlphaOffset, &AlphaOffsetSpeed, 0.17f, 0.08f, false);
		} else if (anglesProfile == PROFILE_CUSTOM) {
			if (zoomIndicator == 1) AlphaOffset = customAngleNear;
			else if (zoomIndicator == 2) AlphaOffset = customAngleMid;
			else if (zoomIndicator == 3) AlphaOffset = customAngleFar;
		} else if (anglesProfile == PROFILE_SA || anglesProfile == PROFILE_LCS) {
			const float* off1 = (anglesProfile == PROFILE_LCS) ? ZmOneAlphaOffsetLCS : ZmOneAlphaOffset;
			const float* off2 = (anglesProfile == PROFILE_LCS) ? ZmTwoAlphaOffsetLCS : ZmTwoAlphaOffset;
			const float* off3 = (anglesProfile == PROFILE_LCS) ? ZmThreeAlphaOffsetLCS : ZmThreeAlphaOffset;
			if (zoomIndicator == 1) AlphaOffset = off1[index] + extraAlpha;
			else if (zoomIndicator == 2) AlphaOffset = off2[index] + extraAlpha;
			else if (zoomIndicator == 3) AlphaOffset = off3[index] + extraAlpha;
		} else {
			// GTA III: the offset is derived from the smooth zoom value.
			float zv = TheCamera->CarZoomValueSmooth;
			if (zv < 0.1f)
				zv = 0.1f;
			if (zoomIndicator == 1) AlphaOffset = GetATanOfXY(23.0f, zv);
			else if (zoomIndicator == 2) AlphaOffset = GetATanOfXY(10.8f, zv);
			else if (zoomIndicator == 3) AlphaOffset = GetATanOfXY(7.0f, zv);
		}
	}

	// ---- Camera height / pitch (WorkOutCamHeight) ----
	{
		const CVector forward = car->GetForward();
		float carAlpha = LimitRadianAngle(GetATanOfXY(forward.Magnitude2D(), forward.z));
		float deltaBeta = LimitRadianAngle(cam->Beta - TargetOrientation);
		carAlpha = -carAlpha * cosf(deltaBeta);
		const float length = (cam->Source - TargetCoors).Magnitude2D();

		int effectivePitchTilt = pitchTilt;
		if (effectivePitchTilt == 2)
			effectivePitchTilt = vc ? 1 : 0;

		if (vc) {
			// Vice City: the Firetruck cannon pitches the camera while firing.
			if (car->m_modelIndex == FireTruk && pad->GetCarGunFired()) {
				carAlpha = DEGTORAD(10.0f);
			} else if (isHeli && effectivePitchTilt != 0 && length != 0.0f) {
				carAlpha = 0.0f;
				const float heliFwdSpeed = DotProduct(car->m_vecMoveSpeed, forward) * 180.0f;
				const float heliFwdZ = forward.z;
				const float heliFwdXY = forward.Magnitude2D();
				const float alphaAmount = min(fabsf(heliFwdSpeed / 90.0f), 1.0f);
				if (heliFwdXY != 0.0f || heliFwdZ != 0.0f)
					carAlpha = GetATanOfXY(heliFwdXY, fabsf(heliFwdZ)) * alphaAmount;

				CColPoint point;
				CEntity* entity = nil;
				CVector test = cam->Source;
				test.z = TargetCoors.z + 0.2f + length * sinf(carAlpha + AlphaOffset) + cam->m_fCloseInCarHeightOffset;
				if (WorldClass::ProcessVerticalLine(test, car->GetPosition().z, point, entity, true, false, false, false, false, false, nil)) {
					const float sinV = (point.point.z - TargetCoors.z - 0.2f - cam->m_fCloseInCarHeightOffset) / length;
					carAlpha = asinf(clamp(sinV, -1.0f, 1.0f)) - AlphaOffset;
					if (carAlpha < 0.0f)
						AlphaOffset += carAlpha;
				}
			}

			if (effectivePitchTilt == 1)
				carAlpha = clamp(carAlpha, 0.0f, DEGTORAD(89.0f));
			else if (effectivePitchTilt == 3)
				carAlpha = clamp(carAlpha, -0.35f, 0.35f);
			else
				carAlpha = 0.0f;

			if (cam->ResetStatics)
				cam->Alpha = carAlpha;

			float targetAlpha = cam->Alpha;
			if (fabsf(LimitRadianAngle(carAlpha - targetAlpha)) > 0.0f && !IsVehicleSuspensionHigh(TheCamera))
				targetAlpha = carAlpha;

			if (!mouseChangesBeta) {
				if (isBike || isHeli)
					WellBufferMe(targetAlpha, &cam->Alpha, &cam->AlphaSpeed, 0.09f, 0.04f, true);
				else
					WellBufferMe(targetAlpha, &cam->Alpha, &cam->AlphaSpeed, 0.15f, 0.07f, true);
			}
		} else {
			// GTA III: nearly level with a small dead-zone plus roof/ground checks
			// (WorkOutCamHeight).
			float topAlphaSpeed = 0.15f;
			float alphaSpeedStep = 0.015f;
			bool camClear = true;

			if (effectivePitchTilt == 0) {
				if (carAlpha < -0.01f)
					carAlpha = -0.01f;
			} else if (effectivePitchTilt == 1) {
				carAlpha = clamp(carAlpha, 0.0f, DEGTORAD(89.0f));
			} else {
				carAlpha = clamp(carAlpha, -0.35f, 0.35f);
			}

			if (cam->ResetStatics)
				cam->Alpha = carAlpha;

			float deltaAlpha = LimitRadianAngle(carAlpha - cam->Alpha);
			const float angleLimit = DEGTORAD(1.8f);
			if (deltaAlpha > angleLimit)
				deltaAlpha -= angleLimit;
			else if (deltaAlpha < -angleLimit)
				deltaAlpha += angleLimit;
			else
				deltaAlpha = 0.0f;

			if (cam->m_bCollisionChecksOn) {
				const float targetHeight = Dimensions.z;
				float targetAlpha = 0.0f;
				bool foundRoofCenter = false, foundRoofSide1 = false, foundRoofSide2 = false;
				bool foundCamRoof = false, foundCamGround = false;
				float camRoof = 0.0f;
				const float carBottom = TargetCoors.z - targetHeight / 2.0f;

				const float carRoof = WorldClass::FindRoofZFor3DCoord(TargetCoors.x, TargetCoors.y, carBottom, &foundRoofCenter);

				CVector fwd = car->GetForward();
				fwd.Normalise();
				const float carSideAngle = GetATanOfXY(fwd.x, fwd.y) + HALFPI;
				const float sideX = 2.5f * cosf(carSideAngle);
				const float sideY = 2.5f * sinf(carSideAngle);
				WorldClass::FindRoofZFor3DCoord(TargetCoors.x + sideX, TargetCoors.y + sideY, carBottom, &foundRoofSide1);
				WorldClass::FindRoofZFor3DCoord(TargetCoors.x - sideX, TargetCoors.y - sideY, carBottom, &foundRoofSide2);

				const float camGround = WorldClass::FindGroundZFor3DCoord(cam->Source.x, cam->Source.y,
					TargetCoors.z + length * sinf(cam->Alpha + AlphaOffset) + cam->m_fCloseInCarHeightOffset, &foundCamGround);
				float camTargetZ = 0.0f;
				if (foundCamGround) {
					camRoof = WorldClass::FindRoofZFor3DCoord(cam->Source.x, cam->Source.y, camGround + targetHeight, &foundCamRoof);
					camTargetZ = camGround + targetHeight * 1.5f + 0.1f;
				} else {
					foundCamRoof = false;
					camTargetZ = TargetCoors.z;
				}

				if (foundRoofCenter && !foundCamRoof && (foundRoofSide1 || foundRoofSide2)) {
					targetAlpha = GetATanOfXY(cam->CA_MAX_DISTANCE, carRoof - camTargetZ - 1.5f);
					camClear = false;
				}
				if (foundCamRoof) {
					const float roof = foundRoofCenter ? min(camRoof, carRoof) : camRoof;
					targetAlpha = GetATanOfXY(cam->CA_MAX_DISTANCE, roof - camTargetZ - 1.5f);
					camClear = false;
				}

				targetAlpha = LimitRadianAngle(targetAlpha);
				if (targetAlpha < DEGTORAD(-7.0f))
					targetAlpha = DEGTORAD(-7.0f);
				if (targetAlpha > AlphaOffset)
					camClear = true;

				PreviousNearCheckNearClipSmall = false;
				if (!camClear) {
					PreviousNearCheckNearClipSmall = true;
					RwCameraSetNearClipPlane(RwCamera, DefaultNearClip);
					deltaAlpha = LimitRadianAngle(targetAlpha - (cam->Alpha + AlphaOffset));
					topAlphaSpeed = 0.3f;
					alphaSpeedStep = 0.03f;
				}

				const float camZ = TargetCoors.z + length * sinf(cam->Alpha + deltaAlpha + AlphaOffset) + cam->m_fCloseInCarHeightOffset;
				bool foundGround, foundRoof;
				const float camGround2 = WorldClass::FindGroundZFor3DCoord(cam->Source.x, cam->Source.y, camZ, &foundGround);
				if (foundGround && camClear) {
					if (camZ - camGround2 < 1.5f) {
						PreviousNearCheckNearClipSmall = true;
						RwCameraSetNearClipPlane(RwCamera, DefaultNearClip);
						const float dz = camGround2 + 1.5f - TargetCoors.z;
						float a = (length == 0.0f || dz == 0.0f) ? cam->Alpha : GetATanOfXY(length, dz);
						deltaAlpha = LimitRadianAngle(a) - cam->Alpha;
					}
				} else if (camClear) {
					float camRoof2 = WorldClass::FindRoofZFor3DCoord(cam->Source.x, cam->Source.y, camZ, &foundRoof);
					if (foundRoof && camZ - camRoof2 < 1.5f) {
						PreviousNearCheckNearClipSmall = true;
						RwCameraSetNearClipPlane(RwCamera, DefaultNearClip);
						if (camRoof2 > TargetCoors.z + 3.5f)
							camRoof2 = TargetCoors.z + 3.5f;
						const float dz = camRoof2 + 1.5f - TargetCoors.z;
						float a = (length == 0.0f || dz == 0.0f) ? cam->Alpha : GetATanOfXY(length, dz);
						deltaAlpha = LimitRadianAngle(a) - cam->Alpha;
					}
				}

				LastTargetAlphaWithCollisionOn = deltaAlpha + cam->Alpha;
				LastTopAlphaSpeed = topAlphaSpeed;
				LastAlphaSpeedStep = alphaSpeedStep;
			} else if (PreviousNearCheckNearClipSmall) {
				RwCameraSetNearClipPlane(RwCamera, DefaultNearClip);
			}

			if (!mouseChangesBeta)
				WellBufferMe(LastTargetAlphaWithCollisionOn, &cam->Alpha, &cam->AlphaSpeed, LastTopAlphaSpeed, LastAlphaSpeedStep, true);
		}

		cam->Source.z = TargetCoors.z + sinf(cam->Alpha + AlphaOffset) * length + cam->m_fCloseInCarHeightOffset;
	}

	// ---- Rotate the camera behind the car when driving forward ----
	{
		const float maxDiffBeta = DEGTORAD(160.0f);
		float forwardSpeed = DotProduct(car->GetForward(), car->m_vecMoveSpeed);
		bool movingForward = forwardSpeed > 0.02f;

		if (fabsf(LimitRadianAngle(TargetOrientation - cam->Beta)) > PI - maxDiffBeta && movingForward && TheCamera->m_uiTransitionState == 0)
			cam->m_bFixingBeta = true;

		bool setBeta = TheCamera->m_bCamDirectlyBehind || TheCamera->m_bCamDirectlyInFront || TheCamera->m_bUseTransitionBeta;

		if ((cam->m_bFixingBeta || setBeta) && !mouseChangesBeta) {
			float stiffness = (cameraStiffness >= 0.0f) ? cameraStiffness : 1.0f;
			WellBufferMe(TargetOrientation, &cam->Beta, &cam->BetaSpeed, 0.15f * stiffness, 0.007f * stiffness, true);

			if (TheCamera->m_bCamDirectlyBehind)
				cam->Beta = TargetOrientation;
			if (TheCamera->m_bCamDirectlyInFront)
				cam->Beta = TargetOrientation + PI;
			if (TheCamera->m_bUseTransitionBeta)
				cam->Beta = cam->m_fTransitionBeta;

			float d2 = (cam->Source - TargetCoors).Magnitude2D();
			if (d2 < 0.001f)
				d2 = cam->CA_MAX_DISTANCE;
			cam->Source.x = TargetCoors.x - cosf(cam->Beta) * d2;
			cam->Source.y = TargetCoors.y - sinf(cam->Beta) * d2;

			if (fabsf(LimitRadianAngle(TargetOrientation - cam->Beta)) < DEGTORAD(2.0f))
				cam->m_bFixingBeta = false;
		}
	}
	TheCamera->m_bCamDirectlyBehind = false;
	TheCamera->m_bCamDirectlyInFront = false;

	// Re-apply mouse free-look after the beta/height logic moved the camera.
	if (mouseChangesBeta) {
		cam->Alpha = clamp(cam->Alpha, -1.2f, 1.2f);
		float d2 = (cam->Source - TargetCoors).Magnitude2D();
		if (d2 < 0.1f)
			d2 = cam->CA_MAX_DISTANCE;
		cam->Source.x = TargetCoors.x - cosf(cam->Beta) * d2;
		cam->Source.y = TargetCoors.y - sinf(cam->Beta) * d2;
		float d3 = (cam->Source - TargetCoors).Magnitude2D();
		cam->Source.z = TargetCoors.z + sinf(cam->Alpha + AlphaOffset) * d3 + cam->m_fCloseInCarHeightOffset;
	}

	// ---- Keep the camera out of geometry ----
	{
		pIgnoreEntity = (CEntity*)car;
		CColPoint colPoint;
		CEntity* hitEntity = nil;
		if (WorldClass::ProcessLineOfSight(TargetCoors, cam->Source, colPoint, hitEntity, true, false, false, true, false, false, true))
			cam->Source = colPoint.point;
		pIgnoreEntity = nil;
	}

	// ---- Raise the camera when another vehicle obscures the view ----
	{
		CColPoint colPoint;
		CEntity* entity = nil;
		float heightTarget = 0.0f;
		if (WorldClass::ProcessLineOfSight(TargetCoors, cam->Source, colPoint, entity, false, true, false, false, false, false, false)) {
			if (entity) {
				CBaseModelInfo* mi = CModelInfo::GetModelInfo(entity->GetModelIndex());
				if (mi)
					heightTarget = ((ColModelClass*)mi->GetColModel())->boundingBox.max.z + 1.0f + TargetCoors.z - cam->Source.z;
			}
			if (heightTarget < 0.0f)
				heightTarget = 0.0f;
		}
		WellBufferMe(heightTarget, &HeightFixerCarsObscuring, &HeightFixerCarsObscuringSpeed, 0.2f, 0.025f, false);
		cam->Source.z += HeightFixerCarsObscuring;
	}

	// ---- Keep the camera above the water ----
	if (!seeUnderwater && CameraTarget.z >= -2.0f) {
		float level = -6000.0f;
		if (CWaterLevel::GetWaterLevelNoWaves(cam->Source.x, cam->Source.y, cam->Source.z, &level)) {
			if (cam->Source.z < level + 0.5f)
				cam->Source.z = level + 0.5f;
		}
	}

	// ---- VCS camera shake ----
	// Ported from ThirteenAG's WidescreenFixesPack
	// (MIT licensed, see licenses/WidescreenFixesPack.txt).
	if (vcsCamShake && (isCar || isBike)) {
		float vehSpeed = car->m_vecMoveSpeed.Magnitude();
		if (vehSpeed > 0.65f) {
			float shakeFactor = (min(vehSpeed, 1.0f) - 0.65f) / 0.35f / 200.0f;
			int r = rand();
			cam->Source.x += ((r & 0xF) - 7) * shakeFactor;
			cam->Source.y += (((r >> 4) & 0xF) - 7) * shakeFactor;
			cam->Source.z += (((r >> 8) & 0xF) - 7) * shakeFactor;
		}
	}

	cam->Front = TargetCoors - cam->Source;
	cam->Front.Normalise();

	// ---- Roll / wobble and helicopter tilt ----
	if (vc && isHeli) {
		float targetTilt = DotProduct(cam->Front, car->m_vecMoveSpeed);
		CVector upTarget = car->GetUp();
		upTarget.Normalise();
		int dir = targetTilt < 0.0f ? -1 : 1;
		if (heliTilt != 0.0f)
			targetTilt += TiltOverShoot[index] * targetTilt / heliTilt * dir;
		WellBufferMe(targetTilt, &heliTilt, &heliTiltSpeed, TiltTopSpeed[index], TiltSpeedStep[index], false);

		cam->Up = CVector(0.0f, 0.0f, 1.0f) - (CVector(0.0f, 0.0f, 1.0f) - upTarget) * heliTilt;
		cam->Up.Normalise();
		CVector left = CrossProduct(cam->Up, cam->Front);
		cam->Up = CrossProduct(cam->Front, left);
		cam->Up.Normalise();
	} else {
		float targetRoll = 0.0f;
		if (cameraWobble && !mouseChangesBeta) {
			float fwdSpeed = 180.0f * DotProduct(car->m_vecMoveSpeed, car->GetForward());
			if (fwdSpeed > 210.0f)
				fwdSpeed = 210.0f;

			float steer = (float)pad->GetSteeringLeftRight() / 128.0f;
			CVector fwdTarget = car->GetForward();
			fwdTarget.Normalise();
			float angleDiff = acosf(clamp(fabsf(DotProduct(fwdTarget, cam->Front)), 0.0f, 1.0f));

			targetRoll = steer * (fwdSpeed / 210.0f) *
				(DEGTORAD(10.0f) * TiltOverShoot[index] + cam->f_max_role_angle) * sinf(angleDiff);
		}
		WellBufferMe(targetRoll, &cam->f_Roll, &cam->f_rollSpeed, 0.15f, 0.07f, false);
		cam->GetVectorsReadyForRW();
	}

	cam->m_cvecTargetCoorsForFudgeInter = TargetCoors;
	lookingRelativelyLeft = false;
	lookingRelativelyRight = false;

	// ---- Turret control (Rhino / Firetruck) ----
	if (modernTurretControl && (car->m_modelIndex == Tank || car->m_modelIndex == FireTruk)) {
		CVector hi = Multiply3x3(cam->Front, car->GetMatrix());
		float angleToFace = (car->m_modelIndex == FireTruk ? -hi.Heading() : hi.Heading());

		if (angleToFace <= *GetDoomAnglePtrLR(car) + PI) {
			if (angleToFace < *GetDoomAnglePtrLR(car) - PI)
				angleToFace += TWOPI;
		} else {
			angleToFace -= TWOPI;
		}

		float neededTurn = angleToFace - *GetDoomAnglePtrLR(car);
		float turnPerFrame = ms_fTimeStep * (car->m_modelIndex == FireTruk ? 0.05f : 0.015f);
		if (neededTurn <= turnPerFrame) {
			if (neededTurn < -turnPerFrame)
				angleToFace = *GetDoomAnglePtrLR(car) - turnPerFrame;
		} else {
			angleToFace = turnPerFrame + *GetDoomAnglePtrLR(car);
		}

		if (car->m_modelIndex == Tank && *GetDoomAnglePtrLR(car) != angleToFace)
			DMAudio.PlayOneShot(car->m_audioEntityId, (isIII() ? 26 : 28), fabsf(angleToFace - *GetDoomAnglePtrLR(car)));

		*GetDoomAnglePtrLR(car) = angleToFace;
		if (*GetDoomAnglePtrLR(car) < -PI)
			*GetDoomAnglePtrLR(car) += TWOPI;
		else if (*GetDoomAnglePtrLR(car) > PI)
			*GetDoomAnglePtrLR(car) -= TWOPI;

		if (car->m_modelIndex == FireTruk) {
			float alphaToFace = atan2f(hi.z, hi.Magnitude2D()) + 0.2617994f;
			float neededAlphaTurn = alphaToFace - *GetDoomAnglePtrUD(car);
			float alphaTurnPerFrame = ms_fTimeStep * 0.02f;

			if (neededAlphaTurn > alphaTurnPerFrame) {
				*GetDoomAnglePtrUD(car) = alphaTurnPerFrame + *GetDoomAnglePtrUD(car);
			} else if (neededAlphaTurn >= -alphaTurnPerFrame) {
				*GetDoomAnglePtrUD(car) = alphaToFace;
			} else {
				*GetDoomAnglePtrUD(car) = *GetDoomAnglePtrUD(car) - alphaTurnPerFrame;
			}

			const float turretMinY = -0.34906587f;
			const float turretMaxY = 0.34906587f;
			if (*GetDoomAnglePtrUD(car) < turretMinY)
				*GetDoomAnglePtrUD(car) = turretMinY;
			else if (*GetDoomAnglePtrUD(car) > turretMaxY)
				*GetDoomAnglePtrUD(car) = turretMaxY;

			if (isReLCS && GetVehicleComponent(car, 8)) {
				CMatrix mat;
				mat.Attach(RwFrameGetMatrix(GetVehicleComponent(car, 8)));
				CVector pos = mat.GetPosition();
				mat.SetRotateZ(-(*GetDoomAnglePtrLR(car)));
				mat.GetPosition() = pos;
				mat.UpdateRW();
			}
		}
	} else if (modernDriveBy && !isHeli) {
		CVector hi = Multiply3x3(cam->Front, car->GetMatrix());
		if (hi.Heading() >= 0.5235987756f && hi.Heading() <= 2.617993878f)
			lookingRelativelyLeft = true;
		else if (hi.Heading() <= -0.5235987756f && hi.Heading() >= -2.617993878f)
			lookingRelativelyRight = true;
	}

	cam->ResetStatics = false;
}

template<class CamClass, class CameraClass, class VehicleClass, class WorldClass, class ColModelClass>
void
Process_FollowCar_SA(const CVector& CameraTarget, float TargetOrientation, CamClass* cam, CameraClass* TheCamera) // bool sthForScript)
{
	// Reminder: SA vehicle subclass 3 is heli, 4 is plane, class 9 is bike

	// Missing things on III CCam
	static CVector m_aTargetHistoryPosOne;
	static CVector m_aTargetHistoryPosTwo;
	static CVector m_aTargetHistoryPosThree;
	static int m_nCurrentHistoryPoints = 0;
	static float lastBeta = -9999.0f;
	static float lastAlpha = -9999.0f;
	static float stepsLeftToChangeBetaByMouse;
	static float flt_9BF250;
	static bool alphaCorrected;
	static float heightIncreaseMult;
	static float camPitchTilt = 0.0f;
	static float camPitchTiltSpeed = 0.0f;

	if (!cam->CamTargetEntity->IsVehicle())
		return;

	if (!ginputLoaded) {
		if (GInput_Load(&ginputPad)) {
			OnGInputSettingsReload();
			ginputPad->SendEvent(GINPUT_EVENT_REGISTER_SETTINGS_RELOAD_CALLBACK, OnGInputSettingsReload);
			ginputLoaded = 2;
		} else
			ginputLoaded = 1;
	}

	VehicleClass* car = (VehicleClass*)cam->CamTargetEntity;

	bool useAnchoring = (cameraAnchoring == 1) || (cameraAnchoring == 2 && (masterProfile == PROFILE_VANILLA || (distanceProfile == PROFILE_VANILLA && anglesProfile == PROFILE_VANILLA)));
	if (useAnchoring) {
		if (cam->Mode == MODE_BEHINDBOAT) {
			if (isVC())
				Process_BehindBoat_VC<CamClass, CameraClass, VehicleClass, WorldClass, ColModelClass>(
					TheCamera, cam, car, CameraTarget, TargetOrientation);
			else
				Process_BehindBoat_Vanilla<CamClass, CameraClass, VehicleClass, WorldClass>(
					TheCamera, cam, car, CameraTarget, TargetOrientation);
		} else {
			Process_Cam_On_A_String_Vanilla<CamClass, CameraClass, VehicleClass, WorldClass, ColModelClass>(
				TheCamera, cam, car, CameraTarget, TargetOrientation);
		}
		return;
	}

	CVector TargetCoors = CameraTarget;
	uint8 camSetArrPos = 0;

	// For compatibility with III with Aircraft mod and VC
	bool isPlane = isIII() && car->m_modelIndex == MI_III_DODO || GetHandlingFlags(car) & 0x40000;
	bool isHeli = GetHandlingFlags(car) & 0x20000;
	bool isBike = (GetHandlingFlags(car) & 0x10000) || car->IsBike();
	bool isCar = car->IsCar() && !isPlane && !isHeli && !isBike;

	CPad* pad = &pad0;

	// Next direction is 0x8CC384 in SA, non-existent in III
	uint8 nextDirectionIsForward = !(pad->GetLookBehindForCar() || pad->GetLookBehindForPed() || pad->GetLookLeft() || pad->GetLookRight()) &&
		cam->DirectionWasLooking == LOOKING_FORWARD;

	if (distanceProfile == PROFILE_LCS && car->m_modelIndex == FireTruk) {
		camSetArrPos = 7;
	}
	else if (car->m_modelIndex == RcBandit || (isVC() && !isReLCS && car->m_modelIndex == MI_VC_RCBARON)) {
		camSetArrPos = 5;
	}
	else if (isVC() && (car->m_modelIndex == RcRaider || car->m_modelIndex == RcGoblin)) {
		camSetArrPos = 6;
	}
	else if (car->IsBoat()) {
		camSetArrPos = 4;
	}
	else if (isBike) {
		camSetArrPos = 1;
	}
	else if (isPlane) {
		camSetArrPos = 3;
	}
	else if (isHeli) {
		camSetArrPos = 2;
	}

	const float (*CARCAM_SET)[15] = (distanceProfile == PROFILE_CUSTOM) ? CARCAM_SET_CUSTOM :
		((distanceProfile == PROFILE_VANILLA) ? CARCAM_SET_VANILLA :
		((distanceProfile == PROFILE_LCS) ? CARCAM_SET_LCS : CARCAM_SET_SA));

	const float *CarZoomModes = (distanceProfile == PROFILE_CUSTOM) ? CarZoomModesCustom :
		((distanceProfile == PROFILE_VANILLA) ? ((isVC() || vehicleSpecificZoom) ? CarZoomModesVC : CarZoomModesIII) :
		((distanceProfile == PROFILE_LCS) ? CarZoomModesLCS : CarZoomModesSA));

	const float (*ANGLES_CARCAM_SET)[15] = (anglesProfile == PROFILE_CUSTOM) ? CARCAM_SET_CUSTOM :
		((anglesProfile == PROFILE_VANILLA) ? CARCAM_SET_VANILLA :
		((anglesProfile == PROFILE_LCS) ? CARCAM_SET_LCS : CARCAM_SET_SA));

	// RC Heli/planes use same alpha values with heli/planes (LCS firetruck will fallback to 0)
	uint8 alphaArrPos = (camSetArrPos > 4 ? (isPlane ? 3 : (isHeli ? 2 : 0)) : camSetArrPos);
	float zoomModeAlphaOffset = 0.0f;

	if (isHeli && car->m_status == STATUS_PLAYER_REMOTE) {
		if (anglesProfile == PROFILE_CUSTOM)
			zoomModeAlphaOffset = ZmTwoAlphaOffsetCustom[alphaArrPos];
		else if (anglesProfile == PROFILE_VANILLA)
			zoomModeAlphaOffset = isVC() ? ZmTwoAlphaOffsetVC[alphaArrPos] : ZmTwoAlphaOffsetIII[alphaArrPos];
		else if (anglesProfile == PROFILE_LCS)
			zoomModeAlphaOffset = ZmTwoAlphaOffsetLCS[alphaArrPos];
		else
			zoomModeAlphaOffset = ZmTwoAlphaOffset[alphaArrPos];
	}
	else {
		switch ((int)TheCamera->CarZoomIndicator) {
			// near
			case 1:
				if (anglesProfile == PROFILE_CUSTOM)
					zoomModeAlphaOffset = ZmOneAlphaOffsetCustom[alphaArrPos];
				else if (anglesProfile == PROFILE_VANILLA)
					zoomModeAlphaOffset = isVC() ? ZmOneAlphaOffsetVC[alphaArrPos] : ZmOneAlphaOffsetIII[alphaArrPos];
				else if (anglesProfile == PROFILE_LCS)
					zoomModeAlphaOffset = ZmOneAlphaOffsetLCS[alphaArrPos];
				else
					zoomModeAlphaOffset = ZmOneAlphaOffset[alphaArrPos];
				break;
			// mid
			case 2:
				if (anglesProfile == PROFILE_CUSTOM)
					zoomModeAlphaOffset = ZmTwoAlphaOffsetCustom[alphaArrPos];
				else if (anglesProfile == PROFILE_VANILLA)
					zoomModeAlphaOffset = isVC() ? ZmTwoAlphaOffsetVC[alphaArrPos] : ZmTwoAlphaOffsetIII[alphaArrPos];
				else if (anglesProfile == PROFILE_LCS)
					zoomModeAlphaOffset = ZmTwoAlphaOffsetLCS[alphaArrPos];
				else
					zoomModeAlphaOffset = ZmTwoAlphaOffset[alphaArrPos];
				break;
			// far
			case 3:
				if (anglesProfile == PROFILE_CUSTOM)
					zoomModeAlphaOffset = ZmThreeAlphaOffsetCustom[alphaArrPos];
				else if (anglesProfile == PROFILE_VANILLA)
					zoomModeAlphaOffset = isVC() ? ZmThreeAlphaOffsetVC[alphaArrPos] : ZmThreeAlphaOffsetIII[alphaArrPos];
				else if (anglesProfile == PROFILE_LCS)
					zoomModeAlphaOffset = ZmThreeAlphaOffsetLCS[alphaArrPos];
				else
					zoomModeAlphaOffset = ZmThreeAlphaOffset[alphaArrPos];
				break;
			default:
				break;
		}
	}

	ColModelClass* carCol = (ColModelClass*)car->GetColModel();
	float colMaxZ = carCol->boundingBox.max.z;  // As opposed to LCS and SA, VC does this: carCol->boundingBox.max.z - carCol->boundingBox.min.z;
	float approxCarLength = 2.0f * fabsf(carCol->boundingBox.min.y); // SA taxi min.y = -2.95, max.z = 0.883502f

	// Turned off by default on LCS_CAM
	if(heightIncreaseOnBike) {
		if (!isBike) {
			heightIncreaseMult = 0.0f;
		} else {
			// Increase colMaxZ slowly when there is a passenger on bike
			if (car->pPassengers[0])
			{
				if (heightIncreaseMult < 1.0f) {
					heightIncreaseMult = min(1.0f, ms_fTimeStep * 0.02f + heightIncreaseMult);
				}
			}
			else
			{
				if (heightIncreaseMult > 0.0f) {
					heightIncreaseMult = max(0.0f, heightIncreaseMult - ms_fTimeStep * 0.02f);
				}
			}
			colMaxZ += 0.4f * heightIncreaseMult;
		}
	}

	float hackedZoomValue = TheCamera->CarZoomValueSmooth;

	// Emulate the zoom values per veh. type in III!
	// Original values: 3.9 - far, 1.9 - mid, 0.05 - near
	// Reminder: We don't have near to far transitions, only far to near.
	if (vehicleSpecificZoom && isIII()) {
		if ((int)TheCamera->CarZoomIndicator == 3)
			hackedZoomValue = CarZoomModes[alphaArrPos + 2 * 5];
		else if ((int)TheCamera->CarZoomIndicator == 2) {
			hackedZoomValue = (CarZoomModes[alphaArrPos + 1 * 5]) +
							  (hackedZoomValue - 1.9f) * (CarZoomModes[alphaArrPos + 2 * 5] - CarZoomModes[alphaArrPos + 1 * 5]) / (3.9f - 1.9f);
		} else if ((int)TheCamera->CarZoomIndicator == 1) {
			hackedZoomValue = (CarZoomModes[alphaArrPos + 0 * 5]) +
				(hackedZoomValue - 0.05f) * (CarZoomModes[alphaArrPos + 1 * 5] - CarZoomModes[alphaArrPos + 0 * 5]) / (1.9f - 0.05f);
		}

		// Had to put this condition to prevent zooming even more in tunnels etc.
		if (hackedZoomValue < CarZoomModes[alphaArrPos])
			hackedZoomValue = CarZoomModes[alphaArrPos];
	}

	float newDistance;
	float minDistForThisCar;
	if (distanceProfile == PROFILE_VANILLA) {
		CVector dimensions = carCol->boundingBox.max - carCol->boundingBox.min;
		float baseDist = dimensions.Magnitude();
		if (isBike)
			baseDist *= 1.45f;
		newDistance = baseDist + 0.1f + hackedZoomValue;
		if (isVC()) {
			if (car->m_modelIndex == RcRaider || car->m_modelIndex == RcGoblin)
				newDistance += 6.0f;
			else if (!isReLCS && car->m_modelIndex == MI_VC_RCBARON)
				newDistance += 9.5f;
		}
		minDistForThisCar = min(baseDist * 0.6f, 3.5f);
	} else {
		float zoomDistOffset = vehicleSpecificZoom ? CARCAM_SET[camSetArrPos][1] : CARCAM_SET[0][1];
		newDistance = hackedZoomValue + zoomDistOffset + approxCarLength;
		if (isVC()) {
			if (car->m_modelIndex != RcRaider && car->m_modelIndex != RcGoblin) {
				if (!isReLCS && car->m_modelIndex == MI_VC_RCBARON)
					newDistance += 9.5f;
			} else
				newDistance += 6.0f;
		}
		minDistForThisCar = approxCarLength * CARCAM_SET[camSetArrPos][3];
	}

	if (elasticStringPhysics && (isCar || isBike || car->IsBoat())) {
		float forwardSpeed = DotProduct(car->m_vecMoveSpeed, car->GetForward()) * 180.0f;
		newDistance += clamp(forwardSpeed * (2.0f / 210.0f), -1.0f, 2.0f);
	}

	if (distanceProfile == PROFILE_VANILLA || anglesProfile == PROFILE_VANILLA || distanceProfile == PROFILE_CUSTOM || anglesProfile == PROFILE_CUSTOM) {
		float vehHeight = carCol->boundingBox.max.z - carCol->boundingBox.min.z;
		if (isBike) {
			TargetCoors += 0.6f * vehHeight * car->GetUp();
		}
		else if (isHeli && car->m_status != STATUS_PLAYER_REMOTE) {
			TargetCoors.x += 0.6f * car->GetUp().x * colMaxZ;
			TargetCoors.y += 0.6f * car->GetUp().y * colMaxZ;
			TargetCoors.z += 0.6f * car->GetUp().z * colMaxZ;
		}
		else {
			TargetCoors.z += (isVC() ? 0.8f * vehHeight : (vehHeight - 0.1f));
		}
	}
	else {
		if (!isHeli || car->m_status == STATUS_PLAYER_REMOTE) {
			float radiusToStayOutside = colMaxZ * CARCAM_SET[camSetArrPos][0] - CARCAM_SET[camSetArrPos][2];
			if (radiusToStayOutside > 0.0f) {
				TargetCoors.z += radiusToStayOutside;
				newDistance += radiusToStayOutside;
				zoomModeAlphaOffset += 0.3f / newDistance * radiusToStayOutside;
			}
		}
		else {
			// 0.6f = fTestShiftHeliCamTarget
			TargetCoors.x += 0.6f * car->GetUp().x * colMaxZ;
			TargetCoors.y += 0.6f * car->GetUp().y * colMaxZ;
			TargetCoors.z += 0.6f * car->GetUp().z * colMaxZ;
		}
	}

	float targetSlopeTilt = 0.0f;
	int effectivePitchTilt = pitchTilt;
	if (effectivePitchTilt == 2) {
		effectivePitchTilt = isVC() ? 1 : 0;
	}

	bool wheelsOnGround = isBike ? (GetMysteriousWheelRelatedThingBike(car) > 0) : (GetWheelsOnGround(car) > 0);
	if (effectivePitchTilt > 0 && (isCar || isBike) && wheelsOnGround) {
		float forwardSlope = atan2f(car->GetForward().z, car->GetForward().Magnitude2D());
		float deltaBeta = cam->Beta - (car->GetForward().Heading() - HALFPI);
		float behindCarNess = cosf(deltaBeta);
		float carAlpha = -forwardSlope * behindCarNess;

		if (effectivePitchTilt == 1) {
			// Authentic Vice City (reVC CCam::WorkOutCamHeight line 1695)
			// Downhill descents elevate higher / angle steeper down; uphill slopes are clamped to 0
			if (carAlpha < 0.0f)
				carAlpha = 0.0f;
			if (carAlpha > DEGTORAD(89.0f))
				carAlpha = DEGTORAD(89.0f);
			targetSlopeTilt = carAlpha;
		}
		else if (effectivePitchTilt == 3) {
			// Full symmetric tilt (tilts camera smoothly on both uphill and downhill slopes)
			targetSlopeTilt = clamp(carAlpha, -0.35f, 0.35f);
		}
	}

	if (effectivePitchTilt == 1) {
		float bufferTopSpeed = isBike ? 0.09f : 0.15f;
		float bufferStep = isBike ? 0.04f : 0.07f;
		WellBufferMe(targetSlopeTilt, &camPitchTilt, &camPitchTiltSpeed, bufferTopSpeed, bufferStep, true);
	}
	else if (effectivePitchTilt == 3) {
		WellBufferMe(targetSlopeTilt, &camPitchTilt, &camPitchTiltSpeed, 0.035f, 0.016f, false);
	}
	else {
		camPitchTilt = 0.0f;
		camPitchTiltSpeed = 0.0f;
	}

	if (isVC()) {
		((CCamVC*)cam)->m_fTilt = camPitchTilt;
		((CCamVC*)cam)->m_fTiltSpeed = camPitchTiltSpeed;
	}

	// SA sets CurrentTweakAngle to this value for RCGOBLIN. VC also adds 0.2f to it
	if (isVC() && car->m_modelIndex == RcGoblin)
		zoomModeAlphaOffset += 0.178997f;

	float minDistForVehType = CARCAM_SET[camSetArrPos][4];
	if ((int)TheCamera->CarZoomIndicator == 1 && (camSetArrPos < 2 || (distanceProfile == PROFILE_LCS && camSetArrPos == 7))) {
		minDistForVehType = minDistForVehType * 0.65f;
	}

	float nextDistance = max(newDistance, minDistForVehType);

	cam->CA_MAX_DISTANCE = newDistance;
	cam->CA_MIN_DISTANCE = 3.5f;

	float currentBaseFOV = (fovProfile == PROFILE_CUSTOM) ? customBaseFOV : DefaultFOV;
	float maxFOVAdd = (fovProfile == PROFILE_CUSTOM) ? customDynamicFOVMax : 30.0f;
	float fovStartSpeed = (fovProfile == PROFILE_CUSTOM) ? customDynamicFOVStartSpeed : 0.4f;

	if (cam->ResetStatics) {
		cam->FOV = currentBaseFOV;

		if (isIII()) {
			// GTA 3 has this in veh. camera
			if (TheCamera->m_bIdleOn)
				TheCamera->m_uiTimeWeEnteredIdle = m_snTimeInMilliseconds;
		}
	}
	else {
		if (dynamicSpeedFOV && (isCar || isBike)) {
			float forwardSpeed = DotProduct(car->GetForward(), car->m_vecMoveSpeed);
			if (forwardSpeed > fovStartSpeed)
				cam->FOV += (forwardSpeed - fovStartSpeed) * ms_fTimeStep;
		}

		if (cam->FOV > currentBaseFOV)
			// 0.98f: CAR_FOV_FADE_MULT
			cam->FOV = pow(0.98f, ms_fTimeStep) * (cam->FOV - currentBaseFOV) + currentBaseFOV;

		if (cam->FOV <= currentBaseFOV + maxFOVAdd)
		{
			if (cam->FOV < currentBaseFOV)
				cam->FOV = currentBaseFOV;
		} else
			cam->FOV = currentBaseFOV + maxFOVAdd;

		if (!dynamicSpeedFOV) {
			cam->FOV = currentBaseFOV;
		}
	}

	// WORKAROUND: I still don't know how looking behind works (m_bCamDirectlyInFront is unused in III, they seem to use m_bUseTransitionBeta)
	if (pad->GetLookBehindForCar())
		if (cam->DirectionWasLooking == LOOKING_FORWARD || !cam->LookingBehind)
			TheCamera->m_bCamDirectlyInFront = true;

	// Taken from RotCamIfInFrontCar, because we don't call it anymore
	if (!(pad->GetLookBehindForCar() || pad->GetLookBehindForPed() || pad->GetLookLeft() || pad->GetLookRight()))
		if (cam->DirectionWasLooking != LOOKING_FORWARD)
			TheCamera->m_bCamDirectlyBehind = true;

	// Called when we just entered the car, just started to look behind or returned back from looking left, right or behind
	if (cam->ResetStatics || TheCamera->m_bCamDirectlyBehind || TheCamera->m_bCamDirectlyInFront) {
		cam->ResetStatics = false;
		cam->Rotating = false;
		cam->m_bCollisionChecksOn = true;
		cam->f_Roll = 0.0f;
		cam->f_rollSpeed = 0.0f;
		// TheCamera.m_bResetOldMatrix = 1;

		// Garage exit cam is not working well in III...
		if (isIII() || !TheCamera->m_bJustCameOutOfGarage) // && !sthForScript)
		{
			cam->Alpha = 0.0f;
			cam->Beta = car->GetForward().Heading() - HALFPI;
			if (TheCamera->m_bCamDirectlyInFront) {
				cam->Beta += PI;
			}
		}

		cam->BetaSpeed = 0.0;
		cam->AlphaSpeed = 0.0;
		cam->Distance = newDistance;
		cam->DistanceSpeed = 0.0;
		camPitchTilt = targetSlopeTilt;
		camPitchTiltSpeed = 0.0f;

		cam->Front.x = -(cos(cam->Beta) * cos(cam->Alpha));
		cam->Front.y = -(sin(cam->Beta) * cos(cam->Alpha));
		cam->Front.z = sin(cam->Alpha);

		m_aTargetHistoryPosOne = TargetCoors - nextDistance * cam->Front;

		m_aTargetHistoryPosTwo = TargetCoors - newDistance * cam->Front;

		m_nCurrentHistoryPoints = 0;
		if (isIII() || !TheCamera->m_bJustCameOutOfGarage) // && !sthForScript)
			cam->Alpha = -zoomModeAlphaOffset - camPitchTilt;
	}

	cam->Front = TargetCoors - m_aTargetHistoryPosOne;
	cam->Front.Normalise();

	// Code that makes cam rotate around the car
	float camRightHeading = cam->Front.Heading() - HALFPI;
	if (camRightHeading < -PI)
		camRightHeading = camRightHeading + TWOPI;

	float desiredRightHeading;
	if (car->m_vecMoveSpeed.Magnitude2D() <= 0.02f)
		desiredRightHeading = camRightHeading;
	else
		desiredRightHeading = car->m_vecMoveSpeed.Heading() - HALFPI;

	if (desiredRightHeading < camRightHeading - PI)
		desiredRightHeading += TWOPI;
	else if (desiredRightHeading > camRightHeading + PI)
		desiredRightHeading -= TWOPI;

	float stiffnessMult = 1.0f;
	if (cameraStiffness >= 0.0f) {
		stiffnessMult = cameraStiffness;
	} else {
		if (masterProfile == PROFILE_LCS)
			stiffnessMult = 0.4f;
		else
			stiffnessMult = 0.25f;
	}

	float v70 = ms_fTimeStep * CARCAM_SET[camSetArrPos][10] * (stiffnessMult * 4.0f);
	float v153 = ms_fTimeStep * CARCAM_SET[camSetArrPos][11] * (stiffnessMult * 4.0f);

	float a6f = (car->m_vecMoveSpeed - DotProduct(car->m_vecMoveSpeed, cam->Front) * cam->Front).Magnitude();

	float v76 = min(1.0f, v70 * a6f) * (desiredRightHeading - camRightHeading);
	if (v76 <= v153)
	{
		if (v76 < -v153)
			v76 = -v153;
	}
	else
	{
		v76 = v153;
	}
	float targetBeta = camRightHeading + v76;

	while (targetBeta < cam->Beta - PI)
		targetBeta += TWOPI;
	while (targetBeta > cam->Beta + PI)
		targetBeta -= TWOPI;

	float carPosChange = (TargetCoors - m_aTargetHistoryPosTwo).Magnitude();
	if (carPosChange < newDistance && newDistance > minDistForThisCar) {
		newDistance = max(minDistForThisCar, carPosChange);
	}
	float maxAlphaAllowed = ANGLES_CARCAM_SET[camSetArrPos][13];

	// Originally this is to prevent camera enter into car while we're standing, but what about moving???
	// This is also original LCS and SA bug, or some attempt to fix lag. We'll never know

	// Fix camera enters into car bug by default
	if (fixTheBug || car->m_vecMoveSpeed.MagnitudeSqr() < 0.04f)
		if (distanceProfile != PROFILE_LCS || car->m_modelIndex != FireTruk)
			if (!isBike || GetMysteriousWheelRelatedThingBike(car) > 3)
				if (!isHeli && (!isPlane || GetWheelsOnGround(car))) {
					CVector out = CrossProduct(car->GetForward(), CVector(0.0f, 0.0f, 1.0f));
					out.Normalise();
					CVector v173 = CrossProduct(out, car->GetForward());
					v173.Normalise();
					float v83 = DotProduct(v173, cam->Front);
					if (v83 > 0.0)
					{
						float v88 = asinf(fabsf(sinf(cam->Beta - (car->GetForward().Heading() - HALFPI))));
						float v200;
						if (v88 <= atan2f(carCol->boundingBox.max.x, -carCol->boundingBox.min.y))
						{
							v200 = (1.5f - carCol->boundingBox.min.y) / cosf(v88);
						}
						else
						{
							float a6g = 1.2f + carCol->boundingBox.max.x;
							v200 = a6g / cos(max(0.0f, HALFPI - v88));
						}
						maxAlphaAllowed = cos(cam->Beta - (car->GetForward().Heading() - HALFPI)) * atan2f(car->GetForward().z, car->GetForward().Magnitude2D())
							+ atan2f(TargetCoors.z - car->GetPosition().z + GetHeightAboveRoad(car, ColModelClass), v200 * 1.2f);
						if (isCar && GetWheelsOnGround(car) > 1
							&& fabsf(DotProduct(car->m_vecTurnSpeed, car->GetForward())) < 0.05f)
						{
							maxAlphaAllowed += cosf(cam->Beta - (car->GetForward().Heading() - HALFPI) + HALFPI) * atan2f(car->GetRight().z, car->GetRight().Magnitude2D());
						}
					}
				}

	float targetAlpha;
	if (anglesProfile == PROFILE_VANILLA) {
		targetAlpha = -zoomModeAlphaOffset - camPitchTilt;
	}
	else {
		targetAlpha = asinf(clamp(cam->Front.z, -1.0f, 1.0f)) - zoomModeAlphaOffset - camPitchTilt;
	}
	if (targetAlpha <= maxAlphaAllowed)
	{
		if (targetAlpha < -ANGLES_CARCAM_SET[camSetArrPos][14])
			targetAlpha = -ANGLES_CARCAM_SET[camSetArrPos][14];
	}
	else
	{
		targetAlpha = maxAlphaAllowed;
	}
	float maxAlphaBlendAmount = ms_fTimeStep * CARCAM_SET[camSetArrPos][6];
	float targetAlphaBlendAmount = (1.0f - pow(CARCAM_SET[camSetArrPos][5], ms_fTimeStep)) * (targetAlpha - cam->Alpha);
	if (targetAlphaBlendAmount <= maxAlphaBlendAmount)
	{
		if (targetAlphaBlendAmount < -maxAlphaBlendAmount)
			targetAlphaBlendAmount = -maxAlphaBlendAmount;
	}
	else
	{
		targetAlphaBlendAmount = maxAlphaBlendAmount;
	}

	// Using GetCarGun(LR/UD) with Y-axis invert check will give us same unprocessed RightStick value as SA
	float stickX = -(pad->GetCarGunLeftRight());
	float stickY = pad->GetCarGunUpDown();

	// In SA this checks for m_bUseMouse3rdPerson so num2/num8 do not move camera
	// when Keyboard & Mouse controls are used. To work best with GInput, check for actual pad state instead
	const bool ginputHasPad = ginputPad->HasPadInHands();
	if (ginputLoaded == 2 ? !ginputHasPad : m_bUseMouse3rdPerson)
		stickY = 0.0f;
	else {
		// Added in r4. GInput doesn't hook VC's Y-axis invert option, so that was needed
		if (ginputHasPad && padSettings.InvertLook)
			stickY = -stickY;

		// Hidden Y-axis invert option in VC. just in case
		if (isVC())
			if (*(bool*)0xA10AF7)
				stickY = -stickY;
	}

	float v103 = cam->FOV * 0.0125f;

	float xMovement = fabsf(stickX) * (v103 * 0.071428575) * stickX * 0.007f * 0.007f;
	float yMovement = fabsf(stickY) * (v103 * 0.042857144) * stickY * 0.007f * 0.007f;

	bool correctAlpha = true;
	//	if (SA checks if we aren't in work car, why?) {
			if (!isCar || car->m_modelIndex != CarWithHydraulics) {
				correctAlpha = false;
			} else {
				xMovement = 0.0f;
				yMovement = 0.0f;
			}
	//	} else
	//		yMovement = 0.0;

	if (!nextDirectionIsForward) {
		yMovement = 0.0;
		xMovement = 0.0;
	}

	if (camSetArrPos == 0 || (distanceProfile == PROFILE_LCS && camSetArrPos == 7)) {
		// This is not working on cars as SA
		// Because III/VC doesn't have any buttons tied to LeftStick if you're not in Classic Configuration, using Dodo or using GInput/Pad, so :shrug:
		if (fabsf(pad->GetSteeringUpDown()) > 120.0f) {

			// OBJECTIVE_LEAVE_VEHICLE
			if (car->pDriver && GetPedObjective(car->pDriver) != (isIII() ? 13 : 16)) {
				yMovement += fabsf(pad->GetSteeringUpDown()) * (cam->FOV * 0.0125 * 0.042857144) * pad->GetSteeringUpDown() * 0.007f * 0.007f * 0.5;
			}
		}
	}

	if (yMovement > 0.0)
		yMovement = yMovement * 0.5;

	bool mouseChangesBeta = false;

	// FIX: Disable mouse movement in drive-by, it's buggy. Original SA bug.
	if (mouseFreeLook && m_bUseMouse3rdPerson && !GetDisablePlayerControls(pad) && nextDirectionIsForward)
	{
		float mouseY = CPad::NewMouseControllerState.y * 2.0f;
		float mouseX = CPad::NewMouseControllerState.x * -2.0f;

		// If you want an ability to toggle free cam while steering with mouse, you can add an OR after DisableMouseSteering.
		// There was a pad->NewState.m_bVehicleMouseLook in SA, which doesn't exists in III.

		if ((mouseX != 0.0 || mouseY != 0.0) && (m_bDisableMouseSteering))
		{
			float v113 = cam->FOV * 0.0125;
			yMovement = mouseY * v113 * GetMouseAccel(TheCamera); // Same as SA, horizontal sensitivity.
			cam->BetaSpeed = 0.0;
			cam->AlphaSpeed = 0.0;
			xMovement = mouseX * v113 * GetMouseAccel(TheCamera);
			targetAlpha = cam->Alpha;
			stepsLeftToChangeBetaByMouse = 1.0f * 50.0f;
			mouseChangesBeta = true;
		}
		else if (stepsLeftToChangeBetaByMouse > 0.0f)
		{
			// Finish rotation by decreasing speed when we stopped moving mouse
			cam->BetaSpeed = 0.0;
			cam->AlphaSpeed = 0.0;
			yMovement = 0.0;
			xMovement = 0.0;
			targetAlpha = cam->Alpha;
			stepsLeftToChangeBetaByMouse = max(0.0f, stepsLeftToChangeBetaByMouse - ms_fTimeStep);
			mouseChangesBeta = true;
		}
	}

	/*
	// That doesn't exist on LCS, and I didn't put much thought into it
	if (car->pPassengers[0] && car->pPassengers[0]->m_objective == OBJECTIVE_SOLICIT)
	{
		if (BYTE1(CTaskManager::GetActiveTask(&car->__parent.__parent.m_apPassengers[0]->intelligence->m_taskManager)[5].m_pParentTask) & 1)
		{
			if (maxAlphaAllowed - flt_8CCEC8 <= v7->Alpha)
				yMovement = 0.0;
			else
				yMovement = ms_fTimeStep * flt_8CCEC4;
		}
	} */
	if (correctAlpha) {
		if (previousMode != MODE_CAMONASTRING)
			alphaCorrected = false;

		if (!alphaCorrected && fabsf(zoomModeAlphaOffset + cam->Alpha) > 0.05f) {
			yMovement = (-zoomModeAlphaOffset - cam->Alpha) * 0.05f;
		} else
			alphaCorrected = true;
	}
	float alphaSpeedFromStickY = yMovement * CARCAM_SET[camSetArrPos][12];
	float betaSpeedFromStickX = xMovement * CARCAM_SET[camSetArrPos][12];
	float v117 = CARCAM_SET[camSetArrPos][9];
	float angleChangeStep = pow(CARCAM_SET[camSetArrPos][8], ms_fTimeStep);
	float targetBetaWithStickBlendAmount = betaSpeedFromStickX + (targetBeta - cam->Beta) / max(ms_fTimeStep, 1.0f);

	if (targetBetaWithStickBlendAmount < -v117)
		targetBetaWithStickBlendAmount = -v117;
	else if (targetBetaWithStickBlendAmount > v117)
		targetBetaWithStickBlendAmount = v117;

	float angleChangeStepLeft = 1.0 - angleChangeStep;
	cam->BetaSpeed = targetBetaWithStickBlendAmount * angleChangeStepLeft + angleChangeStep * cam->BetaSpeed;
	if (fabsf(cam->BetaSpeed) < 0.0001f)
		cam->BetaSpeed = 0.0;

	float v121;
	if (mouseChangesBeta)
		v121 = betaSpeedFromStickX;
	else
		v121 = ms_fTimeStep * cam->BetaSpeed;
	cam->Beta = v121 + cam->Beta;
	
	// SA:
	if (TheCamera->m_bJustCameOutOfGarage)
		cam->Beta = GetATanOfXY(cam->Front.x, cam->Front.y) + PI;

	if (cam->Beta < -PI)
		cam->Beta += TWOPI;
	else if (cam->Beta > PI)
		cam->Beta -= TWOPI;

	/* LCS:
	if (TheCamera->m_bJustCameOutOfGarage) {
		float v168 = atan2f(cam->Front.y, cam->Front.x);
		if (v168 < 0.0f)
			v168 = v168 + TWOPI;

		cam->Beta = v168 + PI;
	}

	cam->Beta = LimitRadianAngle(cam->Beta);
	if (cam->Beta < 0.0f)
		cam->Beta += TWOPI;
	*/

	if ((camSetArrPos <= 1 || (distanceProfile == PROFILE_LCS && camSetArrPos == 7)) && targetAlpha < cam->Alpha && carPosChange >= newDistance) {
		if (isCar && GetWheelsOnGround(car) > 1 ||
			isBike && GetMysteriousWheelRelatedThingBike(car) > 1)
				alphaSpeedFromStickY += (targetAlpha - cam->Alpha) * 0.075f;
	}

	cam->AlphaSpeed = angleChangeStepLeft * alphaSpeedFromStickY + angleChangeStep * cam->AlphaSpeed;
	float maxAlphaSpeed = v117;
	if (alphaSpeedFromStickY > 0.0f)
		maxAlphaSpeed = maxAlphaSpeed * 0.5;

	if (cam->AlphaSpeed <= maxAlphaSpeed)
	{
		float minAlphaSpeed = -maxAlphaSpeed;
		if (cam->AlphaSpeed < minAlphaSpeed)
			cam->AlphaSpeed = minAlphaSpeed;
	}
	else
	{
		cam->AlphaSpeed = maxAlphaSpeed;
	}

	if (fabsf(cam->AlphaSpeed) < 0.0001f)
		cam->AlphaSpeed = 0.0f;

	float alphaWithSpeedAccounted;
	if (mouseChangesBeta)
	{
		alphaWithSpeedAccounted = alphaSpeedFromStickY + targetAlpha;
		cam->Alpha += alphaSpeedFromStickY;
	}
	else
	{
		alphaWithSpeedAccounted = ms_fTimeStep * cam->AlphaSpeed + targetAlpha;
		cam->Alpha += targetAlphaBlendAmount;
	}

	if (cam->Alpha <= maxAlphaAllowed)
	{
		float minAlphaAllowed = -CARCAM_SET[camSetArrPos][14];
		if (minAlphaAllowed > cam->Alpha)
		{
			cam->Alpha = minAlphaAllowed;
			cam->AlphaSpeed = 0.0;
		}
	}
	else
	{
		cam->Alpha = maxAlphaAllowed;
		cam->AlphaSpeed = 0.0;
	}

	// Prevent unsignificant angle changes
	if (fabsf(lastAlpha - cam->Alpha) < 0.0001f)
		cam->Alpha = lastAlpha;

	lastAlpha = cam->Alpha;

	if (fabsf(lastBeta - cam->Beta) < 0.0001f)
		cam->Beta = lastBeta;

	lastBeta = cam->Beta;

	cam->Front.x = -(cos(cam->Beta) * cos(cam->Alpha));
	cam->Front.y = -(sin(cam->Beta) * cos(cam->Alpha));
	cam->Front.z = sin(cam->Alpha);

	// Steering camera wobble (authentic GTA III & Vice City roll & inertia)
	float targetRoll = 0.0f;
	bool manualCameraMovement = mouseChangesBeta || fabsf(stickX) > 0.05f || fabsf(stickY) > 0.05f || !nextDirectionIsForward;
	if (cameraWobble && (isCar || isBike || car->IsBoat() || isPlane) && !manualCameraMovement) {
		float forwardSpeed = DotProduct(car->m_vecMoveSpeed, car->GetForward()) * 180.0f;
		if (forwardSpeed > 210.0f)
			forwardSpeed = 210.0f;
		else if (forwardSpeed < -210.0f)
			forwardSpeed = -210.0f;

		float steer = (float)pad->GetSteeringLeftRight();
		float steerFactor = -(steer / 128.0f) * (forwardSpeed / 210.0f);

		CVector fwdTarget = car->GetForward();
		fwdTarget.Normalise();
		float angleDiff = acosf(clamp(fabsf(DotProduct(fwdTarget, cam->Front)), 0.0f, 1.0f));

		float tiltOvershoot = (isCar || isBike) ? 1.05f : (isPlane ? 1.0f : 0.0f);
		float maxRoll = cam->f_max_role_angle;
		if (maxRoll == 0.0f)
			maxRoll = DEGTORAD(5.0f);

		// Authentic vanilla attenuation using sin(AngleDiff)
		targetRoll = steerFactor * (DEGTORAD(10.0f) * tiltOvershoot + maxRoll) * sinf(angleDiff);
	}
	WellBufferMe(targetRoll, &cam->f_Roll, &cam->f_rollSpeed, 0.15f, 0.07f, false);

	cam->Distance = newDistance;
	cam->DistanceSpeed = 0.0f;

	cam->GetVectorsReadyForRW();
	if (cameraWobble && cam->f_Roll != 0.0f) {
		CVector unrolledUp(0.0f, 0.0f, 1.0f);
		if (fabsf(cam->Front.x) < 0.0001f && fabsf(cam->Front.y) < 0.0001f)
			cam->Front.x = 0.0001f;
		CVector right = CrossProduct(cam->Front, unrolledUp);
		right.Normalise();
		CVector up0 = CrossProduct(right, cam->Front);
		up0.Normalise();
		cam->Up = up0 * cosf(cam->f_Roll) - right * sinf(cam->f_Roll);
		cam->Up.Normalise();
	}
	TheCamera->m_bCamDirectlyBehind = false;
	TheCamera->m_bCamDirectlyInFront = false;

	cam->Source = TargetCoors - newDistance * cam->Front;

	// VCS camera shake. Ported from ThirteenAG's WidescreenFixesPack
	// (MIT licensed, see licenses/WidescreenFixesPack.txt).
	if (vcsCamShake && (isCar || isBike)) {
		float vehSpeed = car->m_vecMoveSpeed.Magnitude();
		if (vehSpeed > 0.65f) {
			float shakeFactor = (min(vehSpeed, 1.0f) - 0.65f) / 0.35f / 200.0f;
			int r = rand();
			cam->Source.x += ((r & 0xF) - 7) * shakeFactor;
			cam->Source.y += (((r >> 4) & 0xF) - 7) * shakeFactor;
			cam->Source.z += (((r >> 8) & 0xF) - 7) * shakeFactor;
		}
	}

	cam->m_cvecTargetCoorsForFudgeInter = TargetCoors;
	m_aTargetHistoryPosThree = m_aTargetHistoryPosOne;
	float v140 = alphaWithSpeedAccounted + zoomModeAlphaOffset;
	float v144 = -(cos(cam->Beta) * cos(v140));
	float v145 = -(sin(cam->Beta) * cos(v140));
	float v146 = sin(v140);

	m_aTargetHistoryPosOne.x = TargetCoors.x - v144 * nextDistance;
	m_aTargetHistoryPosOne.y = TargetCoors.y - v145 * nextDistance;
	m_aTargetHistoryPosOne.z = TargetCoors.z - v146 * nextDistance;

	m_aTargetHistoryPosTwo.x = TargetCoors.x - v144 * newDistance;
	m_aTargetHistoryPosTwo.y = TargetCoors.y - v145 * newDistance;
	m_aTargetHistoryPosTwo.z = TargetCoors.z - v146 * newDistance;

	// SA calls SetColVarsVehicle in here
	if (nextDirectionIsForward) {
		/* SA
			CWorld::pIgnoreEntity = car;
			TheCamera.m_nExtraEntitiesCount = 0;
			TheCamera.sub_50CDE0(car);
			CCamera::CameraColDetAndReact(Source, TargetCoors);
			TheCamera->ImproveNearClip(car, 0, Source, TargetCoors);
			CWorld::pIgnoreEntity = 0;
		*/
		// Instead of above code (because it's rather impossible) I will use LCS FollowCar and SA FollowPedWithMouse col. detection


		// Move cam if there are collisions
		float v206 = powf(0.99, ms_fTimeStep);
		flt_9BF250 = (v206 * flt_9BF250) + ((1.0f - v206) * car->m_vecMoveSpeed.Magnitude());

		CColPoint foundCol;
		CEntity* foundEnt;
		pIgnoreEntity = cam->CamTargetEntity;
		if (WorldClass::ProcessLineOfSight(TargetCoors, cam->Source, foundCol, foundEnt, true, flt_9BF250 < 0.1f, false, true, false, true, false))
		{
			float obstacleTargetDist = (TargetCoors - foundCol.point).Magnitude();
			float obstacleCamDist = newDistance - obstacleTargetDist;
			if (!foundEnt->IsPed() || obstacleCamDist <= 1.0f)
			{
				cam->Source = foundCol.point;
				cam->Distance = obstacleTargetDist;
				cam->DistanceSpeed = 0.0f;
				if (obstacleTargetDist < 1.2f)
				{
					RwCameraSetNearClipPlane(RwCamera, max(0.05f, obstacleTargetDist - 0.3f));
				}
			}
			else
			{
				if (!WorldClass::ProcessLineOfSight(foundCol.point, cam->Source, foundCol, foundEnt, true, flt_9BF250 < 0.1f, false, true, false, true, false))
				{
					float lessClip = obstacleCamDist - 0.35f;
					if (lessClip <= 0.9f)
						RwCameraSetNearClipPlane(RwCamera, lessClip);
					else
						RwCameraSetNearClipPlane(RwCamera, 0.9f);
				} else {
					obstacleTargetDist = (TargetCoors - foundCol.point).Magnitude();
					cam->Source = foundCol.point;
					cam->Distance = obstacleTargetDist;
					cam->DistanceSpeed = 0.0f;
					if (obstacleTargetDist < 1.2f)
					{
						float lessClip = obstacleTargetDist - 0.3f;
						if (lessClip >= 0.05f)
							RwCameraSetNearClipPlane(RwCamera, lessClip);
						else
							RwCameraSetNearClipPlane(RwCamera, 0.05f);
					}
				}
			}
		}
		pIgnoreEntity = nil;
		float nearClip = GetNearPlane();
		float radius = tanf(cam->FOV * 0.017453292f * 0.5f) * GetAspectRatio() * 1.1f;

		// If we're seeing blue hell due to camera intersects some surface, fix it.
		// SA and LCS have this unrolled.
		for (int i = 0;
			i <= 5 && WorldClass::TestSphereAgainstWorld((nearClip * cam->Front) + cam->Source, radius * nearClip, nil, true, true, false, true, false, false);
			i++) {

			CVector surfaceCamDist = ms_testSpherePoint.point - cam->Source;
			CVector v52 = DotProduct(surfaceCamDist, cam->Front) * cam->Front;
			float v86 = (surfaceCamDist - v52).Magnitude() / radius;

			if (v86 > nearClip)
				v86 = nearClip;
			if (v86 < 0.1)
				v86 = 0.1;
			if (nearClip > v86)
				RwCameraSetNearClipPlane(RwCamera, v86);

			if (v86 == 0.1f)
				cam->Source += (TargetCoors - cam->Source) * 0.3f;

			nearClip = GetNearPlane();
			radius = tanf(cam->FOV * 0.017453292f * 0.5f) * GetAspectRatio() * 1.1f;
		}
	}
	TheCamera->m_bCamDirectlyBehind = false;
	TheCamera->m_bCamDirectlyInFront = false;

	// ------- LCS specific part starts

	if (camSetArrPos == 5 && cam->Source.z < 1.0f) // RC Bandit and Baron
		cam->Source.z = 1.0f;

	// Obviously some specific place in LC
	if (isReLCS || isIII())
		if (cam->Source.x > 11.0f && cam->Source.x < 91.0f) {
			if (cam->Source.y > -680.0f && cam->Source.y < -600.0f && cam->Source.z < 24.4f)
				cam->Source.z = 24.4f;
		}

	if (!seeUnderwater) {
		// CCam::FixSourceAboveWaterLevel
		if (CameraTarget.z >= -2.0f) {
			float level = -6000.0;
			// +0.5f is needed for III
			if (CWaterLevel::GetWaterLevelNoWaves(cam->Source.x, cam->Source.y, cam->Source.z, &level)) {
				if (cam->Source.z < level + 0.5f)
					cam->Source.z = level + 0.5f;
			}
		}
	}

	cam->Front = TargetCoors - cam->Source;

	// -------- LCS specific part ends

	cam->GetVectorsReadyForRW();
	// SA
	// gTargetCoordsForLookingBehind = TargetCoors;
	lookingRelativelyLeft = false;
	lookingRelativelyRight = false;
	// SA code from CAutomobile::TankControl/FireTruckControl.
	if (modernTurretControl && (car->m_modelIndex == Tank || car->m_modelIndex == FireTruk)) {
		CVector hi = Multiply3x3(cam->Front, car->GetMatrix());

		// III/VC's firetruck turret angle is reversed
		float angleToFace = (car->m_modelIndex == FireTruk ? -hi.Heading() : hi.Heading());

		if (angleToFace <= *GetDoomAnglePtrLR(car) + PI) {
			if (angleToFace < *GetDoomAnglePtrLR(car) - PI)
				angleToFace = angleToFace + TWOPI;
		} else {
			angleToFace = angleToFace - TWOPI;
		}

		float neededTurn = angleToFace - *GetDoomAnglePtrLR(car);
		float turnPerFrame = ms_fTimeStep * (car->m_modelIndex == FireTruk ? 0.05f : 0.015f);
		if (neededTurn <= turnPerFrame) {
			if (neededTurn < -turnPerFrame)
				angleToFace = *GetDoomAnglePtrLR(car) - turnPerFrame;
		} else {
			angleToFace = turnPerFrame + *GetDoomAnglePtrLR(car);
		}

		if (car->m_modelIndex == Tank && *GetDoomAnglePtrLR(car) != angleToFace) {
			DMAudio.PlayOneShot(car->m_audioEntityId, (isIII() ? 26 : 28), fabsf(angleToFace - *GetDoomAnglePtrLR(car)));
		}
		*GetDoomAnglePtrLR(car) = angleToFace;

		if (*GetDoomAnglePtrLR(car) < -PI) {
			*GetDoomAnglePtrLR(car) += TWOPI;
		} else if (*GetDoomAnglePtrLR(car) > PI) {
			*GetDoomAnglePtrLR(car) -= TWOPI;
		}

		// Because firetruk turret also has Y movement
		if (car->m_modelIndex == FireTruk) {
			float alphaToFace = atan2f(hi.z, hi.Magnitude2D()) + 0.2617994f;
			float neededAlphaTurn = alphaToFace - *GetDoomAnglePtrUD(car);
			float alphaTurnPerFrame = ms_fTimeStep * 0.02f;

			if (neededAlphaTurn > alphaTurnPerFrame) {
				neededTurn = alphaTurnPerFrame;
				*GetDoomAnglePtrUD(car) = neededTurn + *GetDoomAnglePtrUD(car);
			} else {
				if (neededAlphaTurn >= -alphaTurnPerFrame) {
					*GetDoomAnglePtrUD(car) = alphaToFace;
				} else {
					*GetDoomAnglePtrUD(car) = *GetDoomAnglePtrUD(car) - alphaTurnPerFrame;
				}
			}

			float turretMinY = -0.34906587f;
			float turretMaxY = 0.34906587f;
			if (turretMinY <= *GetDoomAnglePtrUD(car)) {
				if (*GetDoomAnglePtrUD(car) > turretMaxY)
					*GetDoomAnglePtrUD(car) = turretMaxY;
			} else {
				*GetDoomAnglePtrUD(car) = turretMinY;
			}

			if (isReLCS) {
				// Actual rotating turret for RE:LCS
				// CAR_BUMP_REAR (firetruck turret lol) = 8
				if (GetVehicleComponent(car, 8)) {
					CMatrix mat;
					CVector pos;

					mat.Attach(RwFrameGetMatrix(GetVehicleComponent(car, 8)));
					pos = mat.GetPosition();
					mat.SetRotateZ(-(*GetDoomAnglePtrLR(car)));
					mat.GetPosition() = pos;
					mat.UpdateRW();
				}
			}
		}
	}
	else if (modernDriveBy && (cam->Mode == MODE_BEHINDBOAT || cam->Mode == MODE_CAMONASTRING) && !isHeli)
	{
		CVector hi = Multiply3x3(cam->Front, car->GetMatrix());

		if (hi.Heading() >= 0.5235987756 && hi.Heading() <= 2.617993878) // > 30 deg and < 150 deg
			lookingRelativelyLeft = true;
		else if (hi.Heading() <= -0.5235987756 && hi.Heading() >= -2.617993878) // < -30 and > -150 deg
			lookingRelativelyRight = true;
	}

	previousMode = cam->Mode;
}

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

#define currentMode (isIII() ? TheCameraIII->Cams[TheCameraIII->ActiveCam].Mode : TheCameraVC->Cams[TheCameraVC->ActiveCam].Mode)

// Needed for storing previous mode for some unknown alpha angle effect.
// Credits goes to The Hero - aap for reversing it (comments are belong to him)
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

void
CCamIII::Process_FollowCar_SA_III(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	Process_FollowCar_SA<CCamIII, CCameraIII, CVehicleIII, CWorldIII, CColModelIII>(CameraTarget, TargetOrientation, this, TheCameraIII);
}

void
CCamVC::Process_FollowCar_SA_VC(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	Process_FollowCar_SA<CCamVC, CCameraVC, CVehicleVC, CWorldVC, CColModelVC>(CameraTarget, TargetOrientation, this, TheCameraVC);
}

BOOL WINAPI
DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH) {
		dllModule = hInst;

		LoadSettings();

		/* Taken from SkyGFX
		if (GetAsyncKeyState(VK_F8) & 0x8000) {
			AllocConsole();
			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
		}
		*/

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
