/*
 *
 * Copyright (C) 2019, Northern Digital Inc. All rights reserved.
 *
 * All Northern Digital Inc. ("NDI") Media and/or Sample Code and/or Sample Code
 * Documentation (collectively referred to as "Sample Code") is licensed and provided "as
 * is" without warranty of any kind. The licensee, by use of the Sample Code, warrants to
 * NDI that the Sample Code is fit for the use and purpose for which the licensee intends to
 * use the Sample Code. NDI makes no warranties, express or implied, that the functions
 * contained in the Sample Code will meet the licensee's requirements or that the operation
 * of the programs contained therein will be error free. This warranty as expressed herein is
 * exclusive and NDI expressly disclaims any and all express and/or implied, in fact or in
 * law, warranties, representations, and conditions of every kind pertaining in any way to
 * the Sample Code licensed and provided by NDI hereunder, including without limitation,
 * each warranty and/or condition of quality, merchantability, description, operation,
 * adequacy, suitability, fitness for particular purpose, title, interference with use or
 * enjoyment, and/or non infringement, whether express or implied by statute, common law,
 * usage of trade, course of dealing, custom, or otherwise. No NDI dealer, distributor, agent
 * or employee is authorized to make any modification or addition to this warranty.
 * In no event shall NDI nor any of its employees be liable for any direct, indirect,
 * incidental, special, exemplary, or consequential damages, sundry damages or any
 * damages whatsoever, including, but not limited to, procurement of substitute goods or
 * services, loss of use, data or profits, or business interruption, however caused. In no
 * event shall NDI's liability to the licensee exceed the amount paid by the licensee for the
 * Sample Code or any NDI products that accompany the Sample Code. The said limitations
 * and exclusions of liability shall apply whether or not any such damages are construed as
 * arising from a breach of a representation, warranty, guarantee, covenant, obligation,
 * condition or fundamental term or on any theory of liability, whether in contract, strict
 * liability, or tort (including negligence or otherwise) arising in any way out of the use of
 * the Sample Code even if advised of the possibility of such damage. In no event shall
 * NDI be liable for any claims, losses, damages, judgments, costs, awards, expenses or
 * liabilities of any kind whatsoever arising directly or indirectly from any injury to person
 * or property, arising from the Sample Code or any use thereof
 *
 */

 /**
  * cpp class that manages communication with the ndi device therough the CAPI library.
  * This file was adapted from main.cpp in the sample directory of CAPISample
  *
  */

/* for sscanf (sscanf_s is not cross platform) */
#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>

#include "tooltracking.h"

const std::string ToolTracking::LENS_6D = "VCU-0.Param.Lens.6D.";
const std::string ToolTracking::LENS_DISTORTION = "VCU-0.Param.Lens.Distortion.";
const std::string ToolTracking::LENS_PINHOLE = "VCU-0.Param.Lens.Pinhole.";
const std::string ToolTracking::IMAGE_AREA_LEFT = "VCU-0.Param.Left";
const std::string ToolTracking::IMAGE_AREA_TOP = "VCU-0.Param.Top";
const std::string ToolTracking::SENSOR_BINNING_X = "VCU-0.Param.Binning X";
const std::string ToolTracking::SENSOR_BINNING_Y = "VCU-0.Param.Binning Y";
const std::string ToolTracking::VCU_STREAMING = "VCU-0.Param.Allow Streaming";
const std::string error = "ERROR";

/**
 * ToolTracking:
 * @hostname: The measurement device's hostname, for example P9-00311.local
 * @rtspVideoPort: the rtsp video port, for example, 554
 * @toolLocation: Directory location of the tool files that will be loaded
 *
 * constructor for tool tracking object
 */
ToolTracking::ToolTracking (std::string hostname, std::string rtspVideoPort, std::string toolLocation)
  : _hostname (hostname), _rtspVideoPort (rtspVideoPort), _toolLocation (toolLocation)
{
  _capi = new CombinedApi ();
}

