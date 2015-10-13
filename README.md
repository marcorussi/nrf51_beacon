# nrf51_beacon
An iBeacon example with nrf51 chip-set.
The firmware has been developed on top of softdevice s110 and Nordic SDK running on a nrf51 PCA10028 Dev. Kit.
You must have an arm-eabi-none GCC toolchain and JLink installed.


**Install**

Download Segger JLink tool from https://www.segger.com/jlink-software.html. Unpack it and move it to /opt directory.
Download the Nordic SDK from https://developer.nordicsemi.com/nRF51_SDK/nRF51_SDK_v9.x.x/. Unpack it and move it to /opt directory.
Clone my nrfjprog.sh repo in /opt directory by running:

    $ cd [your path]/opt
    $ git clone https://github.com/marcorussi/nrfjprog.git

Clone this repo in your projects directory:

    $ git clone https://github.com/marcorussi/nrf51_beacon.git
    $ cd nrf51_beacon
    $ gedit Makefile

Verify and modify following names and paths as required according to your ARM GCC toolchain:

```
PROJECT_NAME := nrf51_ble_beacon
NRFJPROG_PATH := /opt/nrfjprog
SDK_PATH := /opt/nRF51_SDK_9.0.0_2e23562
LINKER_SCRIPT := ble_app_beacon_gcc_nrf51.ld
GNU_INSTALL_ROOT := /home/marco/ARMToolchain/gcc-arm-none-eabi-4_9-2015q2
GNU_VERSION := 4.9.3
GNU_PREFIX := arm-none-eabi
```


**Flash**

Connect your nrf51 Dev. Kit, make and flash it:
 
    $ make
    $ make flash_softdevice (for the first time only)
    $ make flash


**Debug**

Verify that firmware is built for debugging:

    $ gedit Makefile

Uncomment the following line if commented:

```
DEBUG := 1
```

Build, flash and launch GDB:

    $ make
    $ make flash_softdevice (for the first time only)
    $ make flash
    $ make debug_init

Open a new terminal, go into the same working directory:

    $ cd [your path]/nrf51_beacon
    $ make debug_start

for the final optimised firmware release comment the line above in the Makefile (see step above).
ATTENTION: The pre-configured GDB commands launch and break program execution just after advertising packet preparation and its value is printed.
Modify *debug_init* command from the Makefle for customising your GDB commands file.


**Customise**

The iBeacon advertising format has two fields called MAJOR and MINOR that together with the UUID field allow the iBeacon device identification.
You can use default values for MAJOR and MINOR fields, statically defined in main.c file, or configurable values stored in the UICR region of the flash memory. This two values are 2 bytes long each. If you want to use the UICR ones then do following steps:

    $ gedit main.c

Uncomment the following define if commented:

```
#define USE_UICR_FOR_MAJ_MIN_VALUES
```

and build again:

    $ make

Before writing desired value to UICR region remember to erase all flash memory. In fact the UICR region should be erased before writing a new value. This value must be 4 bytes long at most where highest 2 bytes represent the MAJOR value and lowest 2 bytes represent the MINOR value.
Do following steps:

    $ make erase
    $ make memwr add=10001080 val=<your_4bytes_value_hex>
    $ make flash_softdevice
    $ make flash

You can verify the new values by debugging the firmware as shown above. The pre-configured GDB commands launch and break program execution just after advertising packet preparation and its value is printed.






