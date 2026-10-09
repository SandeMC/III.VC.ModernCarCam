#pragma once
#include "GTA.h"

// The camera memory layouts in this file are reconstructed from the GTAForums
// community and the re3 project; see licenses/re3.txt for attribution.

// Number of previous camera vectors kept for averaging.
#define NUMBER_OF_VECTORS_FOR_AVERAGE 2

// Camera modes used by CCam::Mode.
enum
{
	MODE_TOPDOWN1 = 1,
	MODE_TOPDOWN2,
	MODE_BEHINDCAR,
	MODE_FOLLOWPED,
	MODE_AIMING,
	MODE_DEBUG,
	MODE_SNIPER,
	MODE_ROCKET,
	MODE_MODELVIEW,
	MODE_BILL,
	MODE_SYPHON,
	MODE_CIRCLE,
	MODE_CHEESYZOOM,
	MODE_WHEELCAM,
	MODE_FIXED,
	MODE_FIRSTPERSON,
	MODE_FLYBY,
	MODE_CAMONASTRING,
	MODE_REACTIONCAM,
	MODE_FOLLOWPEDWITHBINDING,
	MODE_CHRISWITHBINDINGPLUSROTATION,
	MODE_BEHINDBOAT,
	MODE_PLAYERFALLENWATER,
	MODE_CAMONTRAINROOF,
	MODE_CAMRUNNINGSIDETRAIN,
	MODE_BLOODONTHETRACKS,
	MODE_IMTHEPASSENGERWOOWOO,
	MODE_SYPHONCRIMINFRONT,
	MODE_PEDSDEADBABY,
	MODE_CUSHYPILLOWSARSE,
	MODE_LOOKATCARS,
	MODE_ARRESTCAMONE,
	MODE_ARRESTCAMTWO,
	MODE_M16FIRSTPERSON_34,
	MODE_SPECIALFIXEDFORSYPHON,
	MODE_FIGHT,
	MODE_TOPDOWNPED,
	MODE_SNIPER_RUN_AROUND,
	MODE_ROCKET_RUN_AROUND,
	MODE_FIRSTPERSONPEDONPC_40,
	MODE_FIRSTPERSONPEDONPC_41,
	MODE_FIRSTPERSONPEDONPC_42,
	MODE_EDITOR,
	MODE_M16FIRSTPERSON_44
};

// Vice City per-camera state; one entry for each CCameraVC::Cams slot.
class CCamVC {
public:
	bool    bBelowMinDist; // True when the follow ped camera is closer than its minimum distance.
	bool    bBehindPlayerDesired; // True when the follow ped camera wants to sit behind the player.
	bool    m_bCamLookingAtVector;
	bool    m_bCollisionChecksOn;
	bool    m_bFixingBeta; // True when the camera on a string is fixing its beta.
	bool    m_bTheHeightFixerVehicleIsATrain;
	bool    LookBehindCamWasInFront;
	bool    LookingBehind;
	bool    LookingLeft; // 32
	bool    LookingRight;
	bool    ResetStatics; // True when the interpolation statics should be reset.
	bool    Rotating;

	short   Mode;                   // Camera mode (see the MODE_* enum).
	unsigned int  m_uiFinishTime; // 52

	int     m_iDoCollisionChecksOnFrameNum;
	int     m_iDoCollisionCheckEveryNumOfFrames;
	int     m_iFrameNumWereAt;  // 64
	int     m_iRunningVectorArrayPos;
	int     m_iRunningVectorCounter;
	int     DirectionWasLooking;

	float   f_max_role_angle; // Maximum roll angle (5 degrees).
	float   f_Roll; // Camera roll, used to add a slight lean in camera on a string mode.
	float   f_rollSpeed; // Roll speed, used to add a slight lean in camera on a string mode.
	float   m_fSyphonModeTargetZOffSet;
	float   m_fAmountFractionObscured;
	float   m_fAlphaSpeedOverOneFrame; // 100
	float   m_fBetaSpeedOverOneFrame;
	float   m_fBufferedTargetBeta;
	float   m_fBufferedTargetOrientation;
	float   m_fBufferedTargetOrientationSpeed;
	float   m_fCamBufferedHeight;
	float   m_fCamBufferedHeightSpeed;
	float   m_fCloseInPedHeightOffset;
	float   m_fCloseInPedHeightOffsetSpeed; // 132
	float   m_fCloseInCarHeightOffset;
	float   m_fCloseInCarHeightOffsetSpeed;
	float   m_fDimensionOfHighestNearCar;
	float   m_fDistanceBeforeChanges;
	float   m_fFovSpeedOverOneFrame;
	float   m_fMinDistAwayFromCamWhenInterPolating;
	float   m_fPedBetweenCameraHeightOffset;
	float   m_fPlayerInFrontSyphonAngleOffSet; // 164
	float   m_fRadiusForDead;
	float   m_fRealGroundDist; // Real ground distance, used by follow ped mode.
	float   m_fTargetBeta;
	float   m_fTimeElapsedFloat;
	float   m_fTilt;
	float   m_fTiltSpeed;

	float   m_fTransitionBeta;
	float   m_fTrueBeta;
	float   m_fTrueAlpha; // 200
	float   m_fInitialPlayerOrientation; // Player orientation captured when first person starts.

