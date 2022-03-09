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

#ifndef __TRACK_OVERLAY_H__
#define __TRACK_OVERLAY_H__

#ifdef _WIN32
#ifdef NDIELEMS_EXPORTS
#    define NDIELEMS_API __declspec(dllexport)
#else
#    define NDIELEMS_API __declspec(dllimport)
#endif
#else
#define NDIELEMS_API __attribute__ ((visibility ("default")))
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gst/base/gstdataqueue.h>
#include <gst/video/video-info.h>

#include "lensparam.h"
#include "tooldataclient.h"
#include "nditransform.h"

G_BEGIN_DECLS

#define GST_TYPE_TRACK_OVERLAY (gst_track_overlay_get_type())
#define GST_TRACK_OVERLAY(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TRACK_OVERLAY,GstTrackOverlay))
#define GST_TRACK_OVERLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TRACK_OVERLAY,GstTrackOverlayClass))

#define GST_TRACK_OVERLAY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), GST_TYPE_TRACK_OVERLAY, GstTrackOverlayClass))

#define GST_IS_TRACK_OVERLAY(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TRACK_OVERLAY))
#define GST_IS_TRACK_OVERLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TRACK_OVERLAY))
#define GST_TRACK_OVERLAY_CAST(obj)          ((GstTrackOverlay *)(obj))

#define MAX_NUM_MARKERS 64
#define DEFAULT_TRACK_FREQUENCY 30
typedef struct _GstTrackOverlay GstTrackOverlay;
typedef struct _GstTrackOverlayClass GstTrackOverlayClass;

struct _GstTrackOverlay
{
  GstVideoFilterClass parent;
  GstPad	*trackpad;

  GstVideoInfo in_info;
  size_t marker_count;
  gboolean do_draw_markers;
  Position markers[MAX_NUM_MARKERS];
  Position toolpos[MAX_NUM_MARKERS];
  ProjectToScreenParams project_points;
  GstVideoOverlayComposition * comp;
  GstClockTime track_data_PTS;
  GstClockTime video_data_PTS;
  gboolean got_first_buffer;
  GstClockTime first_buffer_PTS;
  GstDataQueue *queue;
  GstLensParam *gst_lens_param;
  guint track_frequency;
  guint match_marker_low_ms;
  guint match_marker_high_ms;
};

struct _GstTrackOverlayClass
{
  GstVideoFilterClass videofilter_class;
};

GType gst_track_overlay_get_type ();
NDIELEMS_API void register_local_plugin_track_overlay ();

G_END_DECLS

#endif /* __TRACK_OVERLAY_H__ */

