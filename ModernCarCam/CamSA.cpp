#include "ModernCarCam.h"

// ---------------------------------------------------------------------------
// CamSA.cpp - the San Andreas family vehicle follow camera.
//
// This engine reproduces the SA-style "follow car" camera; the SA, LCS, VCS
// and IV profiles all run through it. Its look and handling are selected by the
// distance/angle/fov table profiles and the feature flags (anchoring,
// stiffness, wobble, slope tilt, dynamic FOV, elastic string, reverse look).
//
// The vanilla III/VC engines are still reachable from here: when anchoring is
// active and the game/profile is vanilla, control is handed to CamVanilla.cpp.
//
// The implementation follows the reversed SA/LCS camera and the re3/reVC
// behaviour for the III/VC fallback (https://github.com/Hezkore/hez-gta-re3;
// see licenses/re3.txt). VCS shake and dynamic FOV are ported from
// ThirteenAG's WidescreenFixesPack (MIT licensed, see
// licenses/WidescreenFixesPack.txt).
// ---------------------------------------------------------------------------

// SA exposes CVehicle::m_bEnableMouseSteering (the inverse of the III/VC
// m_bDisableMouseSteering the shared engine was written against).
inline bool MouseSteeringDisabled() {
	if (isSA())
		return !*(bool*)0xC1CC02; // CVehicle::m_bEnableMouseSteering
	return m_bDisableMouseSteering;
}

// Runs the authentic III/VC anchoring engine. Specialised to a no-op for SA:
// San Andreas has no "camera on a string" vehicle mode, so anchoring is
// ignored there (the SA follow camera always runs). The generic definition is
// only instantiated for III/VC, whose engines CamVanilla.cpp provides.
template<class CamClass, class CameraClass, class VehicleClass, class WorldClass, class ColModelClass>
void
RunVanillaAnchor(CameraClass* TheCamera, CamClass* cam, VehicleClass* car, const CVector& CameraTarget, float TargetOrientation)
{
	if (cam->Mode == MODE_BEHINDBOAT) {
		if (isVC())
			Process_BehindBoat_VC<CamClass, CameraClass, VehicleClass, WorldClass, ColModelClass>(TheCamera, cam, car, CameraTarget, TargetOrientation);
		else
			Process_BehindBoat_Vanilla<CamClass, CameraClass, VehicleClass, WorldClass>(TheCamera, cam, car, CameraTarget, TargetOrientation);
	} else {
		Process_Cam_On_A_String_Vanilla<CamClass, CameraClass, VehicleClass, WorldClass, ColModelClass>(TheCamera, cam, car, CameraTarget, TargetOrientation);
	}
}

template<>
void
RunVanillaAnchor<CCamSA, CCameraSA, CVehicleSA, CWorldSA, CColModelSA>(CCameraSA*, CCamSA*, CVehicleSA*, const CVector&, float)
{
}