	float   Alpha;
	float   AlphaSpeed;
	float   FOV;
	float   FOVSpeed;
	float   Beta;
	float   BetaSpeed;
	float   Distance; // 232
	float   DistanceSpeed;
	float   CA_MIN_DISTANCE;
	float   CA_MAX_DISTANCE;
	float   SpeedVar;

	// Zoom distances used by the on-foot ped camera.
	float m_fTargetZoomGroundOne;
	float m_fTargetZoomGroundTwo; // 256
	float m_fTargetZoomGroundThree;
	// Alpha angle offsets used by the on-foot ped camera at each zoom level.
	float m_fTargetZoomOneZExtra;
	float m_fTargetZoomTwoZExtra;
	float m_fTargetZoomThreeZExtra;

	float m_fTargetZoomZCloseIn;
	float m_fMinRealGroundDist;
	float m_fTargetCloseInDist;

	CVector m_cvecSourceSpeedOverOneFrame; // 324
	CVector m_cvecTargetSpeedOverOneFrame; // 336
	CVector m_cvecUpOverOneFrame; // 348

	CVector m_cvecTargetCoorsForFudgeInter; // 360
	CVector m_cvecCamFixedModeVector; // 372
	CVector m_cvecCamFixedModeSource; // 384
	CVector m_cvecCamFixedModeUpOffSet; // 396
	CVector m_vecLastAboveWaterCamPosition; // 408: last camera position above water, used when the player goes underwater.

	CVector m_vecBufferedPlayerBodyOffset; // 420

	// The three vectors that define this camera for the current frame.
	CVector Front;  // 432: direction the camera is looking in.
	CVector Source; // Camera position in world space.
	CVector SourceBeforeLookBehind;
	CVector Up; // The camera's up vector.
	CVector m_arrPreviousVectors[NUMBER_OF_VECTORS_FOR_AVERAGE]; // The last few camera vectors, averaged to smooth the view.
	CEntity* CamTargetEntity;

	float       m_fCameraDistance;
	float       m_fIdealAlpha;
	float       m_fPlayerVelocity;
	CAutomobile* m_pLastCarEntered; // The last vehicle entered; kept so interpolation works.
	CPed* m_pLastPedLookedAt; // The last ped looked at; kept so interpolation works.
	bool        m_bFirstPersonRunAboutActive;

	void GetVectorsReadyForRW(void);
	void Process_FollowCar_SA_VC(const CVector&, float, float, float);
};

// GTA III per-camera state; one entry for each CCameraIII::Cams slot.
class CCamIII
{
public:
	bool    bBelowMinDist; // True when the follow ped camera is closer than its minimum distance.
	bool    bBehindPlayerDesired; // True when the follow ped camera wants to sit behind the player.
	bool    m_bCamLookingAtVector;
	bool    m_bCollisionChecksOn;
	bool    m_bFixingBeta; // True when the camera on a string is fixing its beta.
	bool    m_bTheHeightFixerVehicleIsATrain;
	bool    LookBehindCamWasInFront;
	bool    LookingBehind;
	bool    LookingLeft; // 32
	bool    LookingRight;
	bool    ResetStatics; // True when the interpolation statics should be reset.
	bool    Rotating;

	int16   Mode;                   // Camera mode (see the MODE_* enum).
	uint32  m_uiFinishTime; // 52

	int     m_iDoCollisionChecksOnFrameNum;
	int     m_iDoCollisionCheckEveryNumOfFrames;
	int     m_iFrameNumWereAt;  // 64
	int     m_iRunningVectorArrayPos;
	int     m_iRunningVectorCounter;
	int     DirectionWasLooking;

	float   f_max_role_angle; // Maximum roll angle (5 degrees).
	float   f_Roll; // Camera roll, used to add a slight lean in camera on a string mode.
	float	f_rollSpeed; // Roll speed, used to add a slight lean in camera on a string mode.
	float   m_fSyphonModeTargetZOffSet;
	float	m_fUnknownZOffSet;
	float   m_fAmountFractionObscured;
	float   m_fAlphaSpeedOverOneFrame; // 100
	float   m_fBetaSpeedOverOneFrame;
	float   m_fBufferedTargetBeta;
	float   m_fBufferedTargetOrientation;
	float   m_fBufferedTargetOrientationSpeed;
	float   m_fCamBufferedHeight;
	float   m_fCamBufferedHeightSpeed;
	float   m_fCloseInPedHeightOffset;
	float   m_fCloseInPedHeightOffsetSpeed; // 132
	float   m_fCloseInCarHeightOffset;
	float   m_fCloseInCarHeightOffsetSpeed;
	float   m_fDimensionOfHighestNearCar;
	float   m_fDistanceBeforeChanges;
	float   m_fFovSpeedOverOneFrame;
	float   m_fMinDistAwayFromCamWhenInterPolating;
	float   m_fPedBetweenCameraHeightOffset;
	float   m_fPlayerInFrontSyphonAngleOffSet; // 164
	float   m_fRadiusForDead;
	float   m_fRealGroundDist; // Real ground distance, used by follow ped mode.
	float   m_fTargetBeta;
	float   m_fTimeElapsedFloat;

