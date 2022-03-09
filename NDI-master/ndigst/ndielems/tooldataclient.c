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
 * Utility functions to retrieve information from the buffer 
 * holding the tool information.
 *
 */

#include "tooldataclient.h"
#include "flattooldata_reader.h"

/* Convenient namespace macro to manage long namespace prefix. */
#undef ns
#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(NDI_FlatToolData, x) /* Specified in the schema. */

/**
 * GetTimeInfoFromBuffer:
 * @buffer: buffer holding tooldata - a flatbuffer
 * @framenumbers: supplied array to store frame numbers
 * @time_specs: supplied array to store timestamps
 * @bufferslen: size of the framenumbers and time_specs array (both  
 *              are expected to be the same size)
 * @input_buffer_too_small: output flag, is set to true if input buffer
 *                          size was too small to store all the data
 *
 * retrieves the timestamps and frame numbers of a tool data buffer
 *
 */
size_t GetTimeInfoFromBuffer (void *buffer, unsigned int *framenumbers, struct timespec *time_specs, size_t bufferslen, bool *input_buffer_too_small)
{
  *input_buffer_too_small = false;
  ns (ToolDataWrapper_table_t) trackdata = ns (ToolDataWrapper_as_root (buffer));
  if (!trackdata)
  {
    return 0;
  }
  ns (ToolData_vec_t) tooldataarray = ns (ToolDataWrapper_tooldatas (trackdata));
  size_t inv_len = ns (ToolData_vec_len (tooldataarray));
  size_t toolcnt = 0;
  for (size_t i = 0; i < inv_len; i++)
  {
    ns (ToolData_table_t) tdata = ns (ToolData_vec_at (tooldataarray, i));
    ns (Transform_struct_t) transform = ns (ToolData_transform (tdata));
    /* Bit 8 indicates if the transform is missing */
    if (transform->status & 0x0100)
    {
      continue;
    }
    if (toolcnt >= bufferslen)
    {
      *input_buffer_too_small = true;
      break;
    }
    unsigned int frame_number = ns (ToolData_framenumber (tdata));
    unsigned int time_s = ns (ToolData_timespec_s (tdata));
    unsigned int time_ns = ns (ToolData_timespec_ns (tdata));
    time_specs[toolcnt].tv_sec = time_s;
    time_specs[toolcnt].tv_nsec = time_ns;
    framenumbers[toolcnt] = frame_number;
    toolcnt++;
  }
  return toolcnt;
}

/**
 * GetToolPosFromBuffer:
 * @buffer: buffer holding tooldata - a flatbuffer
 * @toolpos: supplied array to store tool positions
 * @bufferslen: size of the toolpos array
 * @input_buffer_too_small: output flag, is set to true if input buffer
 *                          size was too small to store all the data
 *
 * retrieves the tool positions of a tool data buffer
 *
 */
size_t GetToolPosFromBuffer (void *buffer, Position *toolpos, size_t bufferslen, bool *input_buffer_too_small)
{
  *input_buffer_too_small = false;
  ns (ToolDataWrapper_table_t) trackdata = ns (ToolDataWrapper_as_root (buffer));
  ns (ToolData_vec_t) tooldataarray = ns (ToolDataWrapper_tooldatas (trackdata));
  size_t inv_len = ns (ToolData_vec_len (tooldataarray));
  size_t tool_num = 0;
  bool toolposfull = false;

  for (size_t i = 0; i < inv_len; i++)
  {
    ns (ToolData_table_t) tdata = ns (ToolData_vec_at (tooldataarray, i));
    ns (Transform_struct_t) transform = ns (ToolData_transform (tdata));
    /* Bit 8 indicates if the transform is missing */
    if (transform->status & 0x0100)
    {
      continue;
    }
    if (tool_num >= bufferslen)
    {
      *input_buffer_too_small = true;
      return tool_num;
    }
    if (!toolposfull)
    {
      toolpos[tool_num].tx = transform->tx;
      toolpos[tool_num].ty = transform->ty;
      toolpos[tool_num].tz = transform->tz;
      tool_num++;
    }
  }
  return tool_num;
}

/**
 * GetMarkerPosFromBuffer:
 * @buffer: buffer holding tooldata - a flatbuffer
 * @markerpos: supplied array to store marker positions
 * @bufferslen: size of the markerpos array
 * @input_buffer_too_small: output flag, is set to true if input buffer
 *                          size was too small to store all the data
 *
 * retrieves the marker positions of a tool data buffer
 *
 */
