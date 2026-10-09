III.VC.SA.ModernCarCam
======================

A modern vehicle camera for GTA III, GTA Vice City and GTA San Andreas.
The default settings reproduce each game's own vehicle camera; optional extras
(steering wobble, elastic string physics, terrain tilt, dynamic speed FOV, VCS
camera shake, reverse look-behind, free look, modern drive-by aiming) are layered
on top and are all configurable in III.VC.SA.ModernCarCam.ini. Terrain tilt has a
minimum-slope dead-zone (and a predictable in-air behaviour), the passing-traffic
nudge is one short speed-scaled impulse, and the reverse camera waits a
configurable delay before swinging. The IV profile uses dynamic FOV and, on
GTA III, a softer camera heading follow (the `HeadingFollow` option, handy for
custom profiles too).

Install
-------
1. Install an ASI loader, for example Silent's ASI Loader
   (https://github.com/ThirteenAG/Ultimate-ASI-Loader).
2. Copy III.VC.SA.ModernCarCam.asi and III.VC.SA.ModernCarCam.ini into your
   GTA III, GTA Vice City or GTA San Andreas game folder. The .ini is also
   found in a "scripts" subfolder.
3. Start the game. Open III.VC.SA.ModernCarCam.ini and set
   Profile = Game (default), III, VC, SA, LCS, VCS, IV, Enhanced or Custom.

Sources
-------
- Source code and releases:
  https://github.com/SandeMC/SACarCamWithModernDriveByWithTilt
- Also published on LibertyCity:
  https://libertycity.net/files/244225-moderncarcam-iii-vc-sa.html

Debug menu
----------
When the game's debug menu is available (a debug build, or a debug-menu
enabler), the mod adds a "ModernCarCam" section with live toggles for the modern
drive-by, its two shot-direction locks, the smooth side view, mouse free-look,
free turret control, the water camera and the camera-bug fix. Toggling an entry
applies immediately; the ini is still the source of truth on the next launch.

Credits
-------
Based on the reversed sources of re3 (GTA III) and reVC (Vice City). The VCS
camera shake and speed-dependent FOV are ported from ThirteenAG's
WidescreenFixesPack. The modern drive-by aiming was worked out with help from
ModernizedDriveBy (https://libertycity.net/files/gta-san-andreas/215460-modernized-driveby.html)
by void. See the LICENSE and licenses/ folder for details.