	float   m_fTransitionBeta;
	float   m_fTrueBeta;
	float   m_fTrueAlpha; // 200
	float   m_fInitialPlayerOrientation; // Player orientation captured when first person starts.

	float   Alpha;
	float   AlphaSpeed;
	float   FOV;
	float   FOVSpeed;
	float   Beta;
	float   BetaSpeed;
	float   Distance; // 232
	float   DistanceSpeed;
	float   CA_MIN_DISTANCE;
	float   CA_MAX_DISTANCE;
	float   SpeedVar;

	// Zoom distances used by the on-foot ped camera.
	float m_fTargetZoomGroundOne;
	float m_fTargetZoomGroundTwo; // 256
	float m_fTargetZoomGroundThree;
	// Alpha angle offsets used by the on-foot ped camera at each zoom level.
	float m_fTargetZoomOneZExtra;
	float m_fTargetZoomTwoZExtra;
	float m_fTargetZoomThreeZExtra;

	float m_fTargetZoomZCloseIn;
	float m_fMinRealGroundDist;
	float m_fTargetCloseInDist;

	CVector m_cvecTargetCoorsForFudgeInter; // 360
	CVector m_cvecCamFixedModeVector; // 372
	CVector m_cvecCamFixedModeSource; // 384
	CVector m_cvecCamFixedModeUpOffSet; // 396
	CVector m_vecLastAboveWaterCamPosition; // 408: last camera position above water, used when the player goes underwater.
	CVector m_vecBufferedPlayerBodyOffset; // 420

	// The three vectors that define this camera for the current frame.
	CVector Front;  // 432: direction the camera is looking in.
	CVector Source; // Camera position in world space.
	CVector SourceBeforeLookBehind;
	CVector Up; // The camera's up vector.
	CVector m_arrPreviousVectors[NUMBER_OF_VECTORS_FOR_AVERAGE]; // The last few camera vectors, averaged to smooth the view.
	CEntity* CamTargetEntity;

	float       m_fCameraDistance;
	float       m_fIdealAlpha;
	float       m_fPlayerVelocity;
	CAutomobile* m_pLastCarEntered; // The last vehicle entered; kept so interpolation works.
	CPed* m_pLastPedLookedAt; // The last ped looked at; kept so interpolation works.
	bool        m_bFirstPersonRunAboutActive;

	void GetVectorsReadyForRW(void);
	void Process_FollowCar_SA_III(const CVector&, float, float, float);
};
static_assert(sizeof(CCamIII) == 0x1A4, "CCam: wrong size");

// Spline path data used by scripted camera paths.
struct CCamPathSplines
{
	float m_arr_PathData[800];
};

// One node of a scripted train camera path.
struct CTrainCamNode
{
	CVector m_cvecCamPosition;
	CVector m_cvecPointToLookAt;
	CVector m_cvecMinPointInRange;
	CVector m_cvecMaxPointInRange;
	float m_fDesiredFOV;
	float m_fNearClip;
};

// A camera mode queued for later, with its duration and zoom limits.
struct CQueuedMode
{
	int16 Mode;
	float Duration;
	int16 MinZoom;
	int16 MaxZoom;
};

// Look directions; stored in CCam::DirectionWasLooking.
enum
{
	LOOKING_BEHIND,
	LOOKING_LEFT,
	LOOKING_RIGHT,
	LOOKING_FORWARD,
};

enum
{
	FADE_0,
	FADE_1,	// Mid fade.
	FADE_2,

	FADE_OUT = 0,
	FADE_IN,
};

// Motion-blur presets. The intro presets are named after the cutscene they were
// authored for.
enum
{
	MBLUR_NONE,
	MBLUR_SNIPER,
	MBLUR_NORMAL,
	MBLUR_INTRO1,		// Used by the intro's green camera.
	MBLUR_INTRO2,		// Unused.
	MBLUR_INTRO3,		// Used by the bank scene.
	MBLUR_INTRO4,		// Used by the jail break scene.
	MBLUR_INTRO5,		// Used by the explosion.
	MBLUR_INTRO6,		// Used by the player being shot.
	MBLUR_UNUSED,		// Unused, pinkish tint.
};

