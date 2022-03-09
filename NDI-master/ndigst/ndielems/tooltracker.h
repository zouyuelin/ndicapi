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

#ifndef __TOOL_TRACKER_H__
#define __TOOL_TRACKER_H__

#include <gst/gst.h>
#include "../capitogst/tooltrackinginterface.h"

G_BEGIN_DECLS

#define GST_TYPE_TOOL_TRACKER \
  (gst_tool_tracker_get_type())
#define GST_TOOL_TRACKER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TOOL_TRACKER,GstToolTracker))
#define GST_TOOL_TRACKER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TOOL_TRACKER,GstToolTrackerClass))
#define GST_IS_TOOL_TRACKER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TOOL_TRACKER))
#define GST_IS_TOOL_TRACKER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TOOL_TRACKER))

#define GST_TYPE_TOOL_TRACKER_ENTRY \
  (gst_tool_tracker_entry_get_type())


typedef struct _GstToolTracker GstToolTracker;
typedef struct _GstToolTrackerClass GstToolTrackerClass;

struct _GstToolTrackerClass {
  GObjectClass parent_class;
};

struct _GstToolTracker {
  GObject parent;

  gchar *connect_to;		/* NDI device name or IP address, for example P9-00311.local */
  gchar *rtsp_video_port;
  gchar *tool_location;     /* directory location of the tool ROM files to load */
  gchar *tool_file;		/* rom file name to load, can have wild cards */

  ToolDataExpHandle tool_tracking;
  gboolean is_tracking;
};

GstToolTracker * tool_tracker_new (void);
GType gst_tool_tracker_get_type (void);

void set_tool_tracker_connect_to (GstToolTracker * self, gchar *connect_to);
void set_tool_tracker_port (GstToolTracker * self, gchar *rtsp_video_port);
void set_tool_tracker_tool_dir (GstToolTracker * self, gchar *tool_location);
void set_tool_tracker_tool_file (GstToolTracker * self, gchar *tool_file);
gchar * get_tool_tracker_connect_to (GstToolTracker * self);
gchar * get_tool_tracker_port (GstToolTracker * self);
gchar * get_tool_tracker_tool_dir (GstToolTracker * self);
gchar * get_tool_tracker_tool_file (GstToolTracker * self);
gboolean tool_tracking_connect (GstToolTracker * self);
gboolean tool_tracking_is_connected (GstToolTracker *self);
void tool_tracking_start_tracking (GstToolTracker * self);
void tool_tracking_stop_tracking (GstToolTracker * self);
void get_video_source (GstToolTracker *self, char *video_address_buffer, size_t buffer_len);
void get_lens_params (GstToolTracker *self, LensParams *lensParams);
gint get_track_frequency (GstToolTracker *self);
void *get_tool_data (GstToolTracker *self, size_t *size_out);

G_END_DECLS

#endif /* __TOOL_TRACKER_H__ */
