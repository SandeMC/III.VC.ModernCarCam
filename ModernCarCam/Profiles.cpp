#include "ModernCarCam.h"

// ---------------------------------------------------------------------------
// Profiles.cpp - the camera profile model and the per-game camera tables.
//
// applyProfile() maps the user-facing ModernProfile onto the internal
// CameraProfileType table selectors and sets the default feature flags. The
// tables below are the authentic zoom/angle tables of SA, LCS, GTA III and
// Vice City; the Custom set is overwritten by LoadSettings() from the ini.
// ---------------------------------------------------------------------------

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

const float TiltOverShoot[] = { 1.05f, 1.05f, 0.0f, 0.0f, 1.0f };
const float TiltTopSpeed[]  = { 0.035f, 0.035f, 0.001f, 0.005f, 0.035f };
const float TiltSpeedStep[] = { 0.016f, 0.016f, 0.0002f, 0.0014f, 0.016f };

// Resolve the user-facing profile into the internal table selectors and the
// default feature set. Only the free camera, free turret control and fixes are
// added on top of the game-matched camera; everything else is opt-in.
void applyProfile(ModernProfile profile, bool vc) {
	switch (profile) {
	case PROFILE_SA_CAM:
	case PROFILE_IV:
		masterProfile = PROFILE_SA;
		break;
	case PROFILE_LCS_CAM:
	case PROFILE_VCS:
		masterProfile = PROFILE_LCS;
		break;
	case PROFILE_CUSTOM_CAM:
		masterProfile = PROFILE_CUSTOM;
		break;
	default:
		masterProfile = PROFILE_VANILLA;
		break;
	}
	distanceProfile = masterProfile;
	fovProfile = masterProfile;
	anglesProfile = masterProfile;

	// Game-matched baseline: reproduce the running game and only add the free
	// camera, free turret control and fixes.
	cameraWobble = vc;             // VC has steering roll, III does not
	elasticStringPhysics = false;
	pitchTilt = 2;                  // match game
	dynamicSpeedFOV = false;        // not native to III/VC
	vcsCamShake = false;
	cameraAnchoring = 2;            // match profile
	cameraStiffness = -1.0f;
	vehicleSpecificZoom = vc;
	modernTurretControl = true;
	modernDriveBy = true;
	mouseFreeLook = true;
	heightIncreaseOnBike = vc;
	fixTheBug = true;
	trafficCamWobble = false;       // a mod effect: not part of any vanilla camera, opt-in
	reverseCam = false;             // quality-of-life: Enhanced only, or forced in the ini
	seeUnderwater = false;
	cameraLateralOffset = 0.0f;
	cameraDriverOffset = CVector(0.0f, 0.0f, 0.0f);
	cameraDistanceScale = 1.0f;
	enhancedVC = false;
	cameraHeight = 0.0f;

	switch (profile) {
	case PROFILE_ENHANCED:
	case PROFILE_CUSTOM_CAM:
		// Game-matched camera plus quality-of-life additions.
		elasticStringPhysics = true;
		dynamicSpeedFOV = true;
		pitchTilt = 3;
		vcsCamShake = true;
		trafficCamWobble = true;
		if (profile == PROFILE_ENHANCED) {
			// Enhanced is the Vice City camera with the modern extras: adopt the
			// Vice City feature flags, camera angles, anchor and stiffness even in
			// GTA III, while the III-only engine behaviour (roof/ground height,
			// top-down camera, reversed turret) is left as-is.
			cameraWobble = true;
			vehicleSpecificZoom = true;
			heightIncreaseOnBike = true;
			cameraAnchoring = 1;      // authentic III/VC rigid anchor
			cameraStiffness = 1.0f;   // authentic vanilla stiffness
			reverseCam = true;
			enhancedVC = true;
		}
		break;
	case PROFILE_III:
		// The GTA III camera has no steering wobble, no per-vehicle zoom table
		// and no bike-passenger height; pin them to the III camera even when the
		// profile is forced while Vice City is running.
		cameraWobble = false;
		pitchTilt = 0;
		vehicleSpecificZoom = false;
		heightIncreaseOnBike = false;
		break;
	case PROFILE_VC:
		// Vice City's own camera feature set, also when forced on GTA III.
		cameraWobble = true;
		pitchTilt = 1;
		vehicleSpecificZoom = true;
		heightIncreaseOnBike = true;
		break;
	case PROFILE_VCS:
		vcsCamShake = true;
		cameraWobble = false;
		pitchTilt = 0;
		vehicleSpecificZoom = true;
		heightIncreaseOnBike = true;
		cameraAnchoring = 0;
		dynamicSpeedFOV = true;
		break;
	case PROFILE_SA_CAM:
		// San Andreas has no steering wobble and no VC-style slope tilt.
		cameraWobble = false;
		pitchTilt = 0;
		vehicleSpecificZoom = true;
		heightIncreaseOnBike = true;
		cameraAnchoring = 0;
		dynamicSpeedFOV = true;
		break;
	case PROFILE_LCS_CAM:
		// Liberty City Stories has no steering wobble or slope tilt either.
		cameraWobble = false;
		pitchTilt = 0;
		vehicleSpecificZoom = true;
		heightIncreaseOnBike = true;
		cameraAnchoring = 0;
		dynamicSpeedFOV = true;
		break;
	case PROFILE_IV:
		// IV has no steering wobble or VC-style slope tilt.
		cameraWobble = false;
		pitchTilt = 0;
		vehicleSpecificZoom = true;
		heightIncreaseOnBike = true;
		cameraAnchoring = 0;
		dynamicSpeedFOV = false;
		// GTA IV keeps the driver's seat centred on screen, so the orbit target
		// is the driver's seat (left, forward, up from the car centre). The height
		// was lowered because the IV profile sat too high in GTA III; fine-tune
		// with [Features] CameraHeight.
		cameraDriverOffset = CVector(-0.32f, 0.20f, 0.30f);
		cameraDistanceScale = 0.85f;
		break;
	default:
		break;
	}
}

const char *profileNames[] = { "Game", "III", "VC", "SA", "Enhanced", "LCS", "VCS", "IV", "Custom" };
const char *anchoringNames[] = { "Disabled (SA float)", "Enabled (Rigid anchor)", "Match profile" };
const char *pitchTiltNames[] = { "Disabled (Flat/III)", "Authentic VC (Downhill)", "Match game (Auto)", "Full symmetric" };
