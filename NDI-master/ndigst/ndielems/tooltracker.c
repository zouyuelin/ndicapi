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
 * SECTION:gsttooltracker
 * @title: GstToolTracker
 * @short_description: communicates with the NDI Tracking Device
 * @see_also: trackoverlay, trackbin
 *
 * GstToolTracker provides communication with the NDI Tracking Device, 
 * it is used by the tracksrc and trackbin elements.  If the trackbin
 * element is used, then it creates the GstToolTracker and passes it 
 * to the tracksrc element as a parameter.
 *
 */

#include "tooltracker.h"

#define DEFAULT_CONNECT_TO		NULL
#define DEFAULT_RTSP_VIDEO_PORT NULL
#define DEFAULT_TOOL_LOCATION	NULL
#define DEFAULT_TOOL_FILE		NULL


#define gst_tool_tracker_parent_class parent_class
G_DEFINE_TYPE (GstToolTracker, gst_tool_tracker, G_TYPE_OBJECT);

static void gst_tool_tracker_init (GstToolTracker * self)
{
  self->connect_to = g_strdup (DEFAULT_CONNECT_TO);
  self->rtsp_video_port = g_strdup (DEFAULT_RTSP_VIDEO_PORT);
  self->tool_location = g_strdup (DEFAULT_TOOL_LOCATION);
  self->tool_file = g_strdup (DEFAULT_TOOL_FILE);
}

void set_tool_tracker_connect_to (GstToolTracker * self, gchar *connect_to)
{
  if (self->connect_to) {
    g_free (self->connect_to);
  }
  self->connect_to = connect_to;
}
void set_tool_tracker_port (GstToolTracker * self, gchar *rtsp_video_port)
{
  if (self->rtsp_video_port) {
    g_free (self->rtsp_video_port);
  }
  self->rtsp_video_port = rtsp_video_port;
}
void set_tool_tracker_tool_dir (GstToolTracker * self, gchar *tool_location)
{
  if (self->tool_location) {
    g_free (self->tool_location);
  }
  self->tool_location = tool_location;
}
void set_tool_tracker_tool_file (GstToolTracker *self, gchar *tool_file)
{
  if (self->tool_file) {
    g_free (self->tool_file);
  }
  self->tool_file = tool_file;
}

gchar * get_tool_tracker_connect_to (GstToolTracker *self)
{
  return self->connect_to;
}
gchar * get_tool_tracker_port (GstToolTracker *self)
{
  return self->rtsp_video_port;
}
gchar * get_tool_tracker_tool_dir (GstToolTracker *self)
{
  return self->tool_location;
}
gchar * get_tool_tracker_tool_file (GstToolTracker *self)
{
  return self->tool_file;
}

gboolean tool_tracking_connect (GstToolTracker *self)
{
  self->tool_tracking = CreateToolTracker (self->connect_to, self->rtsp_video_port, self->tool_location, self->tool_file);
  return tool_tracking_is_connected (self);
}

gboolean tool_tracking_is_connected (GstToolTracker *self)
{
  return self->tool_tracking != NULL;
}

void tool_tracking_start_tracking (GstToolTracker *self)
{
  StartTracking (self->tool_tracking);
  self->is_tracking = TRUE;
}

void tool_tracking_stop_tracking (GstToolTracker *self)
{
  StopTracking (self->tool_tracking);
}

void get_video_source (GstToolTracker *self, char *video_address_buffer, size_t buffer_len)
{
  GetVideoSource (self->tool_tracking, video_address_buffer, buffer_len);
}

void get_lens_params (GstToolTracker *self, LensParams *lensParams)
{
  GetLensParams (self->tool_tracking, lensParams);
}

gint get_track_frequency (GstToolTracker *self)
{
  return GetTrackFrequency (self->tool_tracking);
}

void *get_tool_data (GstToolTracker *self, size_t *size_out)
{
  if (!self->tool_tracking)
  {
    return NULL;
  }
  return GetToolData (self->tool_tracking, size_out);
}

static void
tool_tracker_dispose (GObject * obj)
{
  GstToolTracker *self = GST_TOOL_TRACKER (obj);

  tool_tracking_stop_tracking (self);
  DeleteToolTracker (self->tool_tracking);
  self->tool_tracking = NULL;

  g_free (self->connect_to);
  g_free (self->tool_location);
  g_free (self->rtsp_video_port);
  g_free (self->tool_file);

  G_OBJECT_CLASS (gst_tool_tracker_parent_class)->dispose (obj);
}

GstToolTracker *
tool_tracker_new (void)
{
  return g_object_new (GST_TYPE_TOOL_TRACKER, NULL);
}

static void
gst_tool_tracker_class_init (GstToolTrackerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = tool_tracker_dispose;
}
