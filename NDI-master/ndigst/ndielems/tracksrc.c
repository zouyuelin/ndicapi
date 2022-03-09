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
 * SECTION:element-tracksrc
 * @title: tracksrc
 * @see_also: trackoverlay, trackbin, #GstToolTracker
 *
 * The tracksrc element reads tracking data from an NDI Device.
 *
 * The device parameters are set with the properties #connect-to, 
 * #tool-location, #tool-file
 *
 * The #tool-tracker parameter is sent by the trackbin management 
 * element. The trackbin element uses the GstToolTracker object to 
 * communicated with the NDI Device, and in turn passes that communication 
 * object to this element.
 *
 * When used in conjunction with the trackoverlay element, the sink pad 
 * klvsinkpad is used. This pad receives precision timestamps encoded as 
 * KLV meta data (SMPTE336M). Each KLV timestamp sample has a 
 * corresponding PTS. This PTS/timestamp pair gives the information needed 
 * to synchronise tracking data with the video data samples from the NDI
 * VCU. The timestamp stored in the tracking data is compared to the timestamp
 * stored in the KLV data, then the PTS of the tracking data is adjusted
 * accordingly. Further down the pipeline in a trackoverlay element, the
 * synchronized tracking data in matched to a video sample in order to 
 * draw overlays on the video sample.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 tracksrc connect-to=P9-00311.local rtsp-port=554 tool-location=c:/capisample/sroms/ tool-file=8700339.rom ! fakesink
 * ]|
 * simple pipeline showing properties
 * </refsect2>
 * 
 */

#include "config.h"

#include "tracksrc.h"
#include "commonutils.h"


GST_DEBUG_CATEGORY_STATIC (track_src_debug);
#define GST_CAT_DEFAULT track_src_debug

#define MAX_NUM_MARKERS			64
#define DEFAULT_FREQ			30
#define DEFAULT_CONNECT_TO		NULL
#define DEFAULT_RTSP_VIDEO_PORT NULL
#define DEFAULT_TOOL_LOCATION	NULL
#define DEFAULT_TOOL_FILE		NULL
#define KLV_DATA_SIZE			24
#define KLV_KEY_SIZE			16

enum {
  PROP_0,
  PROP_CONNECT_TO,
  PROP_RTSP_PORT,
  PROP_TOOL_LOCATION,
  PROP_TOOL_FILE,
  PROP_TOOL_TRACKER
};

#define gst_track_src_parent_class parent_class
G_DEFINE_TYPE (GstTrackSrc, gst_track_src, GST_TYPE_PUSH_SRC);

static GstStaticPadTemplate sink_klv_factory = GST_STATIC_PAD_TEMPLATE ("klvsink",
  GST_PAD_SINK,
  GST_PAD_SOMETIMES,
  GST_STATIC_CAPS ("meta/x-klv, parsed = (bool) true"));

static GstStaticPadTemplate gst_track_src_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("trackdata")
);
static gboolean gst_ndi_klv_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_ndi_src_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);
static void gst_track_src_finalize (GObject * object);
static void gst_track_src_set_property (GObject * object, guint prop_id,
  const GValue * value, GParamSpec * pspec);
static void gst_track_src_get_property (GObject * object, guint prop_id,
  GValue * value, GParamSpec * pspec);
static gboolean gst_track_src_start (GstBaseSrc * basesrc);
static gboolean gst_track_src_stop (GstBaseSrc * basesrc);
static GstFlowReturn gst_track_src_create (GstPushSrc * basesrc,
  GstBuffer ** buffer);
static gboolean gst_track_src_query (GstBaseSrc * basesrc, GstQuery * query);
static unsigned char KLV_TIMESTAMP_KEY[KLV_KEY_SIZE] = { 0x06, 0x0E, 0x2B, 0x34, 0x01, 0x01, 0x01, 0x03, 0x07, 0x02, 0x01, 0x01, 0x01, 0x05, 0x00, 0x00 };
static guint64 getKLVPrecisionTimeStamp (GstTrackSrc * src, GstBuffer *buffer);

