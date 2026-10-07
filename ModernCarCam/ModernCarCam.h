#pragma once

// ---------------------------------------------------------------------------
// ModernCarCam - shared internal interface.
//
// This header is the private interface shared by the mod's translation units.
// ModernCarCam.cpp owns DllMain and the memory hooks; the rest of the project
// is split by concern:
//
//   Profiles.cpp      the camera tables and applyProfile() (the profile model)
//   Settings.cpp      ini parsing and the settings/feature state
//   CamVanilla.cpp    the vanilla "camera on a string" and behind-boat engines
//   CamSA.cpp         the San Andreas follow-camera engine
//   ModernCarCam.cpp  DllMain, hooks, addresses, shared maths helpers, debug menu
//
// Profile model
// -------------
// The user picks a single user-facing ModernProfile in the ini (Game, III, VC,
// SA, Enhanced, LCS, VCS, IV, Custom). applyProfile() maps that onto the
// internal CameraProfileType (PROFILE_SA, PROFILE_LCS, PROFILE_VANILLA,
// PROFILE_CUSTOM), which selects the per-game camera tables, and sets the
// default feature flags. [Features] keys then optionally override individual
// flags without changing the tables.
//
// Camera engines
// --------------
// Two families of engine exist because GTA III and Vice City (and their later
// ports Re:LCS / SA) implement the vehicle camera differently:
//
//   * CamVanilla.cpp holds the authentic III/VC "on a string" camera, the GTA
//     III top-down-style height pass, and the two behind-boat cameras.
//   * CamSA.cpp holds the San Andreas follow car camera used by the SA, LCS,
//     VCS and IV profiles.
//
// Both families are templated on the game-specific types (CamClass,
// CameraClass, VehicleClass, WorldClass and, where the collision model is
// needed, ColModelClass); this is how one algorithm is shared between GTA III
// and Vice City layouts. Their entry points take the same argument order:
//
//   (CameraClass* TheCamera, CamClass* cam, VehicleClass* car,
//    const CVector& CameraTarget, float TargetOrientation)
//
// ---------------------------------------------------------------------------

#include "common.h"
#include "MemoryMgr.h"
#include "GTA.h"
#include "Camera.h"
#include "Pad.h"
#include "GInputAPI.h"

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------

extern HMODULE dllModule;
extern int gtaversion;

// Reminder: isVC() also returns true for Re:LCS.
extern bool isReLCS;

// ---------------------------------------------------------------------------
// Profile model
// ---------------------------------------------------------------------------

// Internal camera table selector. This mirrors the original mod's table sets
// and is always derived from the user-facing ModernProfile.
enum CameraProfileType : int8_t {
	PROFILE_SA = 0,
	PROFILE_LCS = 1,
	PROFILE_VANILLA = 2,
	PROFILE_CUSTOM = 3
};

// User-facing profiles. "Game-Matched" reproduces whatever game is running;
// III/VC/SA/LCS/VCS/IV force a particular game's camera; "Enhanced" is the
// game-matched camera plus quality-of-life additions.
enum ModernProfile : int8_t {
	PROFILE_GAME_MATCHED = 0,
	PROFILE_III,
	PROFILE_VC,
	PROFILE_SA_CAM,
	PROFILE_ENHANCED,
	PROFILE_LCS_CAM,
	PROFILE_VCS,
	PROFILE_IV,
	PROFILE_CUSTOM_CAM
};

extern ModernProfile cameraProfile;
extern CameraProfileType masterProfile;
extern CameraProfileType distanceProfile;
extern CameraProfileType fovProfile;
extern CameraProfileType anglesProfile;

// Feature and tuning state. applyProfile() sets the defaults for the selected
// profile; LoadSettings() then applies any [Features] overrides.
extern bool cameraWobble;
extern bool elasticStringPhysics;
extern int  pitchTilt; // 0 = disabled (authentic III), 1 = authentic VC (downhill only), 2 = match game (1 in VC, 0 in III), 3 = full symmetric tilt
extern bool dynamicSpeedFOV;
extern bool vcsCamShake;
extern int  cameraAnchoring; // 0 = SA velocity follow, 1 = authentic III/VC rigid anchor, 2 = match profile
extern float cameraStiffness; // -1: match profile (1.0 for Vanilla, 0.25 for SA, 0.4 for LCS)
extern bool vehicleSpecificZoom;
extern bool modernTurretControl;
extern bool modernDriveBy;
extern bool mouseFreeLook;
extern bool heightIncreaseOnBike;
extern bool fixTheBug;
extern bool trafficCamWobble;
extern bool reverseCam;
extern bool seeUnderwater;
extern float cameraLateralOffset;          // side offset of the camera (Custom)
extern CVector cameraDriverOffset;         // target offset, e.g. the IV driver seat
extern float cameraDistanceScale;          // distance multiplier for the modern cameras
extern bool enhancedVC;                    // Enhanced uses Vice City's features + camera angles even in GTA III
extern float cameraHeight;                 // extra height in metres added to the camera target (Custom profile)