// Vice City CCamera singleton layout; the mod reads and writes the fields it
// needs.
struct CCameraVC : public CPlaceableVC
{
	bool    m_bAboveGroundTrainNodesLoaded;
	bool    m_bBelowGroundTrainNodesLoaded;
	bool    m_bCamDirectlyBehind;
	bool    m_bCamDirectlyInFront;
	bool    m_bCameraJustRestored;
	bool    m_bcutsceneFinished;
	bool    m_bCullZoneChecksOn;
	bool    m_bFirstPersonBeingUsed; // True while the first person camera is in use.
	bool    m_bJustJumpedOutOf1stPersonBecauseOfTarget;
	bool    m_bIdleOn;
	bool    m_bInATunnelAndABigVehicle;
	bool    m_bInitialNodeFound;
	bool    m_bInitialNoNodeStaticsSet;
	bool    m_bIgnoreFadingStuffForMusic;
	bool    m_bPlayerIsInGarage;
	bool    m_bPlayerWasOnBike;
	bool    m_bJustCameOutOfGarage;
	bool    m_bJustInitalised;	// True on the first frame, so the speed buffer does not spike at startup.
	bool	m_bJust_Switched;	// True when the camera has just jumped to a new position.
	bool    m_bLookingAtPlayer;
	bool    m_bLookingAtVector;
	bool    m_bMoveCamToAvoidGeom;
	bool    m_bObbeCinematicPedCamOn;
	bool    m_bObbeCinematicCarCamOn;
	bool    m_bRestoreByJumpCut;
	bool    m_bUseNearClipScript;
	bool    m_bStartInterScript;
	bool	m_bStartingSpline;
	bool    m_bTargetJustBeenOnTrain;	// True when the target just boarded a train, needed to restore the camera.
	bool    m_bTargetJustCameOffTrain;
	bool    m_bUseSpecialFovTrain;
	bool    m_bUseTransitionBeta;
	bool    m_bUseScriptZoomValuePed;
	bool    m_bUseScriptZoomValueCar;
	bool    m_bWaitForInterpolToFinish;
	bool    m_bItsOkToLookJustAtThePlayer;	 // True when it is safe to look only at the player, used while interpolating.
	bool    m_bWantsToSwitchWidescreenOff;
	bool    m_WideScreenOn;
	bool    m_1rstPersonRunCloseToAWall;
	bool    m_bHeadBob;
	bool    m_bVehicleSuspenHigh;
	bool    m_bEnable1rstPersonCamCntrlsScript;

	bool    m_bAllow1rstPersonWeaponsCamera;
	bool    m_bFailedCullZoneTestPreviously;
	bool    m_FadeTargetIsSplashScreen;	// True when the fade target is a splash screen; a fading special case.
	bool    WorldViewerBeingUsed;	// True while the debug world viewer camera is in use.
	unsigned char   ActiveCam;
	uint8 somePad;
	unsigned int    m_uiCamShakeStart;          // Time the camera shake started.
	unsigned int    m_uiFirstPersonCamLastInputTime;
	unsigned int    m_uiLongestTimeInMill;
	unsigned int    m_uiNumberOfTrainCamNodes;

	unsigned char     m_uiTransitionJUSTStarted;  // True on the first frame of a transition.
	unsigned char     m_uiTransitionState;        // 0: a single mode; 1: a transition.
	uint8 somePad2;
	uint8 somePad3;
	unsigned int    m_uiTimeLastChange;
	unsigned int    m_uiTimeWeLeftIdle_StillNoInput;
	unsigned int    m_uiTimeWeEnteredIdle;
	unsigned int    m_uiTimeTransitionStart;    // Time the transition started.
	unsigned int    m_uiTransitionDuration;     // Duration of the transition.
	unsigned int    m_uiTransitionDurationTargetCoors;
	int     m_BlurBlue;
	int     m_BlurGreen;
	int     m_BlurRed;
	int     m_BlurType;
	int     m_iWorkOutSpeedThisNumFrames;
	int     m_iNumFramesSoFar;				// Frames counted so far.
	int     m_iCurrentTrainCamNode;			// Index of the train camera node currently in use.
	int     m_motionBlur;					// Active motion-blur preset (see the MBLUR_* enum).

	int     m_imotionBlurAddAlpha;
	int     m_iCheckCullZoneThisNumFrames;
	int     m_iZoneCullFrameNumWereAt;
	int     WhoIsInControlOfTheCamera;		// Whether the cinematic camera or a script controls the camera.
	float   CamFrontXNorm, CamFrontYNorm;
	float CarZoomIndicator; // Car zoom indicator (m_nCarZoom in SA).
	float CarZoomValue;		// Base car zoom value (m_nCarZoomBase in SA).
	float CarZoomValueSmooth; // Smoothed car zoom value (m_fCarZoomSmoothed in SA).
	float   DistanceToWater;
	float   FOVDuringInter;
	float   LODDistMultiplier;				 // LOD distance multiplier derived from the FOV and the standard LOD multiplier; a smaller aperture gives a larger multiplier.
	float   GenerationDistMultiplier;		// Generation distance multiplier derived from the FOV only.

	float   m_fAlphaSpeedAtStartInter;
	float   m_fAlphaWhenInterPol;
	float   m_fAlphaDuringInterPol;
	float   m_fBetaDuringInterPol;
	float   m_fBetaSpeedAtStartInter;
	float   m_fBetaWhenInterPol;
	float   m_fFOVWhenInterPol;
	float   m_fFOVSpeedAtStartInter;
	float   m_fStartingBetaForInterPol;
	float   m_fStartingAlphaForInterPol;
	float   m_PedOrientForBehindOrInFront;