static gboolean get_track_time_data (void *track_data, GstTrackSrc *elem, GstClockTime *clock_epoch_out)
{
  bool buffer_too_small;
  unsigned int frame_numbers[MAX_NUM_MARKERS] = { 0 };
  struct timespec time_specs[MAX_NUM_MARKERS] = { 0 };
  GstClockTime clock_time = GST_CLOCK_TIME_NONE;
  size_t marker_index;
  static GstClockTime last_marker_epoch = GST_CLOCK_TIME_NONE;

  *clock_epoch_out = 0;
  size_t numtools = GetTimeInfoFromBuffer (track_data, frame_numbers, time_specs, MAX_NUM_MARKERS, &buffer_too_small);
  GST_DEBUG_OBJECT (elem, "num tools = %" G_GSIZE_FORMAT ", buffer_too_small = %d", numtools, buffer_too_small);
  if (numtools <= 0)
  {
    return FALSE;
  }

  /* just use the last time stamp, they are all the same*/
  marker_index = numtools - 1;
  clock_time = GST_TIMESPEC_TO_TIME (time_specs[marker_index]);
  gchar *track_epoch = get_epoch_timestamp_from_clocktime (clock_time);
  GST_DEBUG_OBJECT (elem, "tooldata frame number = %d, Unix Epoch %s", frame_numbers[marker_index], track_epoch);
  g_free (track_epoch);
  if (last_marker_epoch != GST_CLOCK_TIME_NONE)
  {
    GST_DEBUG_OBJECT (elem, "Track Data time difference last sample: %" G_GUINT64_FORMAT "ms", GST_TIME_AS_MSECONDS (clock_time) - GST_TIME_AS_MSECONDS (last_marker_epoch));
  }
  *clock_epoch_out = last_marker_epoch = clock_time;

  return TRUE;
}

