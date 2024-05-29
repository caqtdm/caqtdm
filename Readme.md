# Overview
__caQtDM__ offers a collection of special widgets in order to display and control control system data in various ways. The widgets have been inspired from the well known MEDM package used
at many EPICS sites. However caQtDM is not restricted to EPICS and can/could also support other control systems. At PSI caQtDM is used at several facilities and controls not only EPICS, but also simultaneously
an older house-made system. Moreover support has already been introduced for EPICS4, but is still in development.

# Usage
In case of existing MEDM files, these can be translated to Qt xml files by using the utility adl2ui contained in the package. Normally nothing will have to be changed in the resulting files, but in case of small problems, like for instance in overlapping order, the Qt4-designer can be used to correct these. However when you start from scratch, you will develop your screens with the Qt4-designer and display them by the caQtDM utility, which takes care of the data acquisition/control and the display. The conventions for the widgets and channels are fully compatible with MEDM and some other widgets have been added. The MEDM manual will give you normally all the necessary explanations for the different widgets while the mechanisms are the same. However you will have to get accustomed with the Qt4-designer utility and a quick look at its manual delivered with it, should give you an idea.

# Build / Install / Run

## Linux
For building and installing caqtdm following requirements have to met:

1. Qt 4.8.2
2. Qwt 6.0.1
3. EPICS 3.12.2
4. Designer

or

1. Qt 6.4
2. Qwt 6.2.0
3. EPICS 7.0.6
4. Designer

The latest version of caQtDM has build and installation procedure working for several linux platforms. First you will have to unpack the tar archive, which will create the necessary directories and files. In the upper directory you will find the necessary procedures to build the package, run it locally without installing and also an installation procedure.

  1. you will have to install Qt-4.8 and qwt-6 when they are not already installed. On modern systems Qt4 should already be installed, but the qt4-designer not necessarily and you should install it (eventually with sudo apt-get install qt4-designer). qwt is normally not installed and you should install this too.

  2. you will have to install EPICS

  3. all the environment variables used for the installation will be defined in the script caQtDM_Env. When some of the variables are already defined in your environment, the script will use them, otherwise they will be defined. It is up to you to edit this file and adapt the variables to your environment.

  4. the script caQtDM_BuildAll will build you the package now into the directory ./caQtDM_Binaries. You should obtain there adl2ui, caQtDM, libqtcontrols.so and libcaQtDM_Lib.so as well as a designer directory containing 3 files `libqtcontrols*_plugin.so`. If you encounter a problem when building due to some environment variable problem or while not everything was installed before, you may correct your problem and run the script again. In case you want to cleanup you use the script caQtDM_CleanAll, which will also delete the resulting binaries.

  5. you may try to install the package: the package should normally be installed in the EPICS extensions and the environment variables are pointing to them.

Running caQtDM: when the package has been successfully build into the directory ./caQtDM_Binaries, you can start caQtDM by using the script startDM_Local
and the qt4-designer by the script qtdesigner (all located in the main directory where you expanded the tar file). These scripts will set some important environment variables in order to set the path, the plugin_path and to point to the test directory provided with the package (./caQtDM_Tests). In this directory also the stylesheets are located to be used for a nice rendering. You may call the test.ui file with the above mentioned scripts. In order to test the display some EPICS channels have to be defined. This can be done by running a soft-ioc shell called with run-epics also provided in the test directory.

In case you have qwt 6.1 you will have to use in caQtDM_QtControls/src the files qwt_thermo_marker_61.* instead of qwt_thermo_marker.*

In case you already use Qt5.15 with qwt6.1.5 the building should also be straight forward.

good luck and enjoy

In case of problems, please contact Helge Brands at PSI.

## Windows
To compile and run caQtDM on Windows following requirements have to be met:

1. Qt 4.8.2 - Qt 6.4.0
2. Qwt 6.0.1 - Qwt 6.2.0
3. EPICS 7.0.6
4. MS Visual Studio 2013 and MS Visual Studio 2019 
5. Wix 3.0.5419.0

With caQtDM_Env.bat you can configure your system. All setting which are needed will be taken from here.

The building can be started with the batchfile 	: caQtDM_BuildAll.bat
The packaging is made with the batch file 	: caQtDM_Build_Package.bat
This will generate a MSI installation file in the folder of caQtDM_Binaries.
To clean the Folder you should use caQtDM_CleanAll.bat.


# Environment Variables
Following environment variables can be used to configure caqtdm:

- __QT_PLUGIN_PATH__ - to find the plugins of qt and others

- __EPICS_CA_ADDR_LIST__ - see EPICS Documentation 
- __EPICS_CA_MAX_ARRAY_BYTES__ - see EPICS Documentation

- __CAQTDM_LAUNCHFILE__ - Enviroment file for Mobile devices

- __CAQTDM_TIMEOUT_HOURS__ to exit caQtDM after some amount of time
- __CAQTDM_DISPLAY_PATH__ - paths to look for ui and stylesheet files
- __CAQTDM_URL_DISPLAY_PATH__ - paths to look for ui and stylesheet files to download via http
- __CAQTDM_MIME_PATH__ - path to MIME file

- __CAQTDM_EXEC_LIST__ - execution list for context menu
- __MEDM_EXEC_LIST__ - for backwards......

- __BSREAD_DISPATCHER__ - point the bsread plugin to the dispatcher 
- __BSREAD_ZMQ_CONNECTION_TYPE__ - control the connection type of the bsread plugin 
- __BSREAD_ZMQ_ADDR_LIST__ - point the bsread plugin static sources 

- __CAQTDM_OPTIMIZE_EPICS3CONNECTIONS__ - Disable Epics3 connections when tabwidget is not active, set to "TRUE" to activate                                 |
- __CAQTDM_MODBUS_DATABASE__ - Database to use for the modbus plugin 

- __CAQTDM_ARCHIVERSF_URL__ - point the archiver plugin to a different archiver backend 

- __CAQTDM_FINDRECORD_SRV__ - for autocompletion, the request URL
- __CAQTDM_FINDRECORD_FACILITY__ - search limitation for a facility  
- __CAQTDM_FINDRECORD_LIMIT__ - search limit max number of entries

- __CAQTDM_FINDRECORD_DIRECT__ - override all other find record settings (direct json http download)

- __CAQTDM_DEFAULT_UNIT_REPLACEMENTS__ - if set to "false", default unit replacements (°/µ) are disabled.

- __CAQTDM_CUSTOM_UNIT_REPLACEMENTS__ - define custom unit replacements. They are replaced after default replacements took place, if enabled. You can use unicode characters or hexadecimal / decimal utf-8 character codes, seperated by (,) , (=) and (;).
Examples: "0xba,C=55,abc,0xb0;cd=23;0x43=0x44" , "°=0xba"

- __CAQTDM_SUPPRESS_UPDATES_ONLOAD__ - Disables widgets from being updated while a file is being opened. This can reduce load times of big panels by more than 50. Values: "TRUE", "FALSE" , without quotes 
- __CAQTDM_CREATE_LOGFILE__ - If set to "TRUE", caQtDM will create a logfile containing all of the input from the message window. If caQtDM exits successfully, this file gets deleted after termination.          |
- __CAQTDM_LOGFILE_PATH__ - This specifies the path where the logfile, if logging is active, will be stored.
