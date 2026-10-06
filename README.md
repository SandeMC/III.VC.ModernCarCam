# III.VC.ModernCarCam

Fork of [SACarCamWithModernDriveBy](https://github.com/ICantReadYourMind/SACarCamWithModernDriveBy) which is a fork of [SACarCam](https://github.com/erorcun/SACarCam), which implements SA and LCS styled camera into GTA III and VC.

This fork takes the idea further and implements the following:
- Universal ASI for both GTA III and GTA Vice City with runtime game detection
- Configuration file (`ModernCarCam.ini`), where you can toggle every individual feature and customize values
- A faithful reproduction of the original GTA III and Vice City vehicle camera: with the shipped vanilla settings the camera behaves and looks exactly like the original game
- Support for presets (SA, LCS, Vanilla) and fully customizable numeric values via Custom profiles

The authentic camera is based on the reversed sources of re3 (GTA III) and reVC
(Vice City). GTA III and Vice City implement the vehicle camera differently, so
the mod keeps both algorithms separate and layers its ini options on top.

## Vanilla accuracy

With the default settings (`CameraProfile = Vanilla` and the "match game"
feature defaults) the camera reproduces the original:

- Correct base distance, target height and string physics per game
- Correct per-zoom elevation angle (derived from the zoom value in GTA III,
  per-vehicle-type tables in Vice City)
- Correct terrain pitch behaviour (level in GTA III, downhill tilt in Vice City)
- GTA III roof/ground camera-height correction (WorkOutCamHeight)
- Vice City geometry avoidance, helicopter height clamp and Firetruck cannon
  camera tracking
- The dedicated "behind boat" vehicle camera of both games
- Correct steering roll/wobble (Vice City only), bike passenger camera height
  (Vice City only) and vehicle-specific zoom (Vice City only)
- Dynamic speed FOV, which Vice City has and GTA III does not, is available and
  functional in both games as an option

## Full list of features

- Distance, FOV, and Angles configurable independently with presets for SA, LCS, Vanilla, and Custom
- Custom profile parameters: manually tune distance presets (near/mid/far), distance offset, minimum distance, base FOV, dynamic FOV expansion, and elevation angles in `[CustomProfile]`
- Camera anchoring & stiffness option: matches the authentic vanilla III/VC rigid tether behind the car body orientation rather than the San Andreas loose momentum tracking
- VCS style camera shake option: high-speed camera shake effect like in GTA Vice City Stories (ported from ThirteenAG's WidescreenFixesPack, MIT licensed)
- Camera wobble / roll tilt option (Vice City vanilla feature; steering roll and inertia)
- Elastic string physics option (adds a San Andreas style speed stretch on top of the always-present vanilla string)
- Pitch tilt option (Vice City vanilla feature; smoothly tilts the camera when driving on hills/slopes)
- Dynamic speed FOV option (expands the field of view as vehicle speed increases; works in GTA III too)
- Vehicle-specific zoom scaling option (different camera distances per vehicle type: cars, bikes, boats, helis, etc.)
- Modern turret control option (ported from SA; Rhino tank turret and Firetruck water cannon aim where the camera looks)
- Modern drive-by option (drive-by shooting automatically aims in the direction the camera is looking)
- Mouse free-look option (ported from SA; orbit camera freely around the vehicle with the mouse)
- Fix camera clip option (prevents camera from clipping into vehicle models when stationary)
- Keep camera over water option (ported from SA; prevents camera from dipping underwater)
- Bikes height increase option (Vice City vanilla feature; camera height rises smoothly when carrying a passenger on a bike)

## Widescreen Fix compatibility

The vanilla profile leaves the game's per-vehicle camera zoom table untouched,
so the Widescreen Fix keeps control of it and the two mods do not fight over the
same values. Only non-vanilla distance profiles patch that table.

## Credits & licenses

- Original SACarCam by erorcun and the SACarCamWithModernDriveBy fork.
- Authentic camera algorithms based on re3 / reVC.
- VCS camera shake ported from [ThirteenAG's WidescreenFixesPack](https://github.com/ThirteenAG/WidescreenFixesPack), MIT licensed. See `licenses/WidescreenFixesPack.txt`.