// Custom profile parameters (loaded from [Custom] in the ini).
extern float customDistNear;
extern float customDistMid;
extern float customDistFar;
extern float customDistOffset;
extern float customMinDistance;
extern float customCameraHeight;

extern float customBaseFOV;
extern float customDynamicFOVMax;
extern float customDynamicFOVStartSpeed;

extern float customAngleNear;
extern float customAngleMid;
extern float customAngleFar;
extern float customMaxElevationAngle;
extern float customMinElevationAngle;
extern float customLateralOffset;

// Debug-menu choice labels.
extern const char *profileNames[];
extern const char *anchoringNames[];
extern const char *pitchTiltNames[];

// ---------------------------------------------------------------------------
// Per-game camera tables
// ---------------------------------------------------------------------------

extern const float CarZoomModesSA[];
extern const float CarZoomModesLCS[];
extern const float CarZoomModesVC[];
extern const float CarZoomModesIII[];

// Alpha angles (up-down)
extern const float ZmOneAlphaOffset[];
extern const float ZmTwoAlphaOffset[];
extern const float ZmThreeAlphaOffset[];

extern const float ZmOneAlphaOffsetLCS[];
extern const float ZmTwoAlphaOffsetLCS[];
extern const float ZmThreeAlphaOffsetLCS[];

extern const float ZmOneAlphaOffsetVC[];
extern const float ZmTwoAlphaOffsetVC[];
extern const float ZmThreeAlphaOffsetVC[];

extern const float ZmOneAlphaOffsetIII[];
extern const float ZmTwoAlphaOffsetIII[];
extern const float ZmThreeAlphaOffsetIII[];

extern const float CARCAM_SET_SA[][15];
extern const float CARCAM_SET_LCS[][15];
extern const float CARCAM_SET_VANILLA[][15];

// The Custom table set is mutable: LoadSettings() fills it from the ini.
extern float CARCAM_SET_CUSTOM[8][15];
extern float CarZoomModesCustom[15];
extern float ZmOneAlphaOffsetCustom[5];
extern float ZmTwoAlphaOffsetCustom[5];
extern float ZmThreeAlphaOffsetCustom[5];

// Tilt table scaling used by the roll/wobble code.
extern const float TiltOverShoot[];
extern const float TiltTopSpeed[];
extern const float TiltSpeedStep[];

// ---------------------------------------------------------------------------
// GInput state
// ---------------------------------------------------------------------------

extern IGInputPad* ginputPad;
extern int ginputLoaded; // 1: not installed 2: installed
extern GINPUT_PAD_SETTINGS padSettings;

// ---------------------------------------------------------------------------
// Addresses and game globals
// ---------------------------------------------------------------------------

extern void (*&RwCamera);

// Actually static member of CCamera
extern bool& m_bUseMouse3rdPerson;
// Actually static member of CVehicle
extern bool& m_bDisableMouseSteering;

extern uint32& m_snTimeInMilliseconds;
extern float& ms_fTimeStep;

// These are static members of CWorld
extern CColPoint& ms_testSpherePoint;
extern CEntity*& pIgnoreEntity;

extern bool lookingRelativelyLeft;
extern bool lookingRelativelyRight;

// ---------------------------------------------------------------------------
// Vehicle model IDs and per-game class helpers
// ---------------------------------------------------------------------------

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

#define DefaultFOV 70.0f
#define DefaultNearClip 0.9f

// Shared speed constants for the roll/wobble and elastic-string terms. The
// vanilla engines express forward speed both in m/s and on a 180-unit scale
// (the GTA convention), clamped at 210.
constexpr float MaxForwardSpeed = 210.0f;
constexpr float SpeedKphFactor = 180.0f;

// VCS camera shake (ported from ThirteenAG's WidescreenFixesPack): starts at
// 0.65 and ramps to 1.0, scaled down by 200.
constexpr float VCSCamShakeStartSpeed = 0.65f;
constexpr float VCSCamShakeFullSpeed = 1.0f;
constexpr float VCSCamShakeRange = 0.35f;
constexpr float VCSCamShakeDivisor = 200.0f;

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