	float   m_CameraAverageSpeed;		// Average camera speed over the sampled frames.
	float   m_CameraSpeedSoFar;		 // Running total of camera speed.
	float   m_fCamShakeForce;			 // Strength of the current camera shake.
	float	m_fCarZoomValueScript; // Scripted car zoom value (m_fCarZoomValueScript in SA).
	float   m_fFovForTrain;
	float   m_fFOV_Wide_Screen;
	float   m_fNearClipScript;
	float   m_fOldBetaDiff;						 // Previous beta difference, needed to interpolate between two modes.
	float   m_fPedZoomValue;
	float   m_fPedZoomValueSmooth;
	float   m_fPedZoomValueScript;
	float   m_fPositionAlongSpline;				// Progress along the spline, 0 at the start and 1 at the end.
	float   m_ScreenReductionPercentage;
	float   m_ScreenReductionSpeed;
	float   m_AlphaForPlayerAnim1rstPerson;
	float   Orientation;            // Camera orientation, used by peds walking.
	float   PedZoomIndicator;
	float   PlayerExhaustion;       // Player tiredness from 0.0 to 1.0, used for inaccurate sniping.
									// The fields below feed the sound code, which plays reverb based on the
									// surroundings. The distance from a point in front of the camera to the
									// nearest obstacle (a building) is measured for this purpose.
	float   SoundDistUp; // Buffered distance to the obstacle above the camera; the left and right variants are omitted here.
	float   SoundDistUpAsRead; // Distance above the camera as read by the audio code.
	float   SoundDistUpAsReadOld; // Previous frame's distance above the camera as read by the audio code.
									 // Very rough distance to the nearest water, for the audio code.
								  // Front vector X and Y components, normalised to length 1.


	float   m_fAvoidTheGeometryProbsTimer;
	short   m_nAvoidTheGeometryProbsDirn;

	float   m_fWideScreenReductionAmount;	// Widescreen reduction amount, 0 for none and 1 for fully reduced.
	float   m_fStartingFOVForInterPol;

	// The camera instances; usually only one or two are active. The third is
	// used for debugging, to look at the world, and its mode cannot be changed
	// because other objects depend on it.
	CCamVC Cams[3];

	void* pToGarageWeAreIn;
	void* pToGarageWeAreInForHackAvoidFirstPerson;
	CQueuedMode m_PlayerMode;

	// The higher-priority player camera mode, used for the sniper and rocket
	// launcher modes; it overrides m_PlayerMode above.
	CQueuedMode PlayerWeaponMode;
	CVector m_PreviousCameraPosition;		// Previous camera position, used to work out speed.
	CVector m_RealPreviousCameraPosition;	// Previous camera position visible to code outside the camera module; unlike m_PreviousCameraPosition it matches the current coordinates and can be used by an active camera for range finding.
	CVector m_cvecAimingTargetCoors;        // Target position the camera aims at.


	CVector m_vecFixedModeVector;
	CVector m_vecFixedModeSource;
	CVector m_vecFixedModeUpOffSet;
	CVector m_vecCutSceneOffset;

	CVector m_cvecStartingSourceForInterPol;
	CVector m_cvecStartingTargetForInterPol;
	CVector m_cvecStartingUpForInterPol;
	CVector m_cvecSourceSpeedAtStartInter;
	CVector m_cvecTargetSpeedAtStartInter;
	CVector m_cvecUpSpeedAtStartInter;
	CVector m_vecSourceWhenInterPol;
	CVector m_vecTargetWhenInterPol;
	CVector m_vecUpWhenInterPol;
	CVector m_vecClearGeometryVec;
	CVector m_vecGameCamPos;

	CVector SourceDuringInter, TargetDuringInter, UpDuringInter;
	// The RenderWare camera these vectors are applied to.
	void* m_pRwCamera;

	CEntity* pTargetEntity;
};
extern CCameraVC *TheCameraVC;

// GTA III CCamera singleton layout; the mod reads and writes the fields it needs.
struct CCameraIII : public CPlaceable
{
	bool m_bAboveGroundTrainNodesLoaded;
	bool m_bBelowGroundTrainNodesLoaded;
	bool m_bCamDirectlyBehind;
	bool m_bCamDirectlyInFront;
	bool m_bCameraJustRestored;
	bool m_bcutsceneFinished;
	bool m_bCullZoneChecksOn;
	bool m_bFirstPersonBeingUsed;
	bool m_bJustJumpedOutOf1stPersonBecauseOfTarget;
	bool m_bIdleOn;
	bool m_bInATunnelAndABigVehicle;
	bool m_bInitialNodeFound;
	bool m_bInitialNoNodeStaticsSet;
	bool m_bIgnoreFadingStuffForMusic;
	bool m_bPlayerIsInGarage;
	bool m_bJustCameOutOfGarage;
	bool m_bJustInitalised;
	bool m_bJust_Switched;
	bool m_bLookingAtPlayer;
	bool m_bLookingAtVector;
	bool m_bMoveCamToAvoidGeom;
	bool m_bObbeCinematicPedCamOn;
	bool m_bObbeCinematicCarCamOn;
	bool m_bRestoreByJumpCut;
	bool m_bUseNearClipScript;
	bool m_bStartInterScript;
	bool m_bStartingSpline;
	bool m_bTargetJustBeenOnTrain;
	bool m_bTargetJustCameOffTrain;
	bool m_bUseSpecialFovTrain;
	bool m_bUseTransitionBeta;
	bool m_bUseScriptZoomValuePed;
	bool m_bUseScriptZoomValueCar;
	bool m_bWaitForInterpolToFinish;
	bool m_bItsOkToLookJustAtThePlayer;
	bool m_bWantsToSwitchWidescreenOff;
	bool m_WideScreenOn;
	bool m_1rstPersonRunCloseToAWall;
	bool m_bHeadBob;
	bool m_bFailedCullZoneTestPreviously;

	bool m_FadeTargetIsSplashScreen;

