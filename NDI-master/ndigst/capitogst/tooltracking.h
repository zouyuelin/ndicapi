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

#ifndef TOOL_TRACKING_H
#define TOOL_TRACKING_H

#include "CombinedApi.h"
#include "nditransformbase.h" 

struct ToolTracking
{
public:

  static const std::string LENS_6D;
  static const std::string LENS_DISTORTION;
  static const std::string LENS_PINHOLE;
  static const std::string IMAGE_AREA_LEFT;
  static const std::string IMAGE_AREA_TOP;
  static const std::string SENSOR_BINNING_X;
  static const std::string SENSOR_BINNING_Y;
  static const std::string VCU_STREAMING;

  ToolTracking (std::string hostname, std::string rtspVideoPort, std::string toolLocation);
  ~ToolTracking ();
  void AddToolFile (std::string toolfilename);
  bool InitializeTracking ();
  std::vector<ToolData> getToolTrackingData ();
  char *getConnectionName ();
  void StartTracking ();
  void StopTracking ();
  void getLensParameters ();
  void getTrackingFrameFrequency ();
  void copyLensParameters (LensParams *paramsout);
  int queryTrackFrequency ();
  std::string getRTSPVideoPort ();

private:
  double getUserParameterAsDouble (std::string userParam);
  int getUserParameterAsInt (std::string userParam);
  void determineApiSupportForBX2 ();
  void onErrorPrintDebugMessage (std::string methodName, int errorCode);
  void configurePassiveTools ();
  void loadTool (const char* toolDefinitionFilePath);
  void initializeAndEnableTools ();
  void enableVCUStreaming ();

  std::string _hostname;
  std::string _rtspVideoPort;
  std::string _toolLocation;
  std::vector<std::string> _toolFiles;
  CombinedApi *_capi;
  bool _apiSupportsBX2;
  std::vector<PortHandleInfo> _portHandles;
  LensParams _lensParams;
  int _trackingFrequency;
};

#endif // TOOL_TRACKING_H