ToolTracking::~ToolTracking ()
{
  delete _capi;
  _capi = NULL;
}

/**
 * AddToolFile:
 * @toolfilename: Name of a tool rom file to load
 *
 * Adds a tool file to load
 */
void ToolTracking::AddToolFile (std::string toolfilename)
{
  _toolFiles.push_back (toolfilename);
}

/**
 * InitializeTracking:
 *
 * performs needed admin to initialize tool tracking
 *
 * Returns: TRUE on success.
 */
bool ToolTracking::InitializeTracking ()
{
  // Attempt to connect to the device
  if (_capi->connect (_hostname) != 0)
  {
    // Print the error and exit if we can't connect to a device
    std::cout << "Connection Failed!" << std::endl;
    return false;
  }
  std::cout << "Connected!" << std::endl;

  // Determine if the connected device supports the BX2 command
  determineApiSupportForBX2 ();

  // Print the firmware version for debugging purposes
  std::cout << _capi->getUserParameter ("Features.Firmware.Version") << std::endl;

  // Initialize the system. This clears all previously loaded tools, unsaved settings etc...
  onErrorPrintDebugMessage ("_capi->initialize()", _capi->initialize ());

  // Various tool types are configured in slightly different ways
  configurePassiveTools ();

  // Once loaded or detected, tools are initialized and enabled the same way
  initializeAndEnableTools ();

  // get the lens parameters for the vcu
  getLensParameters ();

  // set track frequency to 60 Hz
  _capi->setUserParameter ("Param.Tracking.Track Frequency", "2");
  getTrackingFrameFrequency ();

  enableVCUStreaming ();

  return true;
}

/**
 * getUserParameterAsDouble:
 *
 * @userParam: the user parameter
 *
 * a helper function to get a double from a user parameter
 *
 * Returns: user param as double.
 */
double ToolTracking::getUserParameterAsDouble (std::string userParam)
{
  double doubleOut = 0.0;
  std::string numberout = _capi->getUserParameter (userParam);
  size_t len_output = numberout.size ();
  if (len_output > 0 && numberout.find (error) == std::string::npos)
  {
    std::string format = userParam + "=%lf";
    const char *theFormat = format.c_str ();
    sscanf (numberout.c_str (), theFormat, &doubleOut);
  }
  return doubleOut;
}

/**
 * getUserParameterAsInt:
 *
 * @userParam: the user parameter
 *
 * a helper function to get a integer from a user parameter
 *
 * Returns: user param as integer.
 */
int ToolTracking::getUserParameterAsInt (std::string userParam)
{
  int intOut = 0;
  std::string numberout = _capi->getUserParameter (userParam);
  size_t len_output = numberout.size ();
  if (len_output > 0 && numberout.find (error) == std::string::npos)
  {
    std::string format = userParam + "=%d";
    const char *theFormat = format.c_str ();
    sscanf (numberout.c_str (), theFormat, &intOut);
  }
  return intOut;
}

/**
 * getLensParameters:
 *
 * queries the device for all the relevant lens parameters and stores 
 * them in a local object
 */