	bool WorldViewerBeingUsed;
	uint8 ActiveCam;
	uint32 m_uiCamShakeStart;
	uint32 m_uiFirstPersonCamLastInputTime;
	// The VC layout also has m_bVehicleSuspenHigh,
	// m_bEnable1rstPersonCamCntrlsScript and m_bAllow1rstPersonWeaponsCamera
	// here; they are absent from the III layout.

	uint32 m_uiLongestTimeInMill;
	uint32 m_uiNumberOfTrainCamNodes;
	uint8   m_uiTransitionJUSTStarted;
	uint8   m_uiTransitionState;        // 0: a single mode; 1: a transition.

	uint32 m_uiTimeLastChange;
	uint32 m_uiTimeWeEnteredIdle;
	uint32 m_uiTimeTransitionStart;
	uint32 m_uiTransitionDuration;
	int m_BlurBlue;
	int m_BlurGreen;
	int m_BlurRed;
	int m_BlurType;

	uint32    unknown;
	int m_iWorkOutSpeedThisNumFrames;
	int m_iNumFramesSoFar;


	int m_iCurrentTrainCamNode;
	int m_motionBlur;
	int m_imotionBlurAddAlpha;
	int m_iCheckCullZoneThisNumFrames;
	int m_iZoneCullFrameNumWereAt;
	int WhoIsInControlOfTheCamera;

	float CamFrontXNorm;
	float CamFrontYNorm;
	float CarZoomIndicator; // Car zoom indicator (m_nCarZoom in SA).
	float CarZoomValue;		// Base car zoom value (m_nCarZoomBase in SA).
	float CarZoomValueSmooth; // Smoothed car zoom value (m_fCarZoomSmoothed in SA).

	float DistanceToWater;
	float FOVDuringInter;
	float LODDistMultiplier;
	float GenerationDistMultiplier;
	float m_fAlphaSpeedAtStartInter;
	float m_fAlphaWhenInterPol;
	float m_fAlphaDuringInterPol;
	float m_fBetaDuringInterPol;
	float m_fBetaSpeedAtStartInter;
	float m_fBetaWhenInterPol;
	float m_fFOVWhenInterPol;
	float m_fFOVSpeedAtStartInter;
	float m_fStartingBetaForInterPol;
	float m_fStartingAlphaForInterPol;
	float m_PedOrientForBehindOrInFront;
	float m_CameraAverageSpeed;
	float m_CameraSpeedSoFar;
	float m_fCamShakeForce;
	float m_fCarZoomValueScript; // Scripted car zoom value (m_fCarZoomValueScript in SA).
	float m_fFovForTrain;
	float m_fFOV_Wide_Screen;
	float m_fNearClipScript;
	float m_fOldBetaDiff;
	float m_fPedZoomValue;

	float m_fPedZoomValueScript;
	float m_fPedZoomValueSmooth;
	float m_fPositionAlongSpline;
	float m_ScreenReductionPercentage;
	float m_ScreenReductionSpeed;
	float m_AlphaForPlayerAnim1rstPerson;
	float Orientation;
	float PedZoomIndicator;
	float PlayerExhaustion;
	float SoundDistUp, SoundDistLeft, SoundDistRight;
	float SoundDistUpAsRead, SoundDistLeftAsRead, SoundDistRightAsRead;
	float SoundDistUpAsReadOld, SoundDistLeftAsReadOld, SoundDistRightAsReadOld;
	float m_fWideScreenReductionAmount;
	float m_fStartingFOVForInterPol;

	float m_fMouseAccelHorzntl; // Horizontal mouse acceleration multiplier for the first person camera.
	float m_fMouseAccelVertical; // Vertical mouse acceleration multiplier for the first person camera.
	float m_f3rdPersonCHairMultX;
	float m_f3rdPersonCHairMultY;


	CCamIII Cams[3];
	void* pToGarageWeAreIn;
	void* pToGarageWeAreInForHackAvoidFirstPerson;
	CQueuedMode m_PlayerMode;
	CQueuedMode PlayerWeaponMode;
	CVector m_PreviousCameraPosition;
	CVector m_RealPreviousCameraPosition;
	CVector m_cvecAimingTargetCoors;
	CVector m_vecFixedModeVector;
	CVector m_vecFixedModeSource;
	CVector m_vecFixedModeUpOffSet;
	CVector m_vecCutSceneOffset;

	CVector m_cvecStartingSourceForInterPol;
	CVector m_cvecStartingTargetForInterPol;
	CVector m_cvecStartingUpForInterPol;
	CVector m_cvecSourceSpeedAtStartInter;
	CVector m_cvecTargetSpeedAtStartInter;
	CVector m_cvecUpSpeedAtStartInter;
	CVector m_vecSourceWhenInterPol;
	CVector m_vecTargetWhenInterPol;
	CVector m_vecUpWhenInterPol;
	// The VC layout also has m_vecClearGeometryVec here; it is absent from III.

