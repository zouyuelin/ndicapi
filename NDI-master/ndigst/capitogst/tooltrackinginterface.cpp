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
  * these functions are the interface to the cpp ToolTracking class. Only the functions
  * needed by the gstreamer elements are defined here, and are exported as plain c 
  *
  */

/*#define VERIFY_FLATBUFFER*/

#include <stdio.h>
#include "tooltracking.h"
#include "tooltrackinginterface.h"
#include "flattooldata_builder.h"
#if defined(VERIFY_FLATBUFFER)
#include "flattooldata_verifier.h"
#endif

#define ALLOC_ALIGNED_FLATBUFFER

struct ToolDataExp
{
  void *obj; /* the ToolTracking opaque object*/
  flatcc_builder_t _builder; /* FlatBuffer builder object for creating 
                                track data for the gstreamer elements */
};

/**
 * CreateToolTracker:
 * @hostname: The measurement device's hostname, for example P9-00311.local
 * @RTSPVideoPort: the rtsp video port, for example, 554
 * @toolDir: Directory location of the tool files that will be loaded
 * @toolFile: Name of a tool rom file to load
 *
 * creates the tool tracking object
 */
ToolDataExpHandle CreateToolTracker (const char *hostname, const char *RTSPVideoPort, const char *toolDir, const char *toolFile)
{
  ToolDataExpHandle toolwrapper = new ToolDataExp ();
  ToolTracking* toolTracking = new ToolTracking (hostname ? hostname : "", RTSPVideoPort ? RTSPVideoPort : "", toolDir ? toolDir : "");
  toolwrapper->obj = toolTracking;
  if (toolFile)
    toolTracking->AddToolFile (toolFile);
  if (!toolTracking->InitializeTracking ())
  {
    delete toolTracking;
    toolwrapper->obj = NULL;
    delete toolwrapper;
    toolwrapper = NULL;
    return NULL;
  }
  flatcc_builder_init (&(toolwrapper->_builder));
  return toolwrapper;
}

/**
 * StartTracking:
 *
 * @trackHandle: the tool tracking object
 *
 * start tracking tool data
 */
void StartTracking (ToolDataExpHandle trackHandle)
{
  if (!trackHandle || !trackHandle->obj)
    return;
  ToolTracking* tracking = (ToolTracking *) (trackHandle->obj);
  tracking->StartTracking ();
}

/**
 * StartTracking:
 *
 * @trackHandle: the tool tracking object
 *
 * start tracking tool data
 */
void StopTracking (ToolDataExpHandle trackHandle)
{
  if (!trackHandle || !trackHandle->obj)
    return;
  ToolTracking* tracking = (ToolTracking *) (trackHandle->obj);
  tracking->StopTracking ();
  flatcc_builder_clear (&trackHandle->_builder);
}

/**
 * StartTracking:
 *
 * @trackHandle: the tool tracking object
 *
 * start tracking tool data
 */
void DeleteToolTracker (ToolDataExpHandle trackHandle)
{
  if (!trackHandle || !trackHandle->obj)
    return;
  ToolTracking* tracking = (ToolTracking *) (trackHandle->obj);
  delete tracking;
  delete trackHandle;
}

/**
 * StartTracking:
 *
 * @trackHandle: the tool tracking object
 *
 * start tracking tool data
 */
void GetLensParams (ToolDataExpHandle trackHandle, LensParams *lensParams)
{
  if (!trackHandle || !trackHandle->obj)
    return;
  ToolTracking* tracking = (ToolTracking *) (trackHandle->obj);
  tracking->copyLensParameters (lensParams);
}

/**
 * GetTrackFrequency:
 *
 * @trackHandle: the tool tracking object
 *
 * gets the track frequency
 */
int GetTrackFrequency (ToolDataExpHandle trackHandle)
{
  if (!trackHandle || !trackHandle->obj)
    return 0;
  ToolTracking* tracking = (ToolTracking *) (trackHandle->obj);
  return tracking->queryTrackFrequency ();
}

/* Convenient namespace macro to manage long namespace prefix. */
#undef ns
#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(NDI_FlatToolData, x) /* Specified in the schema. */

/**
 * build_ToolData:
 *
 * @B: FlatBuffer builder object
 * @tooldata: the tool data from the CAPISample API
 *
 * private function used by GetToolData. 
 * Creates the tooldata as a FlatBuffer given the starting CAPISample data
 */