void ToolTracking::getLensParameters ()
{
  _lensParams.vcu_align.q0 = getUserParameterAsDouble (LENS_6D + std::string ("q0"));
  _lensParams.vcu_align.qx = getUserParameterAsDouble (LENS_6D + std::string ("qx"));
  _lensParams.vcu_align.qy = getUserParameterAsDouble (LENS_6D + std::string ("qy"));
  _lensParams.vcu_align.qz = getUserParameterAsDouble (LENS_6D + std::string ("qz"));

  _lensParams.vcu_trans.tx = getUserParameterAsDouble (LENS_6D + std::string ("tx"));
  _lensParams.vcu_trans.ty = getUserParameterAsDouble (LENS_6D + std::string ("ty"));
  _lensParams.vcu_trans.tz = getUserParameterAsDouble (LENS_6D + std::string ("tz"));

  _lensParams.k1 = getUserParameterAsDouble (LENS_DISTORTION + std::string ("k1"));
  _lensParams.k2 = getUserParameterAsDouble (LENS_DISTORTION + std::string ("k2"));
  _lensParams.k3 = getUserParameterAsDouble (LENS_DISTORTION + std::string ("k3"));
  _lensParams.p1 = getUserParameterAsDouble (LENS_DISTORTION + std::string ("p1"));
  _lensParams.p2 = getUserParameterAsDouble (LENS_DISTORTION + std::string ("p2"));

  _lensParams.fu = getUserParameterAsDouble (LENS_PINHOLE + std::string ("fu"));
  _lensParams.fv = getUserParameterAsDouble (LENS_PINHOLE + std::string ("fv"));
  _lensParams.u0 = getUserParameterAsDouble (LENS_PINHOLE + std::string ("u0"));
  _lensParams.v0 = getUserParameterAsDouble (LENS_PINHOLE + std::string ("v0"));

  _lensParams.res_char.binX = getUserParameterAsInt (SENSOR_BINNING_X);
  _lensParams.res_char.binY = getUserParameterAsInt (SENSOR_BINNING_Y);
  _lensParams.res_char.left = getUserParameterAsInt (IMAGE_AREA_LEFT);
  _lensParams.res_char.top = getUserParameterAsInt (IMAGE_AREA_TOP);
}

/**
 * getTrackingFrameFrequency:
 *
 * queries the device for the track frequency and stores it locally
 *
 * Returns: The tracking frequency
 */
void ToolTracking::getTrackingFrameFrequency ()
{
  int freq_enum_out = getUserParameterAsInt ("Param.Tracking.Track Frequency");
  switch (freq_enum_out) {
    case 0:
      _trackingFrequency = 20;
      break;
    case 1:
      _trackingFrequency = 30;
      break;
    case 2:
      _trackingFrequency = 60;
      break;
    default:
      _trackingFrequency = 20;
      break;
  }
}

/**
 * copyLensParameters:
 *
 * @paramsout: the supplied object that is loaded with the data
 *
 * retrieve the lens parameters 
 */
void ToolTracking::copyLensParameters (LensParams *paramsout)
{
  *paramsout = _lensParams;
}

/**
 * queryTrackFrequency:
 *
 * gets the track frequency
 *
 * Returns: tracking frequency
 */
int ToolTracking::queryTrackFrequency ()
{
  return _trackingFrequency;
}

/**
 * getRTSPVideoPort:
 *
 * gets the track frequency
 *
 * Returns: the RTSP video port
 */
std::string ToolTracking::getRTSPVideoPort ()
{
  return _rtspVideoPort;
}

/**
 * determineApiSupportForBX2:
 *
 * Determines whether an NDI device supports the BX2 command 
 * by looking at the API revision
 */
void ToolTracking::determineApiSupportForBX2 ()
{
  // Lookup the API revision
  std::string response = _capi->getApiRevision ();

  // Refer to the API guide for how to interpret the APIREV response
  char deviceFamily = response[0];
  int majorVersion = _capi->stringToInt (response.substr (2, 3));

  // As of early 2017, the only NDI device supporting BX2 is the Vega
  // Vega is a Polaris device with API major version 003
  if (deviceFamily == 'G' && majorVersion >= 3)
  {
    _apiSupportsBX2 = true;
  }
}

/**
 * onErrorPrintDebugMessage:
 *
 * Prints a debug message if a method call failed.
 * To use, pass the method name and the error code returned by the method.
 * Eg: onErrorPrintDebugMessage("_capi->initialize()", _capi->initialize());
 * If the call succeeds, this method does nothing.
 * If the call fails, this method prints an error message to stdout.
 */
void ToolTracking::onErrorPrintDebugMessage (std::string methodName, int errorCode)
{
  if (errorCode < 0)
  {
    std::cout << methodName << " failed: " << _capi->errorToString (errorCode) << std::endl;
  }
}

