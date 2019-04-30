# Command Line Interface (CLI) & OpenSSL Engine for OPTIGA™ Trust X security solution

1. [About](#about)
    * [Prerequisites](#prerequisites)
2. [Getting Started](#getting_started)
    * [First time building the library](#build_lib)
    * [Building the engine](#build_engine) 
3. [CLI Tools Usage](#cli_usage)
    * [trustx_chipinfo](#trustx_chipinfo)
    * [trustx_data](#trustx_data)
    * [trustx_readmetadata_data](#trustx_readmetadata_data)
    * [trustx_readmetadata_status](#trustx_readmetadata_status)
    * [trustx_readmetadata_private](#trustx_readmetadata_private)
4. [Trust X OpenSSL Engine usage](#engine_usage)
5. [Known issues](#known_issues)

## <a name="about"></a>About

    This is a command line tools tools and OpenSSL Engine for OPTIGA Trust M1 on Linux platform.


### <a name="prerequisites"></a>Prerequisites

    Following is the software component to build the tools :
	- GCC
	- OpenSSL development library (libssl-dev)
	- OpenSSL 1.1.X
	- OPTIGA Trust M1 library (source code)
	- wiringPI
	- pthread
	- rt

    Tested platforms:
      - Raspberry PI 3 on Linux kernal 4.19

## <a name="getting_started"></a>Getting Started
### <a name="build_lib"></a>First time building the library
```console 
foo@bar:~$ sudo make install_debug_lib
```
or 
```console 
foo@bar:~$ sudo make install_lib
```

### <a name="build_engine"></a>Building the engine
```console 
foo@bar:~$ sudo make install_debug_engine
```
or 
```console 
foo@bar:~$ sudo make install_engine
```

Note:
- If debug is used subsequence building just do a ‘make’ as the Makefile creates a 
  soft-link to the bin directory
- If without debug than every time you build the library or engine you must reinstall

## <a name="cli_usage"></a>CLI Tools Usage
### <a name="trustx_chipinfo"></a>trustx_chipinfo
    Display the trustx chip information.

### <a name="trustx_data"></a>trustx_data
    Read/Write/Erase OID data object
	Help menu: trustx_data <option> ...<option>
	option:- 
	-r <OID>      : Read from OID 0xNNNN 
	-w <OID>      : Write to OID
	-i <filename> : Input file 
	-o <filename> : Output file 
	-p <offset>   : Offset position 
	-e            : Erase and wirte 
	-h            : Print this help

### <a name="trustx_readmetadata_data"></a>trustx_readmetadata_data
    Read all data object metadata
          
### <a name="trustx_readmetadata_status"></a>trustx_readmetadata_status
    Read all data object metadata

### <a name="trustx_readmetadata_private"></a>trustx_readmetadata_private
    Read all data object metadata

## <a name="engine_usage"></a>Trust X1 OpenSSL Engine usage
