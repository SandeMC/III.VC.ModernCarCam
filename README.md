# III.VC.SA.ModernCarCam

A universal vehicle camera ASI for GTA III, GTA Vice City and GTA San Andreas.
With the shipped settings the camera reproduces the original game; every
additional behaviour is an ini option or a profile layered on top.

The authentic camera is based on the reversed sources of re3 (GTA III) and reVC
(Vice City). GTA III and Vice City implement the vehicle camera differently, so
the mod keeps both algorithms and picks the right one for the running game.
San Andreas already ships the follow camera this mod is built around, so on SA
the mod hooks `CCam::Process_FollowCar_SA` directly and keeps the SA camera's own
distance, FOV, angles, anchor and stiffness.

## Installation

1. Install an ASI loader (for example
   [Silent's ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)).
2. Copy `III.VC.SA.ModernCarCam.asi` and `III.VC.SA.ModernCarCam.ini` into the
   game folder (GTA III, GTA Vice City or GTA San Andreas). The mod also finds
   the ini in a `scripts` subfolder.
3. Edit `III.VC.SA.ModernCarCam.ini` and set `Profile` to the camera you want.
   Everything else is optional.

## Downloads & sources

- Source and releases: [SandeMC/SACarCamWithModernDriveByWithTilt](https://github.com/SandeMC/SACarCamWithModernDriveByWithTilt)
- Also published on LibertyCity: [ModernCarCam (III, VC, SA)](https://libertycity.net/files/244225-moderncarcam-iii-vc-sa.html)

## Profiles

The camera is selected with a single `Profile` setting:

| Profile      | Description                                                        |
|--------------|--------------------------------------------------------------------|
| Game         | Whatever game is running (default)                                 |
| III          | The GTA III camera                                                 |
| VC           | The Vice City camera                                               |
| SA           | The San Andreas follow camera                                      |
| Enhanced     | Game plus quality-of-life additions (Vice City based)              |
| LCS          | Liberty City Stories camera                                        |
| VCS          | Vice City Stories camera                                           |
| IV           | GTA IV camera style                                                |

IV is an approximation of that game's feel applied on top of the modern camera; its exact distances and angles can be dialled in with the Custom profile.

Game, III, VC and SA profiles add only the free camera and fixes on top of the original camera. Everything else (wobble, elastic string, dynamic FOV, shake, etc.) is off in those profiles and available as an override in `[Features]` or through the Enhanced / modern profiles.

Enhanced is based on the Vice City camera: it uses Vice City's feature set (steering roll, per-vehicle zoom, bike-with-passenger height), Vice City's per-zoom camera angles, and Vice City's camera anchor and stiffness, even when running GTA III. It also enables the elastic string, dynamic speed FOV, full terrain pitch tilt, VCS shake and the reverse look-behind camera. When running GTA San Andreas, Enhanced enables those same features but the SA camera keeps its own distance, FOV, angles, anchor and stiffness, and leaves the Vice City steering wobble and VCS shake off by default (they can still be turned on in `[Features]`). The III-only turret hook is not installed on SA.

Enhanced also turns on the smooth side view and sets the drive-by defaults it ships with: `LockShootDirectionKBM = 0` and `LockShootDirectionJOY = 1`.

## Features

- One profile for the whole camera, with optional per-feature overrides
- Faithful GTA III and Vice City vehicle cameras recreations, can add any feature on-top of them
- Free mouse look from San Andreas as an option, with a smoothed gamepad right stick (works in III, VC and SA)
- Free turret control from San Andreas as an option (Rhino / Firetruck)
- Drive-by aiming: classic (aim left/right/behind only while the look key is held) or modern (aim where the camera looks), with an optional per-burst direction lock, separate for keyboard/mouse and gamepad
- Smooth side view: the look left/right/behind transition swings into place instead of snapping
- Dynamic speed FOV (ported from ThirteenAG's Widescreen Fix)
- Vice City Stories camera shake (ported from ThirteenAG's Widescreen Fix)
- Steering wobble from Vice City as an option
- Terrain pitch tilt from Vice City as an option, with a new option to also apply the tilt when going upwards
- Per-vehicle zoom from Vice City as an option
- Bike passenger height option
- Elastic string stretch from San Andreas as an option
- Camera wobbling when passing by traffic option
- Several fixes
- Adjustable stiffness, anchoring and height
- Enhanced reverse driving camera
- Every feature strength is a multiplier: `1` is the profile default, `0` turns
  it off, and any other value scales it (for example `CameraWobble = 2` doubles
  the steering lean, `0.5` halves it). `PitchTilt` takes optional
  `PitchTiltUphill` / `PitchTiltDownhill` overrides, and `DynamicSpeedFOV` /
  `VCSCamShake` take their own start-speed (and max-FOV) options.
- The `[Offsets]` category moves the camera offsets out of the profile so they
  apply to every profile: `CameraHeightOffset`, `CameraLateralOffset`,
  `CameraDistanceOffset`, `CameraMinDistance`, `CameraDistanceScale` and
  `CameraDriverOffsetX/Y/Z`
- Takes over the Widescreen Fix's own Speed Sensitive FOV and VCS Camera Shake options: both are forced off in WSF while this mod is installed, so this mod's `DynamicSpeedFOV` and `VCSCamShake` settings are authoritative

## Debug menu

When the game's debug menu is available (a debug build, or a debug-menu enabler),
the mod adds a **ModernCarCam** section to it with live toggles for the modern
drive-by, its two shot-direction locks, the smooth side view, mouse free-look,
free turret control, the water camera and the camera-bug fix. Toggling an entry
applies immediately; the ini is still the source of truth on the next launch.

## Building

Open `ModernCarCam.sln` and build `Release` (Win32). The output is
`Release\III.VC.SA.ModernCarCam.asi`, next to the shipped
`III.VC.SA.ModernCarCam.ini`.

## Credits & licenses

- Original SACarCam by erorcun and the SACarCamWithModernDriveBy fork by ICantReadYourMind.
- Authentic camera algorithms based on the reversed GTA III / Vice City sources of re3 / reVC, using the [Hezkore/hez-gta-re3](https://github.com/Hezkore/hez-gta-re3) fork. re3 is not released under a standard license; its terms (educational / documentation / modding use, non-commercial, keep derivative work open source, give proper credit) are reproduced in `licenses/re3.txt`.
- VCS camera shake and dynamic speed FOV ported from [ThirteenAG's WidescreenFixesPack](https://github.com/ThirteenAG/WidescreenFixesPack), MIT licensed. See `licenses/WidescreenFixesPack.txt`.
- San Andreas addresses and layouts grounded in [gta-reversed](https://github.com/gta-reversed/gta-reversed) and [DK22Pac/plugin-sdk](https://github.com/DK22Pac/plugin-sdk).
- The modern drive-by aiming (sideways aiming based on the camera angle) was worked out with help from [ModernizedDriveBy](https://libertycity.net/files/gta-san-andreas/215460-modernized-driveby.html) by void.
- This project is MIT licensed; see `LICENSE`. The third-party notices in `licenses/` apply to the portions they describe.
- This project has been developed primarily by AI agents, but it has been thoroughly tested throughout the way.
