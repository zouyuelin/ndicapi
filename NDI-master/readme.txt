Table of Contents
=================
1.0 About CAPIsample
2.0 Using CAPIsample
3.0 Building CAPIsample
 3.1 Required Tools
 3.2 Build Commands
4.0 ARDemo

1.0 About CAPIsample
=================
The Combined API (CAPI) sample application demonstrates how to use the Combined API for Polaris and Aurora
systems in a C++ application to communicate with an NDI measurement system. The source code is intended as a
reference to help developers become familiar with NDI's API.

Please see license.txt and the copyright notices in source files for the terms associated with the
provision of this sample code.

2.0 Using CAPIsample
====================
The usage of the CAPIsample console application is:

   1. Connect an NDI measurement system to the host machine and ensure the device is visible on the OS.
      a) For serial devices, ensure the USB drivers are installed and the operating system recognizes the serial port.
         On Windows this will be a COM port listed in the device manager.
         On Linux this will be a /dev/tty* device (usually /dev/ttyUSBx where x is a number).
         On macOS this will be /dev/cu.usbserial-xxxxx and /dev/tty.usbserial-xxxxx. Use the /dev/cu.usbserial-xxxxx 
         port as it is the variant that supports blocked reads and writes. 
      b) For ethernet devices, ensure the host machine can ping the hostname or IP address of the NDI device.
   2. Compile and link CAPIsample following the instructions below. The build process will produce a shared 
      library and a sample program that uses the library. Pre-compiled binaries are also supplied in ./bin
   3. From a terminal window run the program as follows:

   On Linux:
          ./bin/linux/capisample <hostname> [<scu_hostname>]
   On Windows 64-bit:
		  bin\win64\sample.exe <hostname> [<scu_hostname>]
   On Mac:
		  ./bin/macosx/CAPIsample <hostname> [<scu_hostname>]
      where
          <hostname> (required) The measurement device's hostname, IP address, or serial port.
          [<scu_hostname>] (optional) A System Control Unit (SCU) hostname, used to connect active tools.
      examples"
          Connecting to device by IP address: 169.254.8.50
          Connecting to device by hostname: P9-B0103.local
          Connecting to serial port varies by OS: 
              COM10 (Win), /dev/ttyUSB0 (Linux), /dev/cu.usbserial-001014FA (Mac)

By default, CAPIsample will connect to a device, initialize it and issue a series of tracking 
API commands that illustrate the basic principles of communicating with the measurement system. 
The application does not issue every possible API command, but focuses on the most common 
and basic tasks including:

   - Connecting to an ethernet or serial port device (via serial-to-USB converter)
   - Initializing the system
   - Loading and initializing passive, active-wireless, and active tools (if an SCU is connected)
   - Getting/setting user parameters stored in the device firmware
   - Sending BX or BX2 to retrieve tracking data and printing it to the terminal in .csv format
   - Printing a small amount of tracking data to a .csv file called "example.csv"

The source code for CAPIsample is provided so it can be analyzed and modified as desired.
Developers may want to:

   - Explore simulating alerts and how the alert data is transmitted to the application
   - Investigate using dummy tools to return 3D data
   - Refer to the API guide for their system to add commands that CAPIsample didn't implement
   - Completely gut and rewrite CAPIsample as a starting point for their project

3.0 Building CAPIsample
=======================
CAPIsample can be compiled on Windows using Visual Studio and on Linux and macOS using make.
A Visual Studio 2017 .sln and Eclipse project files are provided for convenience.

3.1 Required Tools
------------------
Windows:    Microsoft Visual Studio 2017 or later
Linux:      native development tool chain including g++ and make
macOS:      XCode or Gnu development tool chain including g++ and make

Optional - Doxygen (see: http://www.stack.nl/~dimitri/doxygen/index.html)

3.2 Build the CAPI sample binaries
----------------------------------
Open a terminal window and change the current directory to the directory where the capisample package is installed. 
The following terminal commands can be used to compile CAPIsample and its Doxygen documentation.

Windows:
  There are 2 options: 
  Open the solution file with Visual Studio and build the solution from with Visual Studio, or
  Open developer console window from the start menu (search for "dev"). Go to the CAPIsample directory and type
    > msbuild /p:Configuration=Release CAPIsample.sln

Mac and Linux
  Open a terminal window in the CAPIsample directory and type 
    $ make
  
Documentation:
  Doxygen documentation can be built if Doxygen is installed by using the command line and typing
    $ doxygen doxygen.conf
 
4.0 ARDemo
==========================
A video camera unit is an option for the Polaris Vega.  This presents opportunities for combining the tracking 
and video data streams for augmented reality scenarios.  

For customers that have this option, an example of using CAPISample along with the GStreamer framework is provided. 
The ARDemo sample files can be found in the capisample\ndigst directory.

Please see capisample\ndigst\doc\GStreamerNotes.docx for more information. 

Pre built binaries for windows can be found here: capisample\ndigst\bin

The executable ardemo.exe can be run with the appropriate parameters, for example: 
    > ardemo 169.254.176.61 c:/capisample/sroms/ 8700339.rom 554
(change the IP address of the PSU, directory path of rom files, rom file, and video port as needed).

Note that to run this executable, the GStreamer framework needs to be installed and the path environment variable set accordingly.
(See capisample\ndigst\doc\GStreamerNotes.docx for more information).

To build this ARDemo sample:

Windows:
  There are 2 options: 
  Open the capisample\ndigst\ndigst.sln solution file with Visual Studio and build the solution from with Visual Studio (one of the x64 configurations), or
  Open developer console window from the start menu (search for "dev"). Go to the capisample\ndigst directory and type
    > msbuild /p:Configuration=Release ndigst.sln

