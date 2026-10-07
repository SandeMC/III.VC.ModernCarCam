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

// Feature and tuning state.
bool cameraWobble = true;
bool elasticStringPhysics = true;
int  pitchTilt = 2;
bool dynamicSpeedFOV = false;
bool vcsCamShake = false;
int  cameraAnchoring = 2;
float cameraStiffness = -1.0f;
bool vehicleSpecificZoom = true;
bool modernTurretControl = true;
bool modernDriveBy = true;
bool mouseFreeLook = true;
bool heightIncreaseOnBike = true;
bool fixTheBug = true;
bool trafficCamWobble = true;
bool reverseCam = true;
bool seeUnderwater = false;
float cameraLateralOffset = 0.0f;
CVector cameraDriverOffset = CVector(0.0f, 0.0f, 0.0f);
float cameraDistanceScale = 1.0f;
bool enhancedVC = false;
float cameraHeight = 0.0f;

// Custom profile parameters (loaded from [Custom] in the ini).
float customDistNear = 0.05f;
float customDistMid = 1.9f;
float customDistFar = 3.9f;
float customDistOffset = 0.0f;
float customMinDistance = 10.0f;
float customCameraHeight = 0.0f;

float customBaseFOV = 70.0f;
float customDynamicFOVMax = 30.0f;
float customDynamicFOVStartSpeed = 0.4f;

float customAngleNear = -0.01f;
float customAngleMid = 0.045f;
float customAngleFar = 0.005f;
float customMaxElevationAngle = 0.785398f;
float customMinElevationAngle = 1.5533431f;
float customLateralOffset = 0.0f;

float customDistanceScale = 1.0f;
float customDriverOffsetX = 0.0f;
float customDriverOffsetY = 0.0f;
float customDriverOffsetZ = 0.0f;

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
		if (GetFileAttributesA(".\\III.VC.ModernCarCam.ini") != INVALID_FILE_ATTRIBUTES) {
			strcpy(iniPath, ".\\III.VC.ModernCarCam.ini");
		} else if (GetFileAttributesA(".\\scripts\\III.VC.ModernCarCam.ini") != INVALID_FILE_ATTRIBUTES) {
			strcpy(iniPath, ".\\scripts\\III.VC.ModernCarCam.ini");
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
	// overrides: omit a key to keep the profile's value.
	auto OverrideBool = [&](const char* key, bool& out) {
		int val = GetPrivateProfileIntA("Features", key, -1, iniPath);
		if (val != -1)
			out = (val != 0);
	};
	auto OverrideInt = [&](const char* key, int& out, int lo, int hi) {
		int val = GetPrivateProfileIntA("Features", key, INT_MIN, iniPath);
		if (val != INT_MIN)
			out = min(max(val, lo), hi);
	};

	auto ReadFloat = [&](const char* sec, const char* key, float def) -> float {
		char buf[32] = { 0 };
		GetPrivateProfileStringA(sec, key, "", buf, sizeof(buf), iniPath);
		return buf[0] ? (float)atof(buf) : def;
	};

	// Custom profile parameters
	customDistNear = ReadFloat("Custom", "CustomDistanceNear", 0.05f);
	customDistMid  = ReadFloat("Custom", "CustomDistanceMid", 1.9f);
	customDistFar  = ReadFloat("Custom", "CustomDistanceFar", 3.9f);
	customDistOffset = ReadFloat("Custom", "CustomDistanceOffset", 0.0f);
	customMinDistance = ReadFloat("Custom", "CustomMinDistance", 10.0f);
	customCameraHeight = ReadFloat("Custom", "CustomCameraHeight", 0.0f);

	customBaseFOV = ReadFloat("Custom", "CustomBaseFOV", 70.0f);
	customDynamicFOVMax = ReadFloat("Custom", "CustomMaxDynamicFOV", 30.0f);
	customDynamicFOVStartSpeed = ReadFloat("Custom", "CustomDynamicFOVStartSpeed", 0.4f);

	customAngleNear = ReadFloat("Custom", "CustomAngleNear", -0.01f);
	customAngleMid  = ReadFloat("Custom", "CustomAngleMid", 0.045f);
	customAngleFar  = ReadFloat("Custom", "CustomAngleFar", 0.005f);
	customMaxElevationAngle = ReadFloat("Custom", "CustomMaxElevationAngle", 0.785398f);
	customMinElevationAngle = ReadFloat("Custom", "CustomMinElevationAngle", 1.5533431f);
	customLateralOffset = ReadFloat("Custom", "CustomLateralOffset", 0.0f);

	customDistanceScale = ReadFloat("Custom", "CustomDistanceScale", 1.0f);
	customDriverOffsetX = ReadFloat("Custom", "CustomDriverOffsetX", 0.0f);
	customDriverOffsetY = ReadFloat("Custom", "CustomDriverOffsetY", 0.0f);
	customDriverOffsetZ = ReadFloat("Custom", "CustomDriverOffsetZ", 0.0f);

	// These offsets are only used by the Custom profile.
	if (cameraProfile == PROFILE_CUSTOM_CAM) {
		cameraLateralOffset = customLateralOffset;
		cameraHeight = customCameraHeight;
		cameraDistanceScale = customDistanceScale;
		cameraDriverOffset = CVector(customDriverOffsetX, customDriverOffsetY, customDriverOffsetZ);
	}

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

	// Optional feature overrides on top of the profile.
	OverrideBool("CameraWobble", cameraWobble);
	OverrideBool("ElasticStringPhysics", elasticStringPhysics);
	OverrideInt("PitchTilt", pitchTilt, 0, 3);
	OverrideBool("DynamicSpeedFOV", dynamicSpeedFOV);
	OverrideBool("VCSCamShake", vcsCamShake);
	OverrideInt("CameraAnchoring", cameraAnchoring, 0, 2);
	cameraStiffness = ReadFloat("Features", "CameraStiffness", cameraStiffness);
	OverrideBool("VehicleSpecificZoom", vehicleSpecificZoom);
	OverrideBool("ModernTurretControl", modernTurretControl);
	OverrideBool("ModernDriveBy", modernDriveBy);
	OverrideBool("MouseFreeLook", mouseFreeLook);
	OverrideBool("FixCameraClip", fixTheBug);
	OverrideBool("TrafficCamWobble", trafficCamWobble);
	OverrideBool("ReverseCamera", reverseCam);
	OverrideBool("BikesHeightIncrease", heightIncreaseOnBike);
	cameraHeight = ReadFloat("Features", "CameraHeight", cameraHeight);

	// Inverted relative to the internal "see underwater" flag.
	int keepWater = GetPrivateProfileIntA("Features", "KeepCameraOverWater", -1, iniPath);
	if (keepWater != -1)
		seeUnderwater = (keepWater == 0);
}
