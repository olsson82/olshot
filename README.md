# OlShot
This is an Arduino based Shooting game system. Free to use, you need to make the components your self.

It's wireless and uses nrf24l01 modules to communicate to the targets and the main system.

# Read more
* [Wiki](https://github.com/olsson82/olshot/wiki)
* [Targets](page/targets.md)
* [Main Unit](page/mainunit.md)

## Still in development
The system is still in development and this page will be updated during the progress.

## Updates
### Target PCB
**2023-02-05: The Target PCB Is now avalible for kicad and arduino code.**

Did a redesign of the target sensor, everything is now on the top side. Added jst connections instead of soldering.
<figure>
    <img src="image/target/targetsensor-front.png"
         alt="Front side">
    <figcaption>Front side of the PCB.</figcaption>
</figure>

### Main Unit
The main unit is still in development. The sd card part has some problems with the level shifter that needs to fix and also i will remove the lipo charger and make it possible to power from usb or 12v. This is due to the nextion require 5v and the lipo batterys can not give the power it needs.


## Discuss
Feel free to take part in this project. Go to [Discussion](https://github.com/olsson82/olshot/discussions) to ask questions or discuss about the project.
