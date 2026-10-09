III.VC.SA.ModernCarCam
======================

A modern vehicle camera for GTA III, GTA Vice City and GTA San Andreas.

======================

Installation
-------

1. Install an ASI loader (for example Silent's ASI Loader - https://github.com/GTAmodding/ASI-Loader/releases/latest or Ultimate ASI Loader - https://github.com/ThirteenAG/Ultimate-ASI-Loader.
2. Copy III.VC.SA.ModernCarCam.asi and III.VC.SA.ModernCarCam.ini into the scripts folder or game folder.
3. Edit III.VC.SA.ModernCarCam.ini and set Profile to the camera you want. Everything else is optional.

For Widescreen Fix users: this mod takes over the Widescreen Fix's own Speed Sensitive FOV and VCS Camera Shake options: both are forced off in WSF while this mod is installed, so this mod's DynamicSpeedFOV and VCSCamShake settings are authoritative (the code is the same, this is done to avoid confusion)

For Drive-by mods users:
- Modernized Driveby features have been reimplemented in this mod; I wouldn't recommend using both mods at once
- Manual Driveby VC - https://libertycity.net/files/gta-vice-city/213323-manual-driveby-vc.html),
  Manual Driveby III - https://libertycity.net/files/gta-3/213322-manual-driveby-iii.html
  and Manual Driveby Refixed (SA) - https://libertycity.net/files/gta-san-andreas/213857-manual-driveby-refixed.html
  are the only "Manual" style mods tested against this mod and verified to be compatible

Sources
-------
- Source code and releases:
  https://github.com/SandeMC/SACarCamWithModernDriveByWithTilt
- Also published on LibertyCity:
  https://libertycity.net/files/244225-moderncarcam-iii-vc-sa.html

Debug menu
----------
When the game's debug menu (https://libertycity.net/files/gta-san-andreas/161611-debugmenu.html) is available, the mod adds a ModernCarCam section to it on GTA III, Vice City and San Andreas. It exposes everything the ini can set as live controls - the profile, every feature toggle, the effect strengths, the slope/FOV/shake/traffic tuning, the anchors and stiffness, the offsets and the Custom camera shape - grouped into submenus. Angles and speed thresholds are edited in the same units as the ini (degrees and km/h).

Credits
-------

- Original SACarCam by erorcun and the SACarCamWithModernDriveBy fork by ICantReadYourMind.
- Authentic camera algorithms based on the reversed GTA III / Vice City sources of re3 / reVC, using the Hezkore/hez-gta-re3 fork. re3 is not released under a standard license; its terms (educational / documentation / modding use, non-commercial, keep derivative work open source, give proper credit) are reproduced in `licenses/re3.txt`.
- VCS camera shake and dynamic speed FOV ported from ThirteenAG's WidescreenFixesPack, MIT licensed. See `licenses/WidescreenFixesPack.txt`.
- San Andreas addresses and layouts grounded in gta-reversed and plugin-sdk.
- The modern drive-by aiming (sideways aiming based on the camera angle) was worked out with help from ModernizedDriveBy (https://libertycity.net/files/gta-san-andreas/215460-modernized-driveby.html) by void.
- This project is MIT licensed; see `LICENSE`. The third-party notices in `licenses/` apply to the portions they describe.
- This project has been developed primarily by AI agents, but it has been thoroughly tested throughout the way.