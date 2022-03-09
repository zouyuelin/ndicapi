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
 * SECTION:element-trackbin
 * @title: trackbin
 * @see_also: tracksrc, trackoverlay, #GstToolTracker, #GstLensParam
 *
 * The trackbin element assembles all the needed elements to manage 
 * the video, track data, and KLV meta data streams.
 * The result is a source video stream with track data drawn on the video 
 * stream.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -e trackbin connect-to=P9-00311.local rtsp-port=554 tool-location=c:/capisample/sroms/ tool-file=8700339.rom ! queue !  x264enc speed-preset=ultrafast ! queue ! h264parse ! mp4mux ! filesink sync=false location=c:/test1.mp4
 * ]|
 * </refsect2>
 *
 */

#include "config.h"

#include "trackbin.h"
#include "commonutils.h"
#include <gst/video/video-info.h>


static int RTSP_PROTOCOL_FLAG = 0;
static int VIDEO_LATENCY = 200;

GST_DEBUG_CATEGORY_STATIC (track_bin_debug);
#define GST_CAT_DEFAULT track_bin_debug

#define MAX_NUM_MARKERS			64
#define DEFAULT_FREQ			30
#define DEFAULT_CONNECT_TO		NULL
#define DEFAULT_RTSP_VIDEO_PORT NULL
#define DEFAULT_TOOL_LOCATION	NULL
#define DEFAULT_TOOL_FILE		NULL
#define KLV_DATA_SIZE			24
#define KLV_KEY_SIZE			16

#define VIDEO_FORMATS "{ RGBx, RGB, BGR, BGRx, xRGB, xBGR, " \
    "RGBA, BGRA, ARGB, ABGR, I420, YV12, AYUV, YUY2, UYVY, " \
    "v308, v210, v216, Y41B, Y42B, Y444, YVYU, NV12, NV21, UYVP, " \
    "RGB16, BGR16, RGB15, BGR15, UYVP, A420, YUV9, YVU9, " \
    "IYU1, ARGB64, AYUV64, r210, I420_10LE, I420_10BE, " \
    "GRAY8, GRAY16_BE, GRAY16_LE }"

enum
{
  PROP_0,
  PROP_CONNECT_TO,
  PROP_RTSP_PORT,
  PROP_TOOL_LOCATION,
  PROP_TOOL_FILE
};

#define gst_track_bin_parent_class parent_class
G_DEFINE_TYPE (GstTrackBin, gst_track_bin, GST_TYPE_BIN);

static GstStaticPadTemplate gst_track_bin_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (VIDEO_FORMATS))
);

static gboolean track_target_init (GstTrackBin * trackbin);
static void pad_added_handler (GstElement *src, GstPad *new_pad, GstTrackBin *data);
static void gst_track_bin_finalize (GObject * object);
static void gst_track_bin_set_property (GObject * object, guint prop_id,
  const GValue * value, GParamSpec * pspec);
static void gst_track_bin_get_property (GObject * object, guint prop_id,
  GValue * value, GParamSpec * pspec);
static gboolean gst_track_bin_start (GstTrackBin * trackbin);
static GstStateChangeReturn gst_track_bin_change_state (GstElement *
  element, GstStateChange transition);


