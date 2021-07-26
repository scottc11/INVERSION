
### MBED Version 5.14.1
- tag = mbed-os-5.14.1

Cloning project

```
git clone https://github.com/scottc11/inversion.git
git submodule init
git submodule update
```

pull latest mbed-lib changes
```
git submodule update --remote
```

---
# TODO

- quantizer mode has option to limit the amount of keys to be active
- FLASH for saving most recent state on power off
- Sequencer Gate output instead of trigger output
- Note override in quantizer / loop mode (when key touched, only output that voltage). This should only be possible when holding the FREEZE button down.
- Root Note adjustments / offset
- Copy/Paste Recorded Sequence to other channels
- Auto Calibration UI
- MIDI implementation
- Record octave changes
- dim octave LED being output in qunatizer mode
- Start Every channel in the middle octave
- Clock multiplier / divider per channel
- Should REC being held down mean loop can be over-dubbed?
- FREEZE should contrinue loop sequence, but only at a pre-set number of steps - creating loop "windows". In other words, freeze actually just cuts the current loop by certain divisions ie. 8 step loop cus down to 4, then 2, then 1 etc.

---
# [mbed_app.json](https://os.mbed.com/docs/mbed-os/v5.15/reference/configuration.html)

This file is used to override the default MBED build configurations


#### [Possible implementation of HAL framework in MBED](https://os.mbed.com/forum/platform-34-ST-Nucleo-F401RE-community/topic/4963/?page=2)

#### [another](https://os.mbed.com/users/gregeric/code/Nucleo_Hello_Encoder/docs/tip/main_8cpp_source.html)