size_t GetMarkerPosFromBuffer (void *buffer, Position *markerpos, size_t bufferslen, bool *input_buffer_too_small)
{
  *input_buffer_too_small = false;
  ns (ToolDataWrapper_table_t) trackdata = ns (ToolDataWrapper_as_root (buffer));
  ns (ToolData_vec_t) tooldataarray = ns (ToolDataWrapper_tooldatas (trackdata));
  size_t inv_len = ns (ToolData_vec_len (tooldataarray));
  size_t marker_num = 0;

  for (size_t i = 0; i < inv_len; i++)
  {
    ns (ToolData_table_t) tdata = ns (ToolData_vec_at (tooldataarray, i));
    ns (Transform_struct_t) transform = ns (ToolData_transform (tdata));
    /* Bit 8 indicates if the transform is missing */
    if (transform->status & 0x0100)
    {
      continue;
    }
    ns (Marker_vec_t) marker_array = ns (ToolData_markers (tdata));
    size_t marker_len = ns (Marker_vec_len (marker_array));
    for (size_t j = 0; j < marker_len; j++)
    {
      if (marker_num >= bufferslen)
      {
        *input_buffer_too_small = true;
        return marker_num;
      }
      ns (Marker_struct_t) marker = ns (Marker_vec_at (marker_array, j));
      if (marker->status == ns (MarkerStatus_OK))
      {
        markerpos[marker_num].tx = ns (Marker_x (marker));
        markerpos[marker_num].ty = ns (Marker_y (marker));
        markerpos[marker_num].tz = ns (Marker_z (marker));
        marker_num++;
      }
    }
  }
  return marker_num;
}

/**
 * GetMarkerDataFromBuffer:
 * @buffer: buffer holding tooldata - a flatbuffer
 * @points: supplied array to store output marker positions
 * @framenumbers: supplied array to store frame numbers
 * @time_specs: supplied array to store timestamps
 * @bufferslen: size of the arrays to return data
 *              (all arrays are expected to be the same size)
 * @input_buffer_too_small: output flag, is set to true if input buffer
 *                          size was too small to store all the data
 *
 * retrieves marker data, framenumbers, and timestamps from the a tool data buffer
 *
 */
size_t GetMarkerDataFromBuffer (void *buffer, Position *points, unsigned int *framenumbers, struct timespec *time_specs, size_t bufferslen, bool *input_buffer_too_small)
{
  *input_buffer_too_small = false;
  ns (ToolDataWrapper_table_t) trackdata = ns (ToolDataWrapper_as_root (buffer));
  ns (ToolData_vec_t) tooldataarray = ns (ToolDataWrapper_tooldatas (trackdata));
  size_t inv_len = ns (ToolData_vec_len (tooldataarray));
  size_t marker_num = 0;
  for (size_t i = 0; i < inv_len; i++)
  {
    if (marker_num >= bufferslen)
    {
      *input_buffer_too_small = true;
      return marker_num;
    }
    ns (ToolData_table_t) tdata = ns (ToolData_vec_at (tooldataarray, i));
    ns (Transform_struct_t) transform = ns (ToolData_transform (tdata));
    unsigned int frame_number = ns (ToolData_framenumber (tdata));
    unsigned int time_s = ns (ToolData_timespec_s (tdata));
    unsigned int time_ns = ns (ToolData_timespec_ns (tdata));

    /* Bit 8 indicates if the transform is missing */
    if (transform->status & 0x0100)
    {
      continue;
    }
    ns (Marker_vec_t) marker_array = ns (ToolData_markers (tdata));
    size_t marker_len = ns (Marker_vec_len (marker_array));
    for (size_t j = 0; j < marker_len; j++)
    {
      ns (Marker_struct_t) marker = ns (Marker_vec_at (marker_array, j));
      if (marker->status == ns (MarkerStatus_OK))
      {
        points[marker_num].tx = ns (Marker_x (marker));
        points[marker_num].ty = ns (Marker_y (marker));
        points[marker_num].tz = ns (Marker_z (marker));
        framenumbers[marker_num] = frame_number;
        time_specs[marker_num].tv_sec = time_s;
        time_specs[marker_num].tv_nsec = time_ns;
        marker_num++;
      }
    }
  }
  return marker_num;
}