static void
gst_track_bin_class_init (GstTrackBinClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GstBinClass *gstbin_class = GST_BIN_CLASS (klass);

  gstelement_class->change_state = gst_track_bin_change_state;
  gobject_class->set_property = gst_track_bin_set_property;
  gobject_class->get_property = gst_track_bin_get_property;
  gobject_class->finalize = gst_track_bin_finalize;

  g_object_class_install_property (gobject_class,
    PROP_CONNECT_TO,
    g_param_spec_string ("connect-to", "connect-to",
      "Name of NDI device to connect to", DEFAULT_CONNECT_TO,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
    PROP_RTSP_PORT,
    g_param_spec_string ("rtsp-port", "rtsp-port",
      "RTSP port for the video connection", DEFAULT_RTSP_VIDEO_PORT,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
    PROP_TOOL_LOCATION,
    g_param_spec_string ("tool-location", "tool-location",
      "Directory location of the tool files that will be loaded", DEFAULT_TOOL_LOCATION,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
    PROP_TOOL_FILE,
    g_param_spec_string ("tool-file", "tool-file",
      "Name of a tool rom file to load", DEFAULT_TOOL_FILE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_add_static_pad_template (gstelement_class,
    &gst_track_bin_src_template);

  gst_element_class_set_static_metadata (gstelement_class,
    "NDI data source", "Source",
    "Sends Combined Video and Track Data From Device",
    "NDI");

  GST_DEBUG_CATEGORY_INIT (track_bin_debug, "trackbin", 0,
    "NDI Track Bin");
}

static gboolean track_target_init (GstTrackBin * trackbin)
{
  gboolean target_chain_ok = FALSE;
  trackbin->track_source = gst_element_factory_make ("tracksrc", "NDItrackSource");
  g_object_set (trackbin->track_source, "tool-tracker", trackbin->gst_tool_tracker, NULL);
  trackbin->track_queue = gst_element_factory_make ("queue", "TrackQueue");
  trackbin->track_overlay = gst_element_factory_make ("trackoverlay", "NDITrackOverlay");

  gst_bin_add_many (GST_BIN (trackbin), trackbin->track_queue, trackbin->track_source, trackbin->track_overlay, NULL);
  gst_element_link_many (trackbin->track_source, trackbin->track_queue, trackbin->track_overlay, NULL);
  trackbin->overlay_video_sink = gst_element_get_static_pad (trackbin->track_overlay, "sink");
  trackbin->overlay_video_src = gst_element_get_static_pad (trackbin->track_overlay, "src");

  /* Connect to the pad-added signal */
  g_signal_connect (trackbin->rtsp_source, "pad-added", G_CALLBACK (pad_added_handler), trackbin);

  return target_chain_ok;
}

static void
gst_track_bin_init (GstTrackBin * trackbin)
{
  trackbin->gst_tool_tracker = tool_tracker_new ();

  GstPadTemplate *templ = gst_static_pad_template_get (&gst_track_bin_src_template);
  trackbin->srcpad = gst_ghost_pad_new_no_target_from_template ("src", templ);
  gst_object_unref (templ);
  gst_pad_set_active (trackbin->srcpad, TRUE);
  gst_element_add_pad (GST_ELEMENT_CAST (trackbin), trackbin->srcpad);
}

static gboolean
gst_track_create_elements (GstTrackBin * trackbin)
{
  GError *e = NULL;
  GstPad *pad;

  if (trackbin->elements_created) {
    return TRUE;
  }
  trackbin->rtsp_source = gst_element_factory_make ("rtspsrc", "VegaRTSPSource");
  /* we can't set the "location" until the tool_tracking is created */
  g_object_set (trackbin->rtsp_source, "latency", VIDEO_LATENCY, NULL);
  g_object_set (trackbin->rtsp_source, "buffer-mode", RTSP_PROTOCOL_FLAG, NULL);
  /* g_object_set(trackbin->rtsp_source, "teardown-timeout", 500000000, NULL); // 5 ms - not in the current version (GStreamer 1.14.4) */

  trackbin->video_bin = gst_parse_bin_from_description ("queue name=h264Queue ! rtph264depay ! h264parse ! avdec_h264 name=h264Decode", TRUE, &e);
  gst_element_set_name (trackbin->video_bin, "H264VideoBin");

  track_target_init (trackbin);

  trackbin->klv_bin = gst_parse_bin_from_description ("queue name=RTPKLVQueue ! rtpklvdepay name=RTPKLVDepay", TRUE, &e);
  gst_element_set_name (trackbin->klv_bin, "KLVTimeStampBin");

  /* add the other elements to the pipeline */
  gst_bin_add_many (GST_BIN (trackbin), trackbin->rtsp_source, trackbin->video_bin, trackbin->klv_bin, NULL);

  pad = gst_element_get_static_pad (trackbin->track_overlay, "src");
  if (pad) {
    GST_DEBUG_OBJECT (trackbin, "setting target src pad %" GST_PTR_FORMAT, pad);
    gst_ghost_pad_set_target (GST_GHOST_PAD (trackbin->srcpad), pad);
    gst_object_unref (pad);
    trackbin->elements_created = TRUE;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static gboolean is_rtp_src_pad_encoding_type (GstTrackBin *trackbin, GstPad *pad, const char *encoding_type)
{
  GstCaps *pad_caps = NULL;
  GstStructure *pad_struct = NULL;
  const gchar *pad_type = NULL;
  const gchar *pad_name = NULL;
  const gchar *encoding = NULL;
  gboolean isRtp = FALSE;
  gboolean is_needed_type = FALSE;

  pad_caps = gst_pad_get_current_caps (pad);
  if (!pad_caps || gst_caps_is_any (pad_caps) || gst_caps_is_empty (pad_caps))
  {
    GST_DEBUG_OBJECT (trackbin, "caps is not KLV - failed first check\n");
    goto exit;
  }

  pad_name = gst_object_get_name (GST_OBJECT_CAST (pad));
  if (!g_str_match_string ("recv_rtp_src", pad_name, 0))
  {
    GST_DEBUG_OBJECT (trackbin, "pad caps - name is does not contain recv_rtp_src\n");
    goto exit;
  }

  pad_struct = gst_caps_get_structure (pad_caps, 0);
  pad_type = gst_structure_get_name (pad_struct);

  isRtp = g_str_has_prefix (pad_type, "application/x-rtp");
  if (!isRtp)
  {
    GST_DEBUG_OBJECT (trackbin, "caps is not rtp - failed second check\n");
    goto exit;
  }

  encoding = gst_structure_get_string (pad_struct, "encoding-name");
  if (!g_str_match_string (encoding_type, encoding, 0))
  {
    GST_DEBUG_OBJECT (trackbin, "pad caps - encoding is NOT %s\n", encoding_type);
    goto exit;
  }
  else
  {
    GST_DEBUG_OBJECT (trackbin, "pad caps - encoding IS %s\n", encoding_type);
  }
  is_needed_type = TRUE;

exit:
  /* Unreference the new pad's caps, if we got them */
  if (pad_caps != NULL)
    gst_caps_unref (pad_caps);

  return is_needed_type;
}

static void pad_added_handler (GstElement *src, GstPad *new_pad, GstTrackBin *trackbin)
{
  GstPadLinkReturn ret = GST_PAD_LINK_OK;

  GST_DEBUG_OBJECT (trackbin, "Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

  if (is_rtp_src_pad_encoding_type (trackbin, new_pad, "SMPTE336M"))
  {
    if (trackbin->klv_chain_linked)
    {
      GST_DEBUG_OBJECT (trackbin, "klv already linked. Ignoring.\n");
      goto exit;
    }
    if (gst_element_link_many (trackbin->rtsp_source, trackbin->klv_bin, NULL))
    {
      trackbin->klv_chain_linked = TRUE;
      GST_DEBUG_OBJECT (trackbin, "KLV source linked OK\n");
    }
    else
    {
      GST_DEBUG_OBJECT (trackbin, "Error KLV source NOT linked\n");
    }

    if (gst_element_link_many (trackbin->klv_bin, trackbin->track_source, NULL))
    {
      GST_DEBUG_OBJECT (trackbin, "linked OK\n");
    }
    else
    {
      GST_DEBUG_OBJECT (trackbin, "linked NOT OK\n");
    }
  }
  else if (is_rtp_src_pad_encoding_type (trackbin, new_pad, "H264"))
  {
    if (trackbin->h264_chain_linked)
    {
      GST_DEBUG_OBJECT (trackbin, "h264 already linked. Ignoring.\n");
      goto exit;
    }

    if (gst_element_link_many (trackbin->rtsp_source, trackbin->video_bin, NULL))
    {
      trackbin->h264_chain_linked = TRUE;
      GST_DEBUG_OBJECT (trackbin, "video bin linked to source OK\n");
    }
    else
    {
      GST_DEBUG_OBJECT (trackbin, "Error video bin linked to source OK\n");
    }

    ret = gst_pad_link ((GstPad *) trackbin->video_bin->srcpads->data, trackbin->overlay_video_sink);
    if (GST_PAD_LINK_FAILED (ret)) {
      GST_DEBUG_OBJECT (trackbin, "linked NOT OK\n");
    }
    else {
      GST_DEBUG_OBJECT (trackbin, "linked OK\n");
    }
  }

exit:
  GST_DEBUG_OBJECT (trackbin, "Exit pad_added_handler\n");
}

static void
gst_track_bin_finalize (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* stores the lens parameters in a gst object to allow it's 
 * use in parameters and events
 */
static void assign_gst_lens_param (GstTrackBin *trackbin, LensParams *lensParams)
{
  trackbin->gst_lens_param = lens_param_new ();
  trackbin->gst_lens_param->q0 = lensParams->vcu_align.q0;
  trackbin->gst_lens_param->qx = lensParams->vcu_align.qx;
  trackbin->gst_lens_param->qy = lensParams->vcu_align.qy;
  trackbin->gst_lens_param->qz = lensParams->vcu_align.qz;

  trackbin->gst_lens_param->tx = lensParams->vcu_trans.tx;
  trackbin->gst_lens_param->ty = lensParams->vcu_trans.ty;
  trackbin->gst_lens_param->tz = lensParams->vcu_trans.tz;

  trackbin->gst_lens_param->k1 = lensParams->k1;
  trackbin->gst_lens_param->k2 = lensParams->k2;
  trackbin->gst_lens_param->k3 = lensParams->k3;
  trackbin->gst_lens_param->p1 = lensParams->p1;
  trackbin->gst_lens_param->p2 = lensParams->p2;

  trackbin->gst_lens_param->u0 = lensParams->u0;
  trackbin->gst_lens_param->v0 = lensParams->v0;
  trackbin->gst_lens_param->fu = lensParams->fu;
  trackbin->gst_lens_param->fv = lensParams->fv;

  trackbin->gst_lens_param->binX = lensParams->res_char.binX;
  trackbin->gst_lens_param->binY = lensParams->res_char.binY;
  trackbin->gst_lens_param->left = lensParams->res_char.left;
  trackbin->gst_lens_param->top = lensParams->res_char.top;
}

static void get_vcu_info (GstTrackBin *trackbin)
{
  get_video_source (trackbin->gst_tool_tracker, trackbin->video_address, VID_ADDRESS_BUF_LEN);
  LensParams lensParams;
  get_lens_params (trackbin->gst_tool_tracker, &lensParams);
  assign_gst_lens_param (trackbin, &lensParams);
  g_object_set (trackbin->track_overlay, "transform-params", trackbin->gst_lens_param, NULL);
  trackbin->got_vcu_info = TRUE;
  g_object_set (trackbin->rtsp_source, "location", trackbin->video_address, NULL);
}

static void get_track_info (GstTrackBin *trackbin)
{
  gint freq = get_track_frequency (trackbin->gst_tool_tracker);
  trackbin->queried_track_freq = TRUE;
  GST_DEBUG_OBJECT (trackbin, "get_track_info - track freq = %d\n", freq);
  if (freq > 0)
  {
    g_object_set (trackbin->track_overlay, "track-freq", freq, NULL);
  }
}

static gboolean
gst_track_bin_start (GstTrackBin * trackbin)
{
  gboolean start_ok = FALSE;
  if (trackbin->gst_tool_tracker == NULL)
  {
    GST_ERROR_OBJECT (trackbin, "No device defined to connect to");
  }
  else
  {
    GST_DEBUG_OBJECT (trackbin, "gst_track_bin_start - setting vcu and track info\n");
    start_ok = tool_tracking_connect (trackbin->gst_tool_tracker);
    if (start_ok)
    {
      get_vcu_info (trackbin);
      get_track_info (trackbin);
    }
  }
  return start_ok;
}

static void
gst_track_bin_set_property (GObject * object, guint prop_id,
  const GValue * value, GParamSpec * pspec)
{
  GstTrackBin *trackbin = GST_TRACK_BIN (object);

  switch (prop_id) {
    case PROP_CONNECT_TO:
      set_tool_tracker_connect_to (trackbin->gst_tool_tracker, g_value_dup_string (value));
      break;
    case PROP_TOOL_LOCATION:
      set_tool_tracker_tool_dir (trackbin->gst_tool_tracker, g_value_dup_string (value));
      break;
    case PROP_TOOL_FILE:
      set_tool_tracker_tool_file (trackbin->gst_tool_tracker, g_value_dup_string (value));
      break;
    case PROP_RTSP_PORT:
      set_tool_tracker_port (trackbin->gst_tool_tracker, g_value_dup_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_track_bin_get_property (GObject * object, guint prop_id,
  GValue * value, GParamSpec * pspec)
{
  GstTrackBin *trackbin = GST_TRACK_BIN (object);

  switch (prop_id) {
    case PROP_CONNECT_TO:
      g_value_set_string (value, get_tool_tracker_connect_to (trackbin->gst_tool_tracker));
      break;
    case PROP_TOOL_LOCATION:
      g_value_set_string (value, get_tool_tracker_tool_dir (trackbin->gst_tool_tracker));
      break;
    case PROP_TOOL_FILE:
      g_value_set_string (value, get_tool_tracker_tool_file (trackbin->gst_tool_tracker));
      break;
    case PROP_RTSP_PORT:
      g_value_set_string (value, get_tool_tracker_port (trackbin->gst_tool_tracker));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstStateChangeReturn
gst_track_bin_change_state (GstElement * element, GstStateChange transition)
{
  GstTrackBin *trackbin = GST_TRACK_BIN (element);
  GstStateChangeReturn ret;

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      if (!gst_track_create_elements (trackbin)) {
        return GST_STATE_CHANGE_FAILURE;
      }
      if (!tool_tracking_is_connected (trackbin->gst_tool_tracker)) {
        if (!gst_track_bin_start (trackbin))
        {
          if (trackbin->gst_tool_tracker)
          {
            GST_ELEMENT_ERROR (trackbin, RESOURCE, FAILED, ("failed to connect to NDI device"),
              ("device: %s, tool directory: %s, video port: %s", trackbin->gst_tool_tracker->connect_to, trackbin->gst_tool_tracker->tool_location, trackbin->gst_tool_tracker->rtsp_video_port));
          }
          else
          {
            GST_ELEMENT_ERROR (trackbin, RESOURCE, FAILED, ("failed to connect to NDI device"),
              ("No gst_tool_tracker"));
          }
          return GST_STATE_CHANGE_FAILURE;
        }
      }
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    default:
      break;
  }

  return ret;
}

gboolean
gst_track_bin_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "trackbin",
    GST_RANK_NONE, GST_TYPE_TRACK_BIN);
}

/* call this if not installing this element, and wish to load by an app */
void register_local_plugin_track_bin ()
{
  /* local plugins */
  gst_plugin_register_static (GST_VERSION_MAJOR, GST_VERSION_MINOR,
    "trackbin", "NDI Track Bin",
    gst_track_bin_plugin_init, VERSION, "LGPL", PACKAGE, PACKAGE_NAME, PACKAGE_ORIGIN);
}
