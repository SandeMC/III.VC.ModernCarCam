#include "ModernCarCam.h"

// ---------------------------------------------------------------------------
// Settings.cpp - settings state and ini parsing.
//
// LoadSettings() reads the ini (next to the asi, or the shipped names), picks
// the user-facing profile, calls applyProfile() for the defaults, then applies
// the optional [General] table-profile overrides and [Features] overrides on
// top. A missing key always leaves the profile's value untouched.
// ---------------------------------------------------------------------------

// User-facing profile and the resolved internal table selectors.
ModernProfile cameraProfile = PROFILE_GAME_MATCHED;
CameraProfileType masterProfile = PROFILE_VANILLA;
CameraProfileType distanceProfile = PROFILE_VANILLA;
CameraProfileType fovProfile = PROFILE_VANILLA;
CameraProfileType anglesProfile = PROFILE_VANILLA;

// Feature and tuning state. Strength values are multipliers: 1.0 = the profile
// default, 0.0 = off, other positive values scale the effect.
float cameraWobble = 1.0f;
float elasticStringPhysics = 0.0f;
float pitchTilt = 0.0f;
float pitchTiltUphill = -1.0f;
float pitchTiltDownhill = -1.0f;
float pitchTiltMinAngle = 0.0f;   // set by applyProfile (2 degrees)
float pitchTiltAirHoldTime = 0.0f;  // set by applyProfile
float pitchTiltAirBlendTime = 0.0f; // set by applyProfile
float maxPitchAngle = -1.0f;
float minPitchAngle = -1.0f;
float dynamicSpeedFOV = 0.0f;
float dynamicSpeedFOVStartSpeed = -1.0f;
float dynamicSpeedFOVMaxFOV = -1.0f;
float vcsCamShake = 0.0f;
float vcsCamShakeStartSpeed = -1.0f;
float cameraAnchoring = 1.0f;
float cameraStiffness = 1.0f;
float headingFollow = 1.0f;
float vehicleSpecificZoom = 1.0f;
float trafficCamWobble = 0.0f;
float trafficCamWobbleMinSpeed = 0.0f;    // set by applyProfile
float trafficCamWobbleFullSpeed = 0.0f;   // set by applyProfile
float trafficCamWobbleMaxMultiplier = 0.0f; // set by applyProfile
float reverseCamDelay = 0.0f;             // set by applyProfile
float dynamicSpeedFOVDecay = 0.0f;        // set by applyProfile
float cameraReturnSpeed = 1.0f;
float cameraReturnTime = 0.5f;
bool modernTurretControl = true;
bool modernDriveBy = true;
bool lockShootDirKBM = true;
bool lockShootDirJOY = true;
bool mouseFreeLook = true;
bool heightIncreaseOnBike = true;
bool fixTheBug = true;
bool reverseCam = true;
bool seeUnderwater = false;
bool enhancedVC = false;
// Smooth side view: true swings the look left/right/behind smoothly, false uses
// the vanilla instant change.
bool smoothSideView = false;

// [Offsets] - applied independently of the selected profile.
float cameraHeightOffset = 0.0f;
float cameraLateralOffset = 0.0f;
float cameraDistanceOffset = 0.0f;
float cameraMinDistance = -1.0f;           // <0 = profile default
float cameraDistanceScale = 1.0f;
CVector cameraDriverOffset = CVector(0.0f, 0.0f, 0.0f);

// Custom profile shape (loaded from [Custom] in the ini).
float customDistNear = 0.05f;
float customDistMid = 1.9f;
float customDistFar = 3.9f;

float customBaseFOV = 70.0f;

float customAngleNear = -0.01f;
float customAngleMid = 0.045f;
float customAngleFar = 0.005f;

// GInput state.
IGInputPad* ginputPad;
int ginputLoaded = 0; // 1: not installed 2: installed
GINPUT_PAD_SETTINGS padSettings = {};

