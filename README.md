# ht.min
![CI](https://github.com/Cycling74/min-devkit/actions/workflows/test.yml/badge.svg)
Max externals created using the Min-DevKit for Max, an API and supporting tools for writing externals in modern C++.

## Acknowledgements
This package was created using [the Min-DevKit for Max](https://github.com/Cycling74/min-devkit).

## How to Use
- Download this repository into your `Max 8/Packages` folder (by default it is located in `~/Documents`).

## List of Externals
- ht.alarm
    - Outputs a bang at the specified time, runs on the scheduler thread.
- ht.alarm~
    - Outputs a bang at the specified time, runs on the audio thread.
- ht.between
    - Outputs a bang from the different outlets depending on wheather the input number is between two values.
- ht.b2bi
    - Converts float into a uint8_t list. 
- ht.COBS
    - Encode a list with Consistent Overhead Byte Stuffing (COBS) algorithm. Compatible with [PacketSerial](https://github.com/bakercp/PacketSerial) library for Arduino. 
- ht.instance
    - Counts the objects with same name


## Notes
- All .mxo and .mxe64 objecs are compiled on macOS and Windows10 (64bit).

## Licence
MIT


## Prerequisites

You can use the objects provided in this package as-is.

To code your own objects, or to re-compile existing objects, you will need a compiler:

* On the Mac this means **Xcode 9 or later** (you can get from the App Store for free). 
* On Windows this means **Visual Studio 2017** (you can download a free version from Microsoft). The installer for Visual Studio 2017 offers an option to install Git, which you should choose to do.

You will also need the Min-DevKit, available from the Package Manager inside of Max or [directly from Github](https://github.com/Cycling74/min-devkit).




## Contributors / Acknowledgements

The ht.min is the work of some amazing and creative artists, researchers, and coders.



## Support

For support, please contact the developer of this package.