void build_ToolData (flatcc_builder_t *B, const std::vector<ToolData> &tooldata)
{
  ns (ToolData_vec_start (B));

  for (int i=0; i < tooldata.size(); i++)
  {
    ToolData tdata = tooldata[i];
    ns (ToolData_vec_push_start (B));
    ns (Transform_t) data_transform = { tdata.transform.toolHandle,
                      tdata.transform.status,
                      tdata.transform.q0,
                      tdata.transform.qx,
                      tdata.transform.qy,
                      tdata.transform.qz,
                      tdata.transform.tx,
                      tdata.transform.ty,
                      tdata.transform.tz,
                      tdata.transform.error };
    ns (ToolData_transform_add (B, &data_transform));
    ns (ToolData_framenumber_add (B, tdata.frameNumber));
    ns (ToolData_timespec_s_add (B, tdata.timespec_s));
    ns (ToolData_timespec_ns_add (B, tdata.timespec_ns));
    ns (Marker_vec_start (B));
    for (int i = 0; i < tdata.markers.size(); i++)
    {
      MarkerData mdata = tdata.markers[i];
      ns (Marker_t) markerflat = { mdata.status, mdata.markerIndex, mdata.x, mdata.y, mdata.z };
      ns (Marker_vec_push (B, &markerflat));
    }
    ns (Marker_vec_ref_t) markersflat = ns (Marker_vec_end (B));
    ns (ToolData_markers_add (B, markersflat));
    ns (ToolData_vec_push_end (B));
  }

  ns (ToolData_vec_ref_t) tooldata_flat_vec = ns (ToolData_vec_end (B));
  ns (ToolDataWrapper_create_as_root (B, tooldata_flat_vec));
}

/**
 * GetToolData:
 *
 * @trackHandle: the tool tracking object
 * @size_out: returns the size of the buffer
 *
 * Gets the tooldata as a flatbuffer. 
 *
 * Returns: a FlatBuffer of tool data
 */
void *GetToolData (ToolDataExpHandle trackHandle, size_t *size_out)
{
  ToolTracking* tracking = (ToolTracking *) (trackHandle->obj);

  *size_out = 0;
  /* first use the CAPISample API to get the tracking data */
  std::vector<ToolData> tooldata = tracking->getToolTrackingData ();
  if (tooldata.size () == 0)
  {
    return NULL;
  }
  void  *buf;
  flatcc_builder_reset (&trackHandle->_builder);

  build_ToolData (&trackHandle->_builder, tooldata);
#if defined(ALLOC_ALIGNED_FLATBUFFER)
  buf = flatcc_builder_finalize_aligned_buffer (&trackHandle->_builder, size_out);
#else
  buf = flatcc_builder_finalize_buffer (&trackHandle->_builder, size_out);
#endif

#if defined(VERIFY_FLATBUFFER)
  if (!NDI_FlatToolData_ToolDataWrapper_verify_as_root (buf, *size_out))
  {
    printf ("error\n");
  }
#endif
  return buf;
}

/**
 * GetVideoSource:
 *
 * @trackHandle: the tool tracking object
 * @addressBuffer: a place to store the URI 
 * @buffer_size: the size of the supplied addressBuffer
 *
 * Determines the URI of the video stream based on the device and port
 *
 */
void GetVideoSource (ToolDataExpHandle trackHandle, char* addressBuffer, size_t buffer_size)
{
  if (!trackHandle || !trackHandle->obj)
    return;
  ToolTracking* tracking = (ToolTracking *) (trackHandle->obj);
  char *connectionName = tracking->getConnectionName ();
  std::string port_name = tracking->getRTSPVideoPort ();
#pragma warning( disable : 4996)
#pragma warning( push )
  sprintf (addressBuffer, "rtsp://%s:%s/video", connectionName, port_name.c_str ());
#pragma warning( pop )
}

/**
 * FreeToolData:
 *
 * @data: the FlatBuffer of tool data
 *
 * Frees buffer
 *
 */
void FreeToolData (void *data)
{
#if defined(ALLOC_ALIGNED_FLATBUFFER)
  flatcc_builder_aligned_free (data);
#else
  free (data);
#endif
}