template<class CamClass, class CameraClass, class VehicleClass, class WorldClass, class ColModelClass>
void
Process_FollowCar_SA(CameraClass* TheCamera, CamClass* cam, VehicleClass* car, const CVector& CameraTarget, float TargetOrientation)
{
	// Reminder: SA vehicle subclass 3 is heli, 4 is plane, class 9 is bike

	// Missing things on III CCam
	static CVector m_aTargetHistoryPosOne;
	static CVector m_aTargetHistoryPosTwo;
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

	// GTA IV (and other driver-centred cameras): shift the orbit target to the
	// driver's seat instead of the car centre. The offset is rotated by the car's
	// heading only (yaw), not its full matrix: body roll and road pitch were
	// rotating the "up"/"side" parts of the seat into the forward direction, so
	// the target wandered and the camera settled back onto the car centre after
	// a few turns. Yaw-only keeps the seat fixed relative to the car's heading.
	CVector adjustedTarget = CameraTarget;
	if (cameraDriverOffset.x != 0.0f || cameraDriverOffset.y != 0.0f || cameraDriverOffset.z != 0.0f) {
		const float h = car->GetForward().Heading();
		const CVector flatForward(-sinf(h), cosf(h), 0.0f);
		const CVector flatRight(cosf(h), sinf(h), 0.0f);
		adjustedTarget += flatRight * cameraDriverOffset.x
			+ flatForward * cameraDriverOffset.y
			+ CVector(0.0f, 0.0f, cameraDriverOffset.z);
	}

	// San Andreas has no "on a string" vehicle mode, so anchoring is ignored
	// there; the SA follow camera always runs.
	bool useAnchoring = !isSA() && ((cameraAnchoring == 1) || (cameraAnchoring == 2 && (masterProfile == PROFILE_VANILLA || (distanceProfile == PROFILE_VANILLA && anglesProfile == PROFILE_VANILLA))));
	if (useAnchoring) {
		RunVanillaAnchor<CamClass, CameraClass, VehicleClass, WorldClass, ColModelClass>(
			TheCamera, cam, car, adjustedTarget, TargetOrientation);
		return;
	}

	CVector TargetCoors = adjustedTarget;
	TargetCoors.z += cameraHeight;

	uint8 camSetArrPos = 0;

	// Vehicle classification differs per game, so it is delegated to the
	// per-game wrapper (handling flags on III/VC, eVehicleType on SA).
	bool isPlane = car->IsPlaneType();
	bool isHeli = car->IsHeliType();
	bool isBike = car->IsBikeType();
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

	if (isHeli && car->GetStatus() == STATUS_PLAYER_REMOTE) {
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
		float forwardSpeed = DotProduct(car->m_vecMoveSpeed, car->GetForward()) * SpeedKphFactor;
		newDistance += clamp(forwardSpeed * (2.0f / MaxForwardSpeed), -1.0f, 2.0f);
	}

	newDistance *= cameraDistanceScale;

	if (distanceProfile == PROFILE_VANILLA || anglesProfile == PROFILE_VANILLA || distanceProfile == PROFILE_CUSTOM || anglesProfile == PROFILE_CUSTOM) {
		float vehHeight = carCol->boundingBox.max.z - carCol->boundingBox.min.z;
		if (isBike) {
			TargetCoors += 0.6f * vehHeight * car->GetUp();
		}
		else if (isHeli && car->GetStatus() != STATUS_PLAYER_REMOTE) {
			TargetCoors.x += 0.6f * car->GetUp().x * colMaxZ;
			TargetCoors.y += 0.6f * car->GetUp().y * colMaxZ;
			TargetCoors.z += 0.6f * car->GetUp().z * colMaxZ;
		}
		else {
			TargetCoors.z += (isVC() ? 0.8f * vehHeight : (vehHeight - 0.1f));
		}
	}
	else {
		if (!isHeli || car->GetStatus() == STATUS_PLAYER_REMOTE) {
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
			// Vice City style: downhill elevates the camera, uphill is level.
			// Same range as the vanilla engine.
			targetSlopeTilt = clamp(carAlpha, 0.0f, 0.35f);
		}
		else if (effectivePitchTilt == 3) {
			// Full symmetric tilt. Use the same range and buffer as the vanilla
			// engine, otherwise the tilt creeps toward the slope far too slowly
			// to be visible (and the old 0.035 top speed looked like it did
			// nothing at all).
			targetSlopeTilt = clamp(carAlpha, -0.35f, 0.35f);
		}
	}

	if (effectivePitchTilt == 1 || effectivePitchTilt == 3) {
		float bufferTopSpeed = isBike ? 0.09f : 0.15f;
		float bufferStep = isBike ? 0.04f : 0.07f;
		WellBufferMe(targetSlopeTilt, &camPitchTilt, &camPitchTiltSpeed, bufferTopSpeed, bufferStep, true);
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

	// Reverse look-behind state (driven further below); reset on (re)entry.
	static float reverseTime = 0.0f;
	static int reverseState = 0;
	static float reverseBetaSpeed = 0.0f;

	// Called when we just entered the car, just started to look behind or returned back from looking left, right or behind
	if (cam->ResetStatics || TheCamera->m_bCamDirectlyBehind || TheCamera->m_bCamDirectlyInFront) {
		cam->ResetStatics = false;
		cam->Rotating = false;
		cam->m_bCollisionChecksOn = true;
		cam->f_Roll = 0.0f;
		cam->f_rollSpeed = 0.0f;
		// TheCamera.m_bResetOldMatrix = 1;

		// Garage exit cam is not working well in III...
		if (isIII() || !TheCamera->m_bJustCameOutOfGarage)
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
		reverseTime = 0.0f;
		reverseState = 0;
		reverseBetaSpeed = 0.0f;
		// Do not snap camPitchTilt to the raw slope target here: this branch also
		// runs when returning from look-left/right/behind, and the instant jump
		// was what read as the camera suddenly slamming to the top. The buffer
		// above now tracks the slope smoothly instead.
		camPitchTiltSpeed = 0.0f;

		cam->Front.x = -(cos(cam->Beta) * cos(cam->Alpha));
		cam->Front.y = -(sin(cam->Beta) * cos(cam->Alpha));
		cam->Front.z = sin(cam->Alpha);

		m_aTargetHistoryPosOne = TargetCoors - nextDistance * cam->Front;

		m_aTargetHistoryPosTwo = TargetCoors - newDistance * cam->Front;

		if (isIII() || !TheCamera->m_bJustCameOutOfGarage)
			cam->Alpha = -zoomModeAlphaOffset - camPitchTilt;
	}

	// ---- Reverse look-behind state (drives Beta below) ----
	// This owns cam->Beta while active so the normal follow logic does not also
	// integrate Beta every frame; that double update was the wobble that stopped
	// the camera from settling behind the car. Declared above, reset on (re)entry.
	const float reverseSpeed = DotProduct(car->GetForward(), car->m_vecMoveSpeed);
	if (reverseSpeed < -0.05f)
		reverseTime += ms_fTimeStep;
	else
		reverseTime = 0.0f;

	if (!nextDirectionIsForward) {
		// Player took over (look left/right/behind): abort the auto swing.
		reverseTime = 0.0f;
		reverseState = 0;
		reverseBetaSpeed = 0.0f;
	}

	if (reverseState != 0 && !reverseCam)
		reverseState = 0;
	else if (reverseCam && reverseTime > 1.0f)
		reverseState = 1;
	else if (reverseState == 1 && reverseSpeed > 0.05f)
		reverseState = 2;

	if (reverseState == 0)
		reverseBetaSpeed = 0.0f;

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

	// The camera's default pitch for every profile. Previously a non-vanilla
	// angles profile fed the camera's own angle back in via asin(Front.z), which
	// made the camera drift pitch endlessly.
	float targetAlpha = -zoomModeAlphaOffset - camPitchTilt;
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
	float stickX = (float)-(pad->GetCarGunLeftRight());
	float stickY = (float)pad->GetCarGunUpDown();

	// In SA this checks for m_bUseMouse3rdPerson so num2/num8 do not move camera
	// when Keyboard & Mouse controls are used. To work best with GInput, check for actual pad state instead.
	// On SA the vertical axis is always read, so the gamepad can look up and down.
	const bool ginputHasPad = ginputPad->HasPadInHands();
	if (!isSA() && (ginputLoaded == 2 ? !ginputHasPad : m_bUseMouse3rdPerson))
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

	// Exponential smoothing so the gamepad camera does not feel rough.
	static float smoothedStickX = 0.0f;
	static float smoothedStickY = 0.0f;
	float stickSmoothing = min(1.0f, ms_fTimeStep * 0.25f);
	smoothedStickX += (stickX - smoothedStickX) * stickSmoothing;
	smoothedStickY += (stickY - smoothedStickY) * stickSmoothing;
	stickX = smoothedStickX;
	stickY = smoothedStickY;

	const bool stickActive = fabsf(stickX) > 0.05f || fabsf(stickY) > 0.05f;

	float v103 = cam->FOV * 0.0125f;

	float xMovement = fabsf(stickX) * (v103 * 0.071428575) * stickX * 0.007f * 0.007f;
	float yMovement = fabsf(stickY) * (v103 * 0.042857144) * stickY * 0.007f * 0.007f;

	bool correctAlpha = true;
	if (!isCar || car->m_modelIndex != CarWithHydraulics) {
		correctAlpha = false;
	} else {
		xMovement = 0.0f;
		yMovement = 0.0f;
	}

	if (!nextDirectionIsForward) {
		yMovement = 0.0;
		xMovement = 0.0;
	}

	// The III/VC "steering nudges the camera" tweak reads the driver's ped
	// objective, which is a III/VC-only field layout. San Andreas does not need
	// it (its own camera handles it).
	if (!isSA() && (camSetArrPos == 0 || (distanceProfile == PROFILE_LCS && camSetArrPos == 7))) {
		// This is not working on cars as SA
		// Because III/VC doesn't have any buttons tied to LeftStick if you're not in Classic Configuration, using Dodo or using GInput/Pad, so :shrug:
		if (fabsf(pad->GetSteeringUpDown()) > 120.0f) {

			// OBJECTIVE_LEAVE_VEHICLE
			if (car->pDriver && GetPedObjective(car->pDriver) != (uint32)(isIII() ? 13 : 16)) {
				yMovement += fabsf(pad->GetSteeringUpDown()) * (cam->FOV * 0.0125 * 0.042857144) * pad->GetSteeringUpDown() * 0.007f * 0.007f * 0.5;
			}
		}
	}

	if (yMovement > 0.0)
		yMovement = yMovement * 0.5;

	bool mouseChangesBeta = false;

	// FIX: Disable mouse movement in drive-by, it's buggy. Original SA bug.
	// Free look is native to SA, so it runs there even when the mod's toggle is off.
	if ((mouseFreeLook || isSA()) && m_bUseMouse3rdPerson && !GetDisablePlayerControls(pad) && nextDirectionIsForward)
	{
		float mouseY = GetMouseControllerY() * 2.0f;
		float mouseX = GetMouseControllerX() * -2.0f;

		// If you want an ability to toggle free cam while steering with mouse, you can add an OR after DisableMouseSteering.
		// There was a pad->NewState.m_bVehicleMouseLook in SA, which doesn't exists in III.
		// SA keeps its native mouse look regardless of the mouse-steering option.
		if ((mouseX != 0.0 || mouseY != 0.0) && (isSA() || MouseSteeringDisabled()))
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

	// Gamepad right stick drives the camera directly (the III/VC free-look)
	// instead of through SA's damped speed blending, which fights the player.
	// This runs after the mouse block so a stick push is not cancelled by the
	// mouse hold from the previous frame.
	if ((mouseFreeLook || isSA()) && stickActive && !GetDisablePlayerControls(pad) && nextDirectionIsForward) {
		xMovement = fabsf(stickX) * (v103 * 0.071428575f) * stickX * 0.007f * 0.007f;
		yMovement = fabsf(stickY) * (v103 * 0.042857144f) * stickY * 0.007f * 0.007f;
		cam->BetaSpeed = 0.0f;
		cam->AlphaSpeed = 0.0f;
		targetAlpha = cam->Alpha;
		stepsLeftToChangeBetaByMouse = 50.0f;
		mouseChangesBeta = true;
	}

	// Ease back to the default camera angle after free look, like the III/VC
	// camera, instead of letting the native blend snap back to straight. The
	// scale ramps from a gentle 0.2 up to full once the hold has run out.
	static bool freeLookWasActive = false;
	static float freeLookReturn = 0.0f;
	const bool freeLookNow = mouseChangesBeta;
	if (freeLookWasActive && !freeLookNow)
		freeLookReturn = 1.0f;
	freeLookWasActive = freeLookNow;
	float freeLookReturnScale = 1.0f;
	if (freeLookReturn > 0.0f) {
		freeLookReturn = max(0.0f, freeLookReturn - ms_fTimeStep / 30.0f);
		freeLookReturnScale = 0.2f + 0.8f * (1.0f - freeLookReturn);
	}

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
	if (reverseState == 0) {
		cam->BetaSpeed = targetBetaWithStickBlendAmount * angleChangeStepLeft + angleChangeStep * cam->BetaSpeed;
		if (fabsf(cam->BetaSpeed) < 0.0001f)
			cam->BetaSpeed = 0.0f;

		float v121;
		if (mouseChangesBeta)
			v121 = betaSpeedFromStickX;
		else
			v121 = ms_fTimeStep * cam->BetaSpeed * freeLookReturnScale;
		cam->Beta = v121 + cam->Beta;
	}
	
	// SA:
	if (TheCamera->m_bJustCameOutOfGarage)
		cam->Beta = GetATanOfXY(cam->Front.x, cam->Front.y) + PI;

	if (cam->Beta < -PI)
		cam->Beta += TWOPI;
	else if (cam->Beta > PI)
		cam->Beta -= TWOPI;

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
		cam->Alpha += targetAlphaBlendAmount * freeLookReturnScale;
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

	// ---- Reverse look-behind: drive Beta with its own state and speed so it
	// settles smoothly behind the car instead of fighting the follow logic ----
	if (reverseState != 0) {
		float target = (reverseState == 1)
			? car->GetForward().Heading() - HALFPI + PI // look behind the car
			: car->GetForward().Heading() - HALFPI;     // back to behind the car
		while (target < cam->Beta - PI) target += TWOPI;
		while (target > cam->Beta + PI) target -= TWOPI;
		cam->BetaSpeed = 0.0f;
		WellBufferMe(target, &cam->Beta, &reverseBetaSpeed, 0.10f, 0.02f, true);
		lastBeta = cam->Beta;
		if (reverseState == 2 && fabsf(LimitRadianAngle(cam->Beta - target)) < 0.02f) {
			reverseState = 0;
			reverseBetaSpeed = 0.0f;
		}
	}

	cam->Front.x = -(cos(cam->Beta) * cos(cam->Alpha));
	cam->Front.y = -(sin(cam->Beta) * cos(cam->Alpha));
	cam->Front.z = sin(cam->Alpha);

	// Slight tilt when passing traffic very closely.
	float trafficWobbleTarget = 0.0f;
	if (trafficCamWobble) {
		CEntity* nearEnt = WorldClass::TestSphereAgainstWorld(car->GetPosition(), 2.2f, (CEntity*)car, false, true, false, false, false, false);
		if (nearEnt) {
			CVector side = CrossProduct(car->GetForward(), CVector(0.0f, 0.0f, 1.0f));
			side.Normalise();
			float lateral = DotProduct(nearEnt->GetPosition() - car->GetPosition(), side);
			trafficWobbleTarget = clamp(lateral * 0.07f, -0.05f, 0.05f);
		}
	}

	// Steering camera wobble (authentic GTA III & Vice City roll & inertia)
	float targetRoll = 0.0f;
	bool manualCameraMovement = mouseChangesBeta || fabsf(stickX) > 0.05f || fabsf(stickY) > 0.05f || !nextDirectionIsForward;
	if (cameraWobble && (isCar || isBike || car->IsBoat() || isPlane) && !manualCameraMovement) {
		float forwardSpeed = DotProduct(car->m_vecMoveSpeed, car->GetForward()) * SpeedKphFactor;
		if (forwardSpeed > MaxForwardSpeed)
			forwardSpeed = MaxForwardSpeed;
		else if (forwardSpeed < -MaxForwardSpeed)
			forwardSpeed = -MaxForwardSpeed;

		float steer = (float)pad->GetSteeringLeftRight();
		float steerFactor = -(steer / 128.0f) * (forwardSpeed / MaxForwardSpeed);

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
	targetRoll += trafficWobbleTarget;
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

	// GTA IV style side offset (over the shoulder).
	if (cameraLateralOffset != 0.0f) {
		CVector side = CrossProduct(car->GetForward(), CVector(0.0f, 0.0f, 1.0f));
		side.Normalise();
		cam->Source += side * cameraLateralOffset;
	}

	// VCS camera shake. Ported from ThirteenAG's WidescreenFixesPack
	// (MIT licensed, see licenses/WidescreenFixesPack.txt).
	if (vcsCamShake && (isCar || isBike)) {
		float vehSpeed = car->m_vecMoveSpeed.Magnitude();
		if (vehSpeed > VCSCamShakeStartSpeed) {
			float shakeFactor = (min(vehSpeed, VCSCamShakeFullSpeed) - VCSCamShakeStartSpeed) / VCSCamShakeRange / VCSCamShakeDivisor;
			int r = rand();
			cam->Source.x += ((r & 0xF) - 7) * shakeFactor;
			cam->Source.y += (((r >> 4) & 0xF) - 7) * shakeFactor;
			cam->Source.z += (((r >> 8) & 0xF) - 7) * shakeFactor;
		}
	}

	cam->m_cvecTargetCoorsForFudgeInter = TargetCoors;
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
		// Instead of SA's SetColVarsVehicle (it is rather impossible to call
		// safely) the LCS FollowCar / SA FollowPedWithMouse collision detection
		// is used below.

		// Move cam if there are collisions
		float v206 = powf(0.99, ms_fTimeStep);
		flt_9BF250 = (v206 * flt_9BF250) + ((1.0f - v206) * car->m_vecMoveSpeed.Magnitude());

		CColPoint foundCol;
		CEntity* foundEnt;
		pIgnoreEntity = (CEntity*)cam->CamTargetEntity;
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
	lookingRelativelyLeft = false;
	lookingRelativelyRight = false;
	// SA code from CAutomobile::TankControl/FireTruckControl. The turret angles,
	// vehicle component frames and audio entity live at III/VC offsets, so this
	// path is left out on SA (a documented limitation).
	if (!isSA() && modernTurretControl && (car->m_modelIndex == Tank || car->m_modelIndex == FireTruk)) {
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
	else if (!isSA() && modernDriveBy && (cam->Mode == MODE_BEHINDBOAT || cam->Mode == MODE_CAMONASTRING) && !isHeli)
	{
		CVector hi = Multiply3x3(cam->Front, car->GetMatrix());

		if (hi.Heading() >= 0.5235987756 && hi.Heading() <= 2.617993878) // > 30 deg and < 150 deg
			lookingRelativelyLeft = true;
		else if (hi.Heading() <= -0.5235987756 && hi.Heading() >= -2.617993878) // < -30 and > -150 deg
			lookingRelativelyRight = true;
	}

	previousMode = cam->Mode;
}

// ---------------------------------------------------------------------------
// Hook entry points. The game calls these CCam methods; they forward to the
// templated engine with this game's types and the entity the camera targets.
// ---------------------------------------------------------------------------
void
CCamIII::Process_FollowCar_SA_III(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	Process_FollowCar_SA<CCamIII, CCameraIII, CVehicleIII, CWorldIII, CColModelIII>(
		TheCameraIII, this, (CVehicleIII*)this->CamTargetEntity, CameraTarget, TargetOrientation);
}

void
CCamVC::Process_FollowCar_SA_VC(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	Process_FollowCar_SA<CCamVC, CCameraVC, CVehicleVC, CWorldVC, CColModelVC>(
		TheCameraVC, this, (CVehicleVC*)this->CamTargetEntity, CameraTarget, TargetOrientation);
}

// SA's CCam::Process_FollowCar_SA takes an extra trailing bool (sthForScript),
// which the hook ignores.
void
CCamSA::Process_FollowCar_SA_SA(const CVector &CameraTarget, float TargetOrientation, float, float, bool)
{
	Process_FollowCar_SA<CCamSA, CCameraSA, CVehicleSA, CWorldSA, CColModelSA>(
		TheCameraSA, this, (CVehicleSA*)this->CamTargetEntity, CameraTarget, TargetOrientation);
}
