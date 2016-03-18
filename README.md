# SFP10X_COM
C library for FTDI-based serial communication with Sendyne's Sensing Family products.  
Copyright &copy; 2010-2016, Sendyne Corp., New York, USA

[Sendyne Corp.](http://sendyne.com)  
[Email](mailto:info@sendyne.com)  
[Sendyne SFP products family][sfp_page]


## Description
Communication library for the Sendyne Signature Series, SFP101 and the SFP102 sensing
products.

This library simplifies the implementation of the communication protocol with
the SFP family. All values coming from the library are in HEX format and need
processing to be human readable. A helper function is provided to take care of
the conversion of HEX data.

For more information, refer to the module specific datasheet (can be found at
[Sendyne SFP products family][sfp_page]).

Take a look at our [videos on using the SFP10X_COM library in NI LabView](https://www.youtube.com/playlist?list=PL2tMAX1cpLJu92bLrU1GD6hDbyyS_E2VL).


## Requirements

### Supported platforms
The SFP10X_COM library is designed to be used on the following platforms:

* Microsoft Windows (XP or above),
* Apple OS X (versions 10.9.X or above),
* Linux Ubuntu and derivatives.

### FTDI D2XX driver
The SFP10X_COM library requires a functional installation of the
[FTDI D2XX][ftdi_d2xx_link] driver library.

Refer to [D2XX webpage][ftdi_d2xx_link] for installation instructions and
documentation.

Note that, on OS X and Linux platforms the FTDI D2XX library has special
requirements to operate properly; see the following documentation:

* [OS X D2XX installation guide][ftdi_mac_an],
* [Linux D2XX installation guide][ftdi_linux_an].

We recommend that, prior to using the SFP10X_COM library, the user check its
FTDI D2XX installation by compiling and running the examples shipped with the
D2XX library.


## Compilation
SFP10X_COM is pure C and should compile with most available compilers (Visual
Studio, GCC, Clang, ...).

The SFP10X_COM library can be used in two ways:

* by adding the header and source file to the user project and including them
  in the build process,
* by building a library and linking the user project against it.

In both cases, the user needs to provide the location of the FTDI D2XX header
files, and to link against the FTDX D2XX library (either statically or
dynamically).

### Notes for Microsoft Windows platforms
A Windows module-definition file (SFP10X\_COM.def) is provided with the
SFP10X_COM library in order to properly build a DLL version of the library.

A DLL and static version of SFP10X_COM library are provided in
[compiled_win](compiled_win/).

### Notes for OS X platforms
Compilation on OS X platforms requires additional linking:

* the user needs to link against the CoreFoundation and IOKit frameworks
  (`-framework CoreFoundation -framework IOKit`),
* the user needs to link against the libobjc library (`-lobjc`).

In addition, as explained in the [FTDI Application Note AN_134][ftdi_mac_an]
regarding the possible conflicts with Apple's own FTDI driver, FTDI driver and
the FTDI D2XX library, the user might need to manually unload kernel
extensions for the SFP10X_COM library to work properly (see commands
`kextstat` and `kextunload`).

### Notes for Linux platforms
Compilation on Linux platforms requires additional linking:

* the user needs to link against pthread and dl libraries (`-lpthread -ldl`).

In addition, similar to OS X platforms, some kernel modules need to unloaded
to allow the FTDI D2XX library to bind to the device. Those kernel modules are
`ftdi_sio` and `usbserial`. Refer to the
[FTDI D2XXdocumentation][ftdi_linux_an] for more details. Finally, executables
should be run with elevated privileges (using the `sudo` command).

### C# wrapper ###
A [C# wrapper](CSharp_wrapper/) for the SFP10X_COM library is also provided to
facilitate integration with Visual C# and .NET projects.


## Example
The file [main.c](main.c) illustrates the functionalities provided by the
SFP10X_COM library.


## License
MIT License (see [LICENSE](LICENSE)).


## Contributing
Bug reports and pull requests welcome!

[sfp_page]: http://www.sendyne.com/Products/Sendyne%20Sensing%20Family.html
[ftdi_d2xx_link]: http://www.ftdichip.com/Drivers/D2XX.htm
[ftdi_mac_an]: http://www.ftdichip.com/Support/Documents/AppNotes/AN_134_FTDI_Drivers_Installation_Guide_for_MAC_OSX.pdf
[ftdi_linux_an]: http://www.ftdichip.com/Support/Documents/AppNotes/AN_220_FTDI_Drivers_Installation_Guide_for_Linux%20.pdf