/**
 * configurePassiveTools:
 *
 * Loading of passive tools.
 * Passive tools use NDI spheres to passively reflect IR light to the cameras.
 */
void ToolTracking::configurePassiveTools ()
{
  std::cout << std::endl << "Configuring Passive Tools - Loading .rom Files..." << std::endl;
  for (int i = 0; i < _toolFiles.size (); i++)
  {
    std::string toolfile = _toolFiles[i];
    std::cout << std::endl << "Loading tool: " << toolfile << std::endl;
    // TODO: check trailing / etc.
    std::string fullFilePath = _toolLocation + toolfile;
    loadTool (fullFilePath.c_str ());
  }
}

/**
 * loadTool:
 *
 * @toolDefinitionFilePath: filepath of the tool file
 *
 * Loads a tool from a tool definition file (.rom)
 */
void ToolTracking::loadTool (const char* toolDefinitionFilePath)
{
  // Request a port handle to load a passive tool into
  int portHandle = _capi->portHandleRequest ();
  onErrorPrintDebugMessage ("_capi->portHandleRequest()", portHandle);

  // Load the .rom file using the previously obtained port handle
  _capi->loadSromToPort (toolDefinitionFilePath, portHandle);
}

/**
 * initializeAndEnableTools:
 *
 * Initialize and enable loaded tools. This is the same regardless of tool type.
 */
void ToolTracking::initializeAndEnableTools ()
{
  std::cout << std::endl << "Initializing and enabling tools..." << std::endl;

  // Initialize and enable tools
  std::vector<PortHandleInfo> allPortHandles = _capi->portHandleSearchRequest (PortHandleSearchRequestOption::NotInit);
  for (int i = 0; i < allPortHandles.size (); i++)
  {
    onErrorPrintDebugMessage ("_capi->portHandleInitialize()", _capi->portHandleInitialize (allPortHandles[i].getPortHandle ()));
    onErrorPrintDebugMessage ("_capi->portHandleEnable()", _capi->portHandleEnable (allPortHandles[i].getPortHandle ()));
  }

  // Print all enabled tools
  _portHandles = _capi->portHandleSearchRequest (PortHandleSearchRequestOption::Enabled);
  for (int i = 0; i < _portHandles.size (); i++)
  {
    std::cout << _portHandles[i].toString () << std::endl;
  }
}

/**
 * enableVCUStreaming:
 *
 * sets up the device to stream video data
 */
void ToolTracking::enableVCUStreaming ()
{
  int streamingVal = getUserParameterAsInt (VCU_STREAMING);
  if (streamingVal == 0)
  {
    _capi->setUserParameter (VCU_STREAMING, "1");
  }
}

/**
 * getToolTrackingData:
 *
 * returns the tool tracking data
 *
 * Returns: a vector of tracking data
 */
std::vector<ToolData> ToolTracking::getToolTrackingData ()
{
  std::vector<ToolData> newToolData = _apiSupportsBX2 ? _capi->getTrackingDataBX2 ("--6d=tools --3d=tools") :
    _capi->getTrackingDataBX (TrackingReplyOption::TransformData | TrackingReplyOption::AllTransforms);

  return newToolData;
}

/**
 * getConnectionName:
 *
 * Gets the connection name.  For a TCP connection, this will be the IP4 address (ex. 169.254.99.8)
 *
 * Returns: the connection name
 */
char *ToolTracking::getConnectionName ()
{
  return _capi->getConnectionName ();
}

/**
 * StartTracking:
 *
 * start tracking tool data
 */
void ToolTracking::StartTracking ()
{
  onErrorPrintDebugMessage ("_capi->startTracking()", _capi->startTracking ());
}

/**
 * StopTracking:
 *
 * stop tracking tool data
 */
void ToolTracking::StopTracking ()
{
  onErrorPrintDebugMessage ("pCapi->stopTracking()", _capi->stopTracking ());
}