# III.VC.ModernCarCam

Fork of [SACarCamWithModernDriveBy](https://github.com/ICantReadYourMind/SACarCamWithModernDriveBy) which is a fork of [SACarCam](https://github.com/erorcun/SACarCam), which implements SA and LCS styled camera into GTA III and VC.

This fork takes the idea further and implements the following:
- Universal ASI for both GTA III and GTA Vice City with runtime game detection
- Configuration file (`ModernCarCam.ini`), where you can toggle every individual feature and customize values
- Restored authentic vanilla GTA III and Vice City camera features and anchoring feel
- Support for presets (SA, LCS, Vanilla) and fully customizable numeric values via Custom profiles

Full list of features:
- Distance, FOV, and Angles configurable independently with presets for SA, LCS, Vanilla, and Custom
- Custom profile parameters: manually tune distance presets (near/mid/far), distance offset, minimum distance, base FOV, dynamic FOV expansion, and elevation angles in `[CustomProfile]`
- Camera anchoring & stiffness option: replicates the authentic vanilla III/VC rigid tether behind the car body orientation rather than loosely floating or lagging behind the momentum vector
- VCS style camera shake option: introduces high-speed camera shake effect like in GTA Vice City Stories (borrowed from ThirteenAG's WidescreenFix; credit to ThirteenAG)
- Camera wobble / roll tilt option (VC vanilla feature; steering roll and inertia)
- Elastic string physics option (III and VC vanilla feature; distance stretches during acceleration, catches up on braking, spring lag)
- Pitch tilt option (VC vanilla feature; smoothly tilts camera up/down when driving on hills/slopes)
- Dynamic speed FOV option (expands field of view as vehicle speed increases)
- Vehicle-specific zoom scaling option (different camera distances per vehicle type: cars, bikes, boats, helis, etc.)
- Modern turret control option (ported from SA; Rhino tank turret and Firetruck water cannon aim where the camera looks)
- Modern drive-by option (drive-by shooting automatically aims in the direction the camera is looking)
- Mouse free-look option (ported from SA; orbit camera freely around the vehicle with the mouse)
- Fix camera clip option (prevents camera from clipping into vehicle models when stationary)
- Keep camera over water option (ported from SA; prevents camera from dipping underwater)
- Bikes height increase option (VC vanilla feature; camera height rises smoothly when carrying a passenger on a bike)
