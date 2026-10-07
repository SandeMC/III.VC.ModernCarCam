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
| VCS          | Vice City Stories camera                                           |
| IV           | GTA IV camera style                                                |

IV is an approximation of that game's feel applied on top of the modern camera; its exact distances and angles can be dialled in with the Custom profile.

Game, III, VC and SA profiles add only the free camera, free turret control and fixes on top of the original camera. Everything else (wobble, elastic string, dynamic FOV, shake, etc.) is off in those profiles and available as an override in `[Features]` or through the Enhanced / modern profiles.

Enhanced is based on the Vice City camera: it uses Vice City's feature set (steering roll, per-vehicle zoom, bike-with-passenger height), Vice City's per-zoom camera angles, and Vice City's camera anchor and stiffness, even when running GTA III. It also enables the elastic string, dynamic speed FOV, full terrain pitch tilt, VCS shake and the reverse look-behind camera. The III-only engine behaviour (roof/ground camera height, top-down camera, reversed turret) is kept.

## Features

- One profile for the whole camera, with optional per-feature overrides
- Faithful GTA III and Vice City vehicle cameras recreations, can add any feature on-top of them
- Free mouse look from San Andreas as an option
- Free turret control from San Andreas as an option (Rhino / Firetruck)
- 360-degree drive-by aiming
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
- The Custom profile exposes camera distance and height offsets (`CustomDistanceOffset`, `CustomCameraHeight`) to dial in those tables

## Building

Open `ModernCarCam.sln` and build `Release` (Win32). The output is
`ModernCarCam.asi`.

## Credits & licenses

- Original SACarCam by erorcun and the SACarCamWithModernDriveBy fork by ICantReadYourMind.
- Authentic camera algorithms based on the reversed GTA III / Vice City sources of re3 / reVC, using the [Hezkore/hez-gta-re3](https://github.com/Hezkore/hez-gta-re3) fork. re3 is not released under a standard license; its terms (educational / documentation / modding use, non-commercial, keep derivative work open source, give proper credit) are reproduced in `licenses/re3.txt`.
- VCS camera shake and dynamic speed FOV ported from [ThirteenAG's WidescreenFixesPack](https://github.com/ThirteenAG/WidescreenFixesPack), MIT licensed. See `licenses/WidescreenFixesPack.txt`.
- This project is MIT licensed; see `LICENSE`. The third-party notices in `licenses/` apply to the portions they describe.
- This project has been developed primarily by AI agents, but it has been thoroughly tested throughout the way.
