# III.VC.ModernCarCam

A universal vehicle camera ASI for GTA III and GTA Vice City. With the shipped
settings the camera reproduces the original game; every additional behaviour is
an ini option or a profile layered on top.

The authentic camera is based on the reversed sources of re3 (GTA III) and reVC
(Vice City). GTA III and Vice City implement the vehicle camera differently, so
the mod keeps both algorithms and picks the right one for the running game.

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
| VCS          | Vice City Stories camera (includes the high-speed shake)           |
| IV           | GTA IV camera style (driver's seat centred)                        |

IV is an approximation of that game's feel (smooth, momentum based, no
steering wobble) applied on top of the modern camera; its exact distances and
angles can be dialled in with the Custom profile.

Game, III, VC and SA add only the free camera, free turret control and
fixes on top of the original camera. Everything else (wobble, elastic string,
dynamic FOV, shake, etc.) is off in those profiles and available as an override
in `[Features]` or through the Enhanced / modern profiles.

Enhanced is based on the Vice City camera: it uses Vice City's feature set
(steering roll, per-vehicle zoom, bike-with-passenger height), Vice City's
per-zoom camera angles, and Vice City's camera anchor and stiffness, even when
running GTA III. It also enables the elastic string, dynamic speed FOV, full
terrain pitch tilt, VCS shake and the reverse look-behind camera. The III-only
engine behaviour (roof/ground camera height, top-down camera, reversed turret)
is kept.

## Features

- One profile for the whole camera, with optional per-feature overrides
- Faithful GTA III and Vice City vehicle cameras, including the behind-boat
  camera and the GTA III top-down camera (GTA III only: Vice City's own view
  cycle deliberately skips its top-down mode)
- Free mouse look, ported from the San Andreas camera
- Free turret control (Rhino / Firetruck) and 360-degree drive-by aiming
- Dynamic speed FOV, a faithful port of the Widescreen Fix's
  `CarSpeedDependantFOV`
- Vice City Stories camera shake (ported from ThirteenAG's WidescreenFixesPack)
- Steering wobble, terrain pitch tilt, per-vehicle zoom, bike passenger height,
  elastic string stretch, camera clipping fixes and more
- The Custom profile exposes camera distance and height offsets
  (`CustomDistanceOffset`, `CustomCameraHeight`) to dial in those tables

## Widescreen Fix compatibility

The vanilla profiles leave the game's per-vehicle zoom table untouched so the
Widescreen Fix keeps control of it; only the non-vanilla distance profiles
patch it.

## Building

Open `ModernCarCam.sln` and build `Release` (Win32). The output is
`ModernCarCam.asi`.

## Credits & licenses

- Original SACarCam by erorcun and the SACarCamWithModernDriveBy fork.
- Authentic camera algorithms based on re3 / reVC.
- VCS camera shake and dynamic speed FOV ported from
  [ThirteenAG's WidescreenFixesPack](https://github.com/ThirteenAG/WidescreenFixesPack),
  MIT licensed. See `licenses/WidescreenFixesPack.txt`.
- This project is MIT licensed; see `LICENSE`.