	CVector m_vecGameCamPos;
	CVector SourceDuringInter;
	CVector TargetDuringInter;
	CVector UpDuringInter;
	void* m_pRwCamera;
	CEntity* pTargetEntity;
	CCamPathSplines m_arrPathArray[4];
	CTrainCamNode m_arrTrainCamNode[800];
	CMatrix m_cameraMatrix;
	bool m_bGarageFixedCamPositionSet;
	bool m_vecDoingSpecialInterPolation;
	bool m_bScriptParametersSetForInterPol;
	bool m_bFading;
	bool m_bMusicFading;
	CMatrix m_viewMatrix;
	CVector m_vecFrustumNormals[4];
	CVector m_vecOldSourceForInter;
	CVector m_vecOldFrontForInter;
	CVector m_vecOldUpForInter;

	float m_vecOldFOVForInter;
	float m_fFLOATingFade;
	float m_fFLOATingFadeMusic;
	float m_fTimeToFadeOut;
	float m_fTimeToFadeMusic;
	float m_fFractionInterToStopMovingTarget;
	float m_fFractionInterToStopCatchUpTarget;
	float m_fGaitSwayBuffer;
	float m_fScriptPercentageInterToStopMoving;
	float m_fScriptPercentageInterToCatchUp;

	uint32  m_fScriptTimeForInterPolation;


	int16   m_iFadingDirection;
	int     m_iModeObbeCamIsInForCar;
	int16   m_iModeToGoTo;
	int16   m_iMusicFadingDirection;
	int16   m_iTypeOfSwitch;

	uint32 m_uiFadeTimeStarted;
	uint32 m_uiFadeTimeStartedMusic;

	virtual ~CCameraIII() { };
};
static_assert(sizeof(CCameraIII) == 0xE9D8, "CCameraIII: wrong size");
extern CCameraIII *TheCameraIII;

// ---------------------------------------------------------------------------
// San Andreas CCam / CCamera.
//
// Field order and offsets follow the plugin-sdk San Andreas CCam header; the
// field names used by the mod's shared engine are kept identical to the III/VC
// wrappers. static_assert guards the total size (0x238).
// ---------------------------------------------------------------------------
class CCamSA
{
public:
	bool    bBelowMinDist;                       // 0x00
	bool    bBehindPlayerDesired;                // 0x01
	bool    m_bCamLookingAtVector;               // 0x02
	bool    m_bCollisionChecksOn;                // 0x03
	bool    m_bFixingBeta;                       // 0x04
	bool    m_bTheHeightFixerVehicleIsATrain;    // 0x05
	bool    LookBehindCamWasInFront;             // 0x06
	bool    LookingBehind;                       // 0x07
	bool    LookingLeft;                         // 0x08
	bool    LookingRight;                        // 0x09
	bool    ResetStatics;                        // 0x0A
	bool    Rotating;                            // 0x0B
	int16   Mode;                                // 0x0C (eCamMode)
	uint8   _padMode[2];                         // 0x0E
	uint32  m_uiFinishTime;                      // 0x10
	uint32  m_iDoCollisionChecksOnFrameNum;      // 0x14
	uint32  m_iDoCollisionCheckEveryNumOfFrames; // 0x18
	uint32  m_iFrameNumWereAt;                   // 0x1C
	uint32  m_iRunningVectorArrayPos;            // 0x20
	uint32  m_iRunningVectorCounter;             // 0x24
	uint32  DirectionWasLooking;                 // 0x28
	float   f_max_role_angle;                    // 0x2C
	float   f_Roll;                              // 0x30
	float   f_rollSpeed;                         // 0x34
	float   m_fSyphonModeTargetZOffSet;          // 0x38
	float   m_fAmountFractionObscured;           // 0x3C
	float   m_fAlphaSpeedOverOneFrame;           // 0x40
	float   m_fBetaSpeedOverOneFrame;            // 0x44
	float   m_fBufferedTargetBeta;               // 0x48
	float   m_fBufferedTargetOrientation;        // 0x4C
	float   m_fBufferedTargetOrientationSpeed;   // 0x50
	float   m_fCamBufferedHeight;                // 0x54
	float   m_fCamBufferedHeightSpeed;           // 0x58
	float   m_fCloseInPedHeightOffset;           // 0x5C
	float   m_fCloseInPedHeightOffsetSpeed;      // 0x60
	float   m_fCloseInCarHeightOffset;           // 0x64
	float   m_fCloseInCarHeightOffsetSpeed;      // 0x68
	float   m_fDimensionOfHighestNearCar;        // 0x6C
	float   m_fDistanceBeforeChanges;            // 0x70
	float   m_fFovSpeedOverOneFrame;             // 0x74
	float   m_fMinDistAwayFromCamWhenInterPolating; // 0x78
	float   m_fPedBetweenCameraHeightOffset;     // 0x7C
	float   m_fPlayerInFrontSyphonAngleOffSet;   // 0x80
	float   m_fRadiusForDead;                    // 0x84
	float   m_fRealGroundDist;                   // 0x88
	float   m_fTargetBeta;                       // 0x8C
	float   m_fTimeElapsedFloat;                 // 0x90
	float   m_fTilt;                             // 0x94
	float   m_fTiltSpeed;                        // 0x98
	float   m_fTransitionBeta;                   // 0x9C
	float   m_fTrueBeta;                         // 0xA0
	float   m_fTrueAlpha;                        // 0xA4
	float   m_fInitialPlayerOrientation;         // 0xA8
	float   Alpha;                               // 0xAC (m_fVerticalAngle)
	float   AlphaSpeed;                          // 0xB0
	float   FOV;                                 // 0xB4
	float   FOVSpeed;                            // 0xB8
	float   Beta;                                // 0xBC (m_fHorizontalAngle)
	float   BetaSpeed;                           // 0xC0
	float   Distance;                            // 0xC4
	float   DistanceSpeed;                       // 0xC8
	float   CA_MIN_DISTANCE;                     // 0xCC
	float   CA_MAX_DISTANCE;                     // 0xD0
	float   SpeedVar;                            // 0xD4
	float   m_fCameraHeightMultiplier;           // 0xD8
	float   m_fTargetZoomGroundOne;              // 0xDC
	float   m_fTargetZoomGroundTwo;              // 0xE0
	float   m_fTargetZoomGroundThree;            // 0xE4
	float   m_fTargetZoomOneZExtra;              // 0xE8
	float   m_fTargetZoomTwoZExtra;              // 0xEC
	float   m_fTargetZoomTwoInteriorZExtra;      // 0xF0
	float   m_fTargetZoomThreeZExtra;            // 0xF4
	float   m_fTargetZoomZCloseIn;               // 0xF8
	float   m_fMinRealGroundDist;                // 0xFC
	float   m_fTargetCloseInDist;                // 0x100
	float   m_fBeta_Targeting;                   // 0x104
	float   m_fX_Targetting;                     // 0x108
	float   m_fY_Targetting;                     // 0x10C
	void*   m_pCarWeAreFocussingOn;              // 0x110
	void*   m_pCarWeAreFocussingOnI;             // 0x114
	float   m_fCamBumpedHorz;                    // 0x118
	float   m_fCamBumpedVert;                    // 0x11C
	uint32  m_nCamBumpedTime;                    // 0x120
	CVector m_cvecSourceSpeedOverOneFrame;       // 0x124
	CVector m_cvecTargetSpeedOverOneFrame;       // 0x130
	CVector m_cvecUpOverOneFrame;                // 0x13C
	CVector m_cvecTargetCoorsForFudgeInter;      // 0x148
	CVector m_cvecCamFixedModeVector;            // 0x154
	CVector m_cvecCamFixedModeSource;            // 0x160
	CVector m_cvecCamFixedModeUpOffSet;          // 0x16C
	CVector m_vecLastAboveWaterCamPosition;      // 0x178
	CVector m_vecBufferedPlayerBodyOffset;       // 0x184
	CVector Front;                               // 0x190
	CVector Source;                              // 0x19C
	CVector SourceBeforeLookBehind;              // 0x1A8
	CVector Up;                                  // 0x1B4
	CVector m_arrPreviousVectors[2];             // 0x1C0
	CVector m_avecTargetHistoryPos[4];           // 0x1D8
	uint32  m_anTargetHistoryTime[4];            // 0x208
	uint32  m_nCurrentHistoryPoints;             // 0x218
	CEntitySA* CamTargetEntity;                  // 0x21C
	float   m_fCameraDistance;                   // 0x220
	float   m_fIdealAlpha;                       // 0x224
	float   m_fPlayerVelocity;                   // 0x228
	CAutomobile* m_pLastCarEntered;              // 0x22C
	CPed*   m_pLastPedLookedAt;                  // 0x230
	bool    m_bFirstPersonRunAboutActive;        // 0x234