static void
gst_track_src_class_init (GstTrackSrcClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSrcClass *gstbasesrc_class;
  GstPushSrcClass *gstpushsrc_class;
  GstPushSrcClass *gstpush_src_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasesrc_class = (GstBaseSrcClass *) klass;
  gstpushsrc_class = (GstPushSrcClass *) klass;
  gstpush_src_class = GST_PUSH_SRC_CLASS (klass);

  gobject_class->set_property = gst_track_src_set_property;
  gobject_class->get_property = gst_track_src_get_property;
  gobject_class->finalize = gst_track_src_finalize;

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

  g_object_class_install_property (gobject_class,
    PROP_TOOL_TRACKER,
    g_param_spec_object ("tool-tracker", "Tool Tracker", "Tool Tracker Communicates with the NDI Device",
      G_TYPE_OBJECT, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  gst_element_class_add_static_pad_template (gstelement_class,
    &sink_klv_factory);
  gst_element_class_add_static_pad_template (gstelement_class,
    &gst_track_src_src_template);

  gst_element_class_set_static_metadata (gstelement_class,
    "NDI track data source", "Source",
    "Sends Track Data From Device",
    "NDI");

  gstbasesrc_class->start = GST_DEBUG_FUNCPTR (gst_track_src_start);
  gstbasesrc_class->stop = GST_DEBUG_FUNCPTR (gst_track_src_stop);
  gstpush_src_class->create = GST_DEBUG_FUNCPTR (gst_track_src_create);
  gstbasesrc_class->query = GST_DEBUG_FUNCPTR (gst_track_src_query);

  GST_DEBUG_CATEGORY_INIT (track_src_debug, "tracksrc", 0,
    "NDI Target Source");
}

static void
gst_track_src_init (GstTrackSrc * src)
{
  src->freq = DEFAULT_FREQ;
  src->buffer_duration = GST_MSECOND / (guint64) DEFAULT_FREQ;
  src->gst_tool_tracker = tool_tracker_new ();

  gst_base_src_set_live (GST_BASE_SRC (src), true);
  gst_base_src_set_format (GST_BASE_SRC (src), GST_FORMAT_TIME);
  gst_base_src_set_do_timestamp (GST_BASE_SRC (src), FALSE);

  src->klvsinkpad = gst_pad_new_from_static_template (&sink_klv_factory, "klvsink");
  gst_pad_set_event_function (src->klvsinkpad,
    GST_DEBUG_FUNCPTR (gst_ndi_klv_sink_event));
  gst_pad_set_chain_function (src->klvsinkpad,
    GST_DEBUG_FUNCPTR (gst_ndi_src_chain));
  gst_element_add_pad (GST_ELEMENT (src), src->klvsinkpad);
}

/* this function handles sink events */
static gboolean
gst_ndi_klv_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
  gboolean ret;
  GstTrackSrc *src = GST_TRACK_SRC (parent);

  GST_LOG_OBJECT (src, "Received %s event: %" GST_PTR_FORMAT,
    GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps * caps;

      gst_event_parse_caps (event, &caps);
      /* not doing anyting with the caps */
      /* forward */
      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}

/*
 * chain function for the klv data
 */
static GstFlowReturn
gst_ndi_src_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  GstTrackSrc *src = GST_TRACK_SRC (parent);
  if (buf)
  {
    guint64 klvtime = getKLVPrecisionTimeStamp (src, buf);

    /* we need to set the timestamp based on the klv time */
    gst_base_src_set_do_timestamp (GST_BASE_SRC (src), FALSE);
    GstClockTime klvPTS = buf->pts;
    GST_DEBUG_OBJECT (src, "COLLECT PTS-klv %" GST_TIME_FORMAT, GST_TIME_ARGS (klvPTS));
    /* setting this every sample as the difference between PTS and epoch is not constant */
    {
      GST_DEBUG_OBJECT (src, "SETTING OFFSET TIME PTS-klv %" GST_TIME_FORMAT, GST_TIME_ARGS (klvPTS));
      GST_DEBUG_OBJECT (src, "Setting BASE, klv epoch %" G_GUINT64_FORMAT ", PTS-klv %" G_GUINT64_FORMAT ", %" GST_TIME_FORMAT,
        klvtime, klvPTS, GST_TIME_ARGS (klvPTS));
      GST_OBJECT_LOCK (src);
      src->last_klv_PTS = klvPTS;
      src->last_klv_e_prec_TS = klvtime;
      src->got_klv_time = true;
      /* done with the klv data */
      gst_buffer_unref (buf);
      GST_OBJECT_UNLOCK (src);
    }
  }
  return GST_FLOW_OK;
}

/**
 * retrieves the precision time stamp from the KLV data package
 */
static guint64 getKLVPrecisionTimeStamp (GstTrackSrc * src, GstBuffer *buffer)
{
  static guint64 lastMicroSecs = 0;
  GstMapInfo info;
  gst_buffer_map (buffer, &info, GST_MAP_READ);
  if (info.size < KLV_DATA_SIZE)
  {
    GST_WARNING_OBJECT (src, "bad data length, size = %" G_GSIZE_FORMAT "\n", info.size);
    gst_buffer_unmap (buffer, &info);
    return lastMicroSecs;
  }
  guint8 *dataBytes = info.data;
  guint64 ePrecStamp = 0;
  guint64 ePrecStampBIGEND = 0;
  gboolean isTimestampKey = TRUE;
  for (int i = 0; i < KLV_KEY_SIZE; i++)
  {
    if (dataBytes[i] != KLV_TIMESTAMP_KEY[i])
    {
      isTimestampKey = FALSE;
      break;
    }
  }
  guint8 size = dataBytes[KLV_KEY_SIZE]; // key in bytes 0 to KLV_KEY_SIZE-1, data length at byte KLV_KEY_SIZE
  /* Decode the time stamp */
  if (isTimestampKey && size > 0)
  {
    ePrecStampBIGEND = *(guint64 *) &dataBytes[KLV_KEY_SIZE+1];
    ePrecStamp = GST_READ_UINT64_BE (&ePrecStampBIGEND);
    guint64 microSecs = GST_TIME_AS_USECONDS (ePrecStamp);
    GstClockTime clockTime = (GstClockTime) ePrecStamp;
    gchar *klvTimeStr = get_epoch_timestamp_from_clocktime (clockTime);
    GST_DEBUG_OBJECT (src, "klv ePrecStamp Unix Epoch %s, diff-last-time = %" G_GUINT64_FORMAT, klvTimeStr, microSecs - lastMicroSecs);
    g_free (klvTimeStr);
    lastMicroSecs = microSecs;
  }
  else
  {
    GST_DEBUG_OBJECT (src, "klv buffer does not have a precision time stamp");
  }

  gst_buffer_unmap (buffer, &info);
  return ePrecStamp;
}

static void
gst_track_src_finalize (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gst_track_src_start (GstBaseSrc * basesrc)
{
  gboolean start_ok = FALSE;
  GstTrackSrc *src = GST_TRACK_SRC (basesrc);

  GST_OBJECT_LOCK (src);

  if (src->gst_tool_tracker == NULL)
  {
    GST_ERROR_OBJECT (src, "No device defined to connect to");
  }
  else
  {
    if (!tool_tracking_is_connected (src->gst_tool_tracker))
    {
      start_ok = tool_tracking_connect (src->gst_tool_tracker);
    }
    else
    {
      start_ok = true;
    }
    if (start_ok && !src->gst_tool_tracker->is_tracking)
    {
      tool_tracking_start_tracking (src->gst_tool_tracker);
    }
    else
    {
      GST_OBJECT_UNLOCK (src);
      if (src->gst_tool_tracker)
      {
        GST_ELEMENT_ERROR (src, RESOURCE, FAILED, ("failed to connect to NDI device"),
          ("device: %s, tool directory: %s, video port: %s", src->gst_tool_tracker->connect_to, src->gst_tool_tracker->tool_location, src->gst_tool_tracker->rtsp_video_port));
      }
      else
      {
        GST_ELEMENT_ERROR (src, RESOURCE, FAILED, ("failed to connect to NDI device"),
          ("No gst_tool_tracker"));
      }
      return FALSE;
    }
  }
  GST_OBJECT_UNLOCK (src);
  return start_ok;
}

static gboolean
gst_track_src_stop (GstBaseSrc * basesrc)
{
  return TRUE;
}

static void assign_gst_lens_param (GstTrackSrc *src, LensParams *lensParams)
{
  src->gst_lens_param = lens_param_new ();
  src->gst_lens_param->q0 = lensParams->vcu_align.q0;
  src->gst_lens_param->qx = lensParams->vcu_align.qx;
  src->gst_lens_param->qy = lensParams->vcu_align.qy;
  src->gst_lens_param->qz = lensParams->vcu_align.qz;

  src->gst_lens_param->tx = lensParams->vcu_trans.tx;
  src->gst_lens_param->ty = lensParams->vcu_trans.ty;
  src->gst_lens_param->tz = lensParams->vcu_trans.tz;

  src->gst_lens_param->k1 = lensParams->k1;
  src->gst_lens_param->k2 = lensParams->k2;
  src->gst_lens_param->k3 = lensParams->k3;
  src->gst_lens_param->p1 = lensParams->p1;
  src->gst_lens_param->p2 = lensParams->p2;

  src->gst_lens_param->u0 = lensParams->u0;
  src->gst_lens_param->v0 = lensParams->v0;
  src->gst_lens_param->fu = lensParams->fu;
  src->gst_lens_param->fv = lensParams->fv;

  src->gst_lens_param->binX = lensParams->res_char.binX;
  src->gst_lens_param->binY = lensParams->res_char.binY;
  src->gst_lens_param->left = lensParams->res_char.left;
  src->gst_lens_param->top = lensParams->res_char.top;
}

static void get_vcu_info (GstPushSrc *basesrc, GstTrackSrc *src)
{
  LensParams lensParams;
  get_lens_params (src->gst_tool_tracker, &lensParams);
  assign_gst_lens_param (src, &lensParams);
  src->got_vcu_info = TRUE;

  GstEvent *lens_params_set = gst_event_new_custom (GST_EVENT_CUSTOM_DOWNSTREAM,
    gst_structure_new ("GstNDILensParamsSet",
      "LensParams", G_TYPE_OBJECT, src->gst_lens_param, NULL));
  gst_pad_push_event (GST_BASE_SRC_PAD (basesrc), lens_params_set);
}

static void get_track_info (GstPushSrc *basesrc, GstTrackSrc *src)
{
  src->freq = get_track_frequency (src->gst_tool_tracker);
  src->queried_track_freq = TRUE;
  if (src->freq > 0)
  {
    src->buffer_duration = GST_MSECOND / src->freq;
  }
  GstEvent *lens_params_set = gst_event_new_custom (GST_EVENT_CUSTOM_DOWNSTREAM,
    gst_structure_new ("GstNDITrackFreqSet",
      "TrackFrequency", G_TYPE_INT, src->freq, NULL));
  gst_pad_push_event (GST_BASE_SRC_PAD (basesrc), lens_params_set);
}

/**
 * create implementation of the GstPushSrcClass
 * the track data buffer is created here.
 */
static GstFlowReturn
gst_track_src_create (GstPushSrc * basesrc, GstBuffer ** buffer)
{
  GstTrackSrc *src;
  void *track_data = 0;
  static GstClockTime last_time;
  GstClockTime trackPTS = 0;
  GstClockTime epochDiff = GST_CLOCK_TIME_NONE;
  size_t size_out = 0;
  gboolean got_time = FALSE;
  GstClockTime last_marker_epoch;
  bool forward_data = TRUE;

  *buffer = NULL;

  src = GST_TRACK_SRC (basesrc);
  if (!src->got_vcu_info)
  {
    get_vcu_info (basesrc, src);
  }
  if (!src->queried_track_freq)
  {
    get_track_info (basesrc, src);
  }

  track_data = get_tool_data (src->gst_tool_tracker, &size_out);

  GST_LOG_OBJECT (src, "tool data size = %" G_GSIZE_FORMAT, size_out);
  if (!track_data)
  {
    GST_WARNING_OBJECT (src, "track data is NULL");
  }
  else
  {
    got_time = get_track_time_data (track_data, src, &last_marker_epoch);
  }

  /* adjust the presentation time stamp of the tracking data based on the klv time reference */
  if (got_time && last_marker_epoch != 0 && src->got_klv_time)
  {
    GST_OBJECT_LOCK (src);
    if (last_marker_epoch > src->last_klv_e_prec_TS)
    {
      trackPTS = last_marker_epoch - src->last_klv_e_prec_TS + src->last_klv_PTS;
    }
    else
    {
      GST_WARNING_OBJECT (src, "Not expected - marker epoch less than last klv epoch");
    }
    GST_OBJECT_UNLOCK (src);
    GST_DEBUG_OBJECT (src, "Adjusted track : PTS %" GST_TIME_FORMAT, GST_TIME_ARGS (trackPTS));
  }
  else
  {
    GST_DEBUG_OBJECT (src, "Adjusted track (no marker data): PTS %" GST_TIME_FORMAT, GST_TIME_ARGS (trackPTS));
    forward_data = FALSE;
  }

  *buffer = gst_buffer_new ();
  GST_BUFFER_DURATION (*buffer) = src->buffer_duration;
  (*buffer)->pts = trackPTS;

  if (track_data && forward_data)
  {
    /* the buffer is freed by calling FreeToolData when 
       the ref count goes to 0
     */
    gst_buffer_append_memory (*buffer,
      gst_memory_new_wrapped (0, track_data, size_out, 0,
        size_out, track_data, (GDestroyNotify) FreeToolData));
  }
  else if (track_data)
  {
    FreeToolData (track_data);
  }
  return GST_FLOW_OK;
}

static gboolean
gst_track_src_query (GstBaseSrc * basesrc, GstQuery * query)
{
  GstTrackSrc *src = GST_TRACK_SRC (basesrc);
  GstClockTime latency = 0;
  gboolean res = FALSE;

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_LATENCY:
    {
      gst_query_set_latency (query,
        gst_base_src_is_live (GST_BASE_SRC_CAST (src)), latency,
        GST_CLOCK_TIME_NONE);
      GST_DEBUG_OBJECT (src, "Reporting latency of %" GST_TIME_FORMAT,
        GST_TIME_ARGS (latency));
      res = TRUE;
      break;
    }
    default:
      res = GST_BASE_SRC_CLASS (parent_class)->query (basesrc, query);
      break;
  }
  return res;
}

static void
gst_track_src_set_property (GObject * object, guint prop_id,
  const GValue * value, GParamSpec * pspec)
{
  GstTrackSrc *src = GST_TRACK_SRC (object);

  switch (prop_id) {

    case PROP_CONNECT_TO:
      set_tool_tracker_connect_to (src->gst_tool_tracker, g_value_dup_string (value));
      break;
    case PROP_TOOL_LOCATION:
      set_tool_tracker_tool_dir (src->gst_tool_tracker, g_value_dup_string (value));
      break;
    case PROP_TOOL_FILE:
      set_tool_tracker_tool_file (src->gst_tool_tracker, g_value_dup_string (value));
      break;
    case PROP_RTSP_PORT:
      set_tool_tracker_port (src->gst_tool_tracker, g_value_dup_string (value));
      break;
    case PROP_TOOL_TRACKER:
      if (src->gst_tool_tracker)
        g_object_unref (src->gst_tool_tracker);
      src->gst_tool_tracker = GST_TOOL_TRACKER (g_value_get_object (value));
      if (src->gst_tool_tracker)
      {
        g_object_ref (src->gst_tool_tracker);
      }
      /* if the tool tracker is being set here, then it does not need to be created by this element */
      src->got_vcu_info = TRUE;
      /* likewise, we will assume the frequency is being managed elsewhere */
      src->queried_track_freq = TRUE;
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_track_src_get_property (GObject * object, guint prop_id,
  GValue * value, GParamSpec * pspec)
{
  GstTrackSrc *src = GST_TRACK_SRC (object);

  switch (prop_id) {

    case PROP_CONNECT_TO:
      g_value_set_string (value, get_tool_tracker_connect_to (src->gst_tool_tracker));
      break;
    case PROP_TOOL_LOCATION:
      g_value_set_string (value, get_tool_tracker_tool_dir (src->gst_tool_tracker));
      break;
    case PROP_TOOL_FILE:
      g_value_set_string (value, get_tool_tracker_tool_file (src->gst_tool_tracker));
      break;
    case PROP_RTSP_PORT:
      g_value_set_string (value, get_tool_tracker_port (src->gst_tool_tracker));
      break;
    case PROP_TOOL_TRACKER:
      g_value_set_object (value, src->gst_tool_tracker);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

gboolean
gst_track_src_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "tracksrc",
    GST_RANK_NONE, GST_TYPE_TRACK_SRC);
}

/* call this if not installing this element, and wish to load by an app */
void register_local_plugin_target_src ()
{
  /* local plugins */
  gst_plugin_register_static (GST_VERSION_MAJOR, GST_VERSION_MINOR,
    "tracksrc", "NDI Target Source",
    gst_track_src_plugin_init, VERSION, "LGPL", PACKAGE, PACKAGE_NAME, PACKAGE_ORIGIN);
}
