# TODO
- version stamp mbed-cli in docker file
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
### MBED CLI Version 1.10.5
# MBED System Clock Configuration

The system clock configuration for STM targets can be found in the `system_clock.c` file.
The actual path depends on the seleted target, but it should look something like this:

```
mbed-os/targets/TARGET_STM/TARGET_STM32F4/TARGET_STM32F446xE/system_clock.c
```

In the case of having an external 8mhz crystal connected to to the MCU, the config goes as so:

```
// Enable HSE oscillator and activate PLL with HSE as source
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
RCC_OscInitStruct.HSEState = RCC_HSE_ON;             // External 8 MHz xtal on OSC_IN/OSC_OUT
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
RCC_OscInitStruct.PLL.PLLM = 8;                      // VCO input clock = 1 MHz (8 MHz / 8)
RCC_OscInitStruct.PLL.PLLN = 360;                    // VCO output clock = 360 MHz (1 MHz * 360)
RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;          // PLLCLK = 180 MHz (360 MHz / 2)
RCC_OscInitStruct.PLL.PLLQ = 7;                      //
RCC_OscInitStruct.PLL.PLLR = 2;                      //
```

What this means, is essentiallt the main system clock is running at the max processor speed of `180 MHz` 🤷‍♂️

## Building without Docker

### Pull submodules
```
git submodule init
git submodule update
```
### Build the project .elf file
```
mbed compile
```

### Making changed to git submodules
First, `cd` into the submodule directory and checkout a new branch with `git checkout -b myBranchName`

You can now commit changes and push to the remote

## Building with docker

Build targets in Makefile and justfile build required docker image and then use that image to build the mbed project.

### Makefile

Build debug build:

    make

### justfile

Pull library specified in `mbed-os.lib`:

    just init

Build debug build:

    just build

Build release build:

    just build release

Clear build directory:

    just clean

Build/rebuild docker image:

    just docker

Run arbitrary command from docker environment:

    just do <cmd>
    just do mbed --version

Enter shell in docker environment:

    just run