	void GetVectorsReadyForRW(void);
	void Process_FollowCar_SA_SA(const CVector&, float, float, float, bool);
};
static_assert(sizeof(CCamSA) == 0x238, "CCamSA: wrong size");

// San Andreas CCamera singleton layout; padding keeps each named field at its
// engine offset.
struct CCameraSA : public CPlaceableSA
{
	uint8   _pad18[0x1A - 0x18];         // 0x18
	bool    m_bCamDirectlyBehind;        // 0x1A
	bool    m_bCamDirectlyInFront;       // 0x1B
	uint8   _pad1C[0x21 - 0x1C];         // 0x1C
	bool    m_bIdleOn;                   // 0x21
	uint8   _pad22[0x28 - 0x22];         // 0x22
	bool    m_bJustCameOutOfGarage;      // 0x28
	uint8   _pad29[0x37 - 0x29];         // 0x29
	bool    m_bUseTransitionBeta;        // 0x37
	uint8   _pad38[0x40 - 0x38];         // 0x38
	bool    m_bVehicleSuspenHigh;        // 0x40
	uint8   _pad41[0x58 - 0x41];         // 0x41
	uint8   m_uiTransitionState;         // 0x58
	uint8   ActiveCam;                   // 0x59
	uint8   _pad5A[0x74 - 0x5A];         // 0x5A
	uint32  m_uiTimeWeEnteredIdle;       // 0x74
	uint8   _pad78[0xB4 - 0x78];         // 0x78
	int32   CarZoomIndicator;            // 0xB4 (m_nCarZoom)
	float   CarZoomValue;                // 0xB8 (m_fCarZoomBase)
	float   CarZoomValueTotal;           // 0xBC (m_fCarZoomTotal)
	float   CarZoomValueSmooth;          // 0xC0 (m_fCarZoomSmoothed)
	uint8   _padC4[0x174 - 0xC4];        // 0xC4
	CCamSA  Cams[3];                     // 0x174
};
extern CCameraSA *TheCameraSA;