void OnGInputSettingsReload()
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
		if (GetFileAttributesA(".\\III.VC.SA.ModernCarCam.ini") != INVALID_FILE_ATTRIBUTES) {
			strcpy(iniPath, ".\\III.VC.SA.ModernCarCam.ini");
		} else if (GetFileAttributesA(".\\scripts\\III.VC.SA.ModernCarCam.ini") != INVALID_FILE_ATTRIBUTES) {
			strcpy(iniPath, ".\\scripts\\III.VC.SA.ModernCarCam.ini");
		}
	}

	char profile[32] = { 0 };
	GetPrivateProfileStringA("General", "Profile", "Game", profile, sizeof(profile), iniPath);
	if (_stricmp(profile, "Game") == 0 || _stricmp(profile, "Game-Matched") == 0) {
		cameraProfile = PROFILE_GAME_MATCHED;
	} else if (_stricmp(profile, "III") == 0) {
		cameraProfile = PROFILE_III;
	} else if (_stricmp(profile, "VC") == 0 || _stricmp(profile, "Vice City") == 0) {
		cameraProfile = PROFILE_VC;
	} else if (_stricmp(profile, "SA") == 0 || _stricmp(profile, "San Andreas") == 0) {
		cameraProfile = PROFILE_SA_CAM;
	} else if (_stricmp(profile, "Enhanced") == 0) {
		cameraProfile = PROFILE_ENHANCED;
	} else if (_stricmp(profile, "LCS") == 0) {
		cameraProfile = PROFILE_LCS_CAM;
	} else if (_stricmp(profile, "VCS") == 0) {
		cameraProfile = PROFILE_VCS;
	} else if (_stricmp(profile, "IV") == 0) {
		cameraProfile = PROFILE_IV;
	} else if (_stricmp(profile, "Custom") == 0) {
		cameraProfile = PROFILE_CUSTOM_CAM;
	} else {
		cameraProfile = PROFILE_GAME_MATCHED;
	}

	applyProfile(cameraProfile, isVC());

	// Individual table overrides. They follow the main Profile unless set.
	auto ParseTableProfile = [](const char* str, CameraProfileType def) -> CameraProfileType {
		if (!str || !*str)
			return def;
		if (_stricmp(str, "SA") == 0 || _stricmp(str, "IV") == 0)
			return PROFILE_SA;
		if (_stricmp(str, "LCS") == 0 || _stricmp(str, "VCS") == 0)
			return PROFILE_LCS;
		if (_stricmp(str, "Custom") == 0)
			return PROFILE_CUSTOM;
		return PROFILE_VANILLA; // Game-Matched / III / VC / Original
	};
	char tableBuf[32] = { 0 };
	GetPrivateProfileStringA("General", "DistanceProfile", "", tableBuf, sizeof(tableBuf), iniPath);
	distanceProfile = ParseTableProfile(tableBuf, distanceProfile);
	GetPrivateProfileStringA("General", "FOVProfile", "", tableBuf, sizeof(tableBuf), iniPath);
	fovProfile = ParseTableProfile(tableBuf, fovProfile);
	GetPrivateProfileStringA("General", "AnglesProfile", "", tableBuf, sizeof(tableBuf), iniPath);
	anglesProfile = ParseTableProfile(tableBuf, anglesProfile);

	// The profile already chose the defaults. These ini keys are optional
	// overrides: omit a key to keep the profile's value. Strength keys take a
	// multiplier (1 = the profile default, 0 = off).
	auto ReadFloat = [&](const char* sec, const char* key, float def) -> float {
		char buf[32] = { 0 };
		GetPrivateProfileStringA(sec, key, "", buf, sizeof(buf), iniPath);
		return buf[0] ? (float)atof(buf) : def;
	};
	auto OverrideFloat = [&](const char* key, float& out) {
		char buf[32] = { 0 };
		GetPrivateProfileStringA("Features", key, "", buf, sizeof(buf), iniPath);
		if (buf[0])
			out = (float)atof(buf);
	};
	auto OverrideBool = [&](const char* key, bool& out) {
		int val = GetPrivateProfileIntA("Features", key, -1, iniPath);
		if (val != -1)
			out = (val != 0);
	};

	// [Offsets] - applied on top of every profile, so they are additive (or
	// multiplicative, for the scale) deltas rather than replacements.
	cameraHeightOffset += ReadFloat("Offsets", "CameraHeightOffset", 0.0f);
	cameraLateralOffset += ReadFloat("Offsets", "CameraLateralOffset", 0.0f);
	cameraDistanceOffset += ReadFloat("Offsets", "CameraDistanceOffset", 0.0f);
	cameraDistanceScale *= ReadFloat("Offsets", "CameraDistanceScale", 1.0f);
	cameraDriverOffset += CVector(
		ReadFloat("Offsets", "CameraDriverOffsetX", 0.0f),
		ReadFloat("Offsets", "CameraDriverOffsetY", 0.0f),
		ReadFloat("Offsets", "CameraDriverOffsetZ", 0.0f));
	cameraMinDistance = ReadFloat("Offsets", "CameraMinDistance", -1.0f);

	// [Custom] profile shape.
	customDistNear = ReadFloat("Custom", "CustomDistanceNear", 0.05f);
	customDistMid  = ReadFloat("Custom", "CustomDistanceMid", 1.9f);
	customDistFar  = ReadFloat("Custom", "CustomDistanceFar", 3.9f);

	customBaseFOV = ReadFloat("Custom", "CustomBaseFOV", 70.0f);

	customAngleNear = ReadFloat("Custom", "CustomAngleNear", -0.01f);
	customAngleMid  = ReadFloat("Custom", "CustomAngleMid", 0.045f);
	customAngleFar  = ReadFloat("Custom", "CustomAngleFar", 0.005f);

	for (int i = 0; i < 8; i++) {
		CARCAM_SET_CUSTOM[i][1] = 0.0f; // distance offset now lives in [Offsets]
		CARCAM_SET_CUSTOM[i][4] = 10.0f;
		CARCAM_SET_CUSTOM[i][13] = 0.785398f;
		CARCAM_SET_CUSTOM[i][14] = 1.5533431f;
	}
	for (int i = 0; i < 5; i++) {
		CarZoomModesCustom[i] = customDistNear;
		CarZoomModesCustom[i + 5] = customDistMid;
		CarZoomModesCustom[i + 10] = customDistFar;
		ZmOneAlphaOffsetCustom[i] = customAngleNear;
		ZmTwoAlphaOffsetCustom[i] = customAngleMid;
		ZmThreeAlphaOffsetCustom[i] = customAngleFar;
	}

	// Optional feature overrides on top of the profile.
	OverrideFloat("CameraWobble", cameraWobble);
	OverrideFloat("ElasticStringPhysics", elasticStringPhysics);
	OverrideFloat("PitchTilt", pitchTilt);
	OverrideFloat("PitchTiltUphill", pitchTiltUphill);
	OverrideFloat("PitchTiltDownhill", pitchTiltDownhill);
	// The ini expresses the dead-zone in degrees (easy to reason about); the
	// engine works in radians like the rest of the pitch maths.
	{
		char buf[32] = { 0 };
		GetPrivateProfileStringA("Features", "PitchTiltMinAngle", "", buf, sizeof(buf), iniPath);
		if (buf[0])
			pitchTiltMinAngle = DEGTORAD((float)atof(buf));
	}
	OverrideFloat("PitchTiltAirHoldTime", pitchTiltAirHoldTime);
	OverrideFloat("PitchTiltAirBlendTime", pitchTiltAirBlendTime);
	OverrideFloat("MaxPitch", maxPitchAngle);
	OverrideFloat("MinPitch", minPitchAngle);
	OverrideFloat("DynamicSpeedFOV", dynamicSpeedFOV);
	OverrideFloat("DynamicSpeedFOVStartSpeed", dynamicSpeedFOVStartSpeed);
	OverrideFloat("DynamicSpeedFOVMaxFOV", dynamicSpeedFOVMaxFOV);
	OverrideFloat("VCSCamShake", vcsCamShake);
	OverrideFloat("VCSCamShakeStartSpeed", vcsCamShakeStartSpeed);
	OverrideFloat("CameraAnchoring", cameraAnchoring);
	OverrideFloat("CameraStiffness", cameraStiffness);
	OverrideFloat("HeadingFollow", headingFollow);
	OverrideFloat("VehicleSpecificZoom", vehicleSpecificZoom);
	OverrideFloat("TrafficCamWobble", trafficCamWobble);
	OverrideFloat("TrafficCamWobbleMinSpeed", trafficCamWobbleMinSpeed);
	OverrideFloat("TrafficCamWobbleFullSpeed", trafficCamWobbleFullSpeed);
	OverrideFloat("TrafficCamWobbleMaxMultiplier", trafficCamWobbleMaxMultiplier);
	OverrideFloat("ReverseCameraDelay", reverseCamDelay);
	OverrideFloat("DynamicSpeedFOVDecay", dynamicSpeedFOVDecay);
	OverrideFloat("CameraReturnSpeed", cameraReturnSpeed);
	OverrideFloat("CameraReturnTime", cameraReturnTime);
	OverrideBool("ModernTurretControl", modernTurretControl);
	OverrideBool("ModernDriveBy", modernDriveBy);
	OverrideBool("LockShootDirectionKBM", lockShootDirKBM);
	OverrideBool("LockShootDirectionJOY", lockShootDirJOY);
	OverrideBool("MouseFreeLook", mouseFreeLook);
	OverrideBool("FixCameraClip", fixTheBug);
	OverrideBool("ReverseCamera", reverseCam);
	OverrideBool("BikesHeightIncrease", heightIncreaseOnBike);
	OverrideBool("SmoothSideView", smoothSideView);

	// Inverted relative to the internal "see underwater" flag.
	int keepWater = GetPrivateProfileIntA("Features", "KeepCameraOverWater", -1, iniPath);
	if (keepWater != -1)
		seeUnderwater = (keepWater == 0);

	// Enhanced on San Andreas keeps the SA distance/FOV/angles, anchor and
	// stiffness even if the ini overrides them. The VC steering wobble and VCS
	// camera shake are off by default on SA but can still be enabled here.
	if (isSA() && cameraProfile == PROFILE_ENHANCED) {
		distanceProfile = PROFILE_SA;
		fovProfile = PROFILE_SA;
		anglesProfile = PROFILE_SA;
		cameraAnchoring = 0.0f;
		cameraStiffness = 1.0f;
	}
}
