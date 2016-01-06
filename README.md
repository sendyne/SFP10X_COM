# SFP10X_COM
C library for FTDI-based serial communication with Sendyne's SFP products.  
Copyright &copy; 2015-1026, Sendyne Corp., New York, USA

[Sendyne Corp.](http://sendyne.com)  
[Email](mailto:infoi@sendyne.com)  
[Sendyne SFP products family](http://www.sendyne.com/Products/Sendyne%20Sensing%20Family.html)


## Requirements

### Supported platforms
The SFP10X_COM library is designed to be used on the following platforms:

* Microsoft Windows (XP or above),
* Apple OS X (versions 10.9.X or above),
* Linux Ubuntu and derivatives.

### FTDI D2xx driver
The SFP10X_COM library requires a functional installation of the [FTDI D2XX](http://www.ftdichip.com/Drivers/D2XX.htm) driver library.

Refer to [D2XX webpage](http://www.ftdichip.com/Drivers/D2XX.htm) for installation instructions and documentation.


## Compilation
SFP10X_COM is pure C and should compile with most available compilers (Visual Studio, GCC, Clang, ...).

The SFP10X_COM library can be used in two ways:

* by adding the header and source file to the user project and including them in the build process,
* by building a library and linking the user project against it.

In both cases, the user needs to provide the location of the FTDI D2XX header files, and to link against the FTDX D2XX library (either statically or dynamically).

### Notes for Microsoft Windows platforms
A Windows module-definition file (SFP10X\_COM.def) is provided with the SFP10X_COM library in order to properly build a DLL version of the library.

### Notes for OS X platforms
Compilation on OS X platforms requires additional linking:

* the user needs to link against the CoreFoundation and IOKit frameworks (`-framwork CoreFoundation -framework IOKit`),
* the user needs to link against the libobjc library (`-lobjc`).

In addition, as explained in the [FTDI Application Note AN_134](http://www.ftdichip.com/Support/Documents/AppNotes/AN_134_FTDI_Drivers_Installation_Guide_for_MAC_OSX.pdf) regarding the possible conflicts with Apple own FTDI driver, FTDI driver and the FTDI D2XX library. The user might need to manually unload kernel extensions for the SFP10X_COM library to work properly.


## Example
The file [main.c](main.c) illustrates the functionalities provided by the SFP10X_COM library.


## License
MIT License (see [LICENSE](LICENSE)).


## Contributing
Bug reports and pull requests welcome!