// ---------------------------------------------------------------------------
// Shared maths helpers
// ---------------------------------------------------------------------------

inline float LimitRadianAngle(float angle) {
	while (angle >= PI) angle -= TWOPI;
	while (angle < -PI) angle += TWOPI;
	return angle;
}

inline const CVector
Multiply3x3(const CMatrix& mat, const CVector& vec)
{
	return CVector(
		mat.m_matrix.right.x * vec.x + mat.m_matrix.up.x * vec.y + mat.m_matrix.at.x * vec.z,
		mat.m_matrix.right.y * vec.x + mat.m_matrix.up.y * vec.y + mat.m_matrix.at.y * vec.z,
		mat.m_matrix.right.z * vec.x + mat.m_matrix.up.z * vec.y + mat.m_matrix.at.z * vec.z);
}

inline const CVector
Multiply3x3(const CVector& vec, const CMatrix& mat)
{
	return CVector(
		mat.m_matrix.right.x * vec.x + mat.m_matrix.right.y * vec.y + mat.m_matrix.right.z * vec.z,
		mat.m_matrix.up.x * vec.x + mat.m_matrix.up.y * vec.y + mat.m_matrix.up.z * vec.z,
		mat.m_matrix.at.x * vec.x + mat.m_matrix.at.y * vec.y + mat.m_matrix.at.z * vec.z);
}

inline float GetATanOfXY(float x, float y) {
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

inline float GetAspectRatio() {
	if (isVC())
		return *(float*)0x94DD38; // CDraw::ms_fAspectRatio
	else {
		static float aspectRatio = 0.0f;
		static bool gotTheAR = false;
		if (!gotTheAR) {
			gotTheAR = true;

			RsGlobalType& RsGlobal = *(RsGlobalType*)0x8F4360;
			return aspectRatio = ((float)RsGlobal.width) / ((float)RsGlobal.height);
		} else
			return aspectRatio;
	}
}

inline float GetMouseAccel(CCameraVC *camera) {
	UNREFERENCED_PARAMETER(camera);
	return *(float*)0x94DBD0; // CCamera::m_fMouseAccelHorzntl
}

inline float GetMouseAccel(CCameraIII *camera) {
	return camera->m_fMouseAccelHorzntl;
}

inline bool IsVehicleSuspensionHigh(CCameraVC* camera) { return camera->m_bVehicleSuspenHigh; }
inline bool IsVehicleSuspensionHigh(CCameraIII*) { return false; }

// The engine step buffer; defined in ModernCarCam.cpp.
void WellBufferMe(float Target, float* CurrentValue, float* CurrentSpeed, float MaxSpeed, float Acceleration, bool IsAngle);

// RwCamera near clip; defined in ModernCarCam.cpp.
void* RwCameraSetNearClipPlane(void* camera, float nearClip);

// ---------------------------------------------------------------------------
// Settings / profiles / debug menu entry points
// ---------------------------------------------------------------------------

void applyProfile(ModernProfile profile, bool vc);
void LoadSettings();
void OnGInputSettingsReload();
void registerDebugMenu();
void onMasterProfileChange(void);

// ---------------------------------------------------------------------------
// Camera engine entry points
//
// The argument order is uniform across all engines:
//   (TheCamera, cam, car, CameraTarget, TargetOrientation).
// ColModelClass is only templated where the collision model's bounding box is
// needed (the behind-boat and cam-on-a-string engines do not require it).
// ---------------------------------------------------------------------------

template<class CamClass, class CameraClass, class VehicleClass, class WorldClass>
void Process_BehindBoat_Vanilla(CameraClass* TheCamera, CamClass* cam, VehicleClass* car, const CVector& CameraTarget, float TargetOrientation);

template<class CamClass, class CameraClass, class VehicleClass, class WorldClass, class ColModelClass>
void Process_BehindBoat_VC(CameraClass* TheCamera, CamClass* cam, VehicleClass* car, const CVector& CameraTarget, float TargetOrientation);

template<class CamClass, class CameraClass, class VehicleClass, class WorldClass, class ColModelClass>
void Process_Cam_On_A_String_Vanilla(CameraClass* TheCamera, CamClass* cam, VehicleClass* car, const CVector& CameraTarget, float TargetOrientation);

template<class CamClass, class CameraClass, class VehicleClass, class WorldClass, class ColModelClass>
void Process_FollowCar_SA(CameraClass* TheCamera, CamClass* cam, VehicleClass* car, const CVector& CameraTarget, float TargetOrientation);
