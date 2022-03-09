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
 * SECTION:gstlensparam
 * @title: GstLensParam
 * @short_description: Encapsulates the information required to transform 
 * the tracking data to video space.
 * @see_also: trackoverlay, tracksrc, trackbin, #GstToolTracker
 *
 * The transform information is available from the NDI Tracking Device, 
 * and GstToolTracker provides an API for this information.
 * Either the tracksrc or trackbin elements will create the 
 * GstLensParam object. If created by the trackbin element, it is passed to 
 * the trackoverlay element by the "transform-params" parameter. If the 
 * tracksrc element creates GstLensParam, then is is passed to the 
 * trackoverlay element by a GST_EVENT_CUSTOM_DOWNSTREAM event of type 
 * GstNDILensParamsSet. The trackoverlay element will then transform
 * the 3D tracking data to the 2D video space.
 *
 */

#include "lensparam.h"

#define gst_lens_param_parent_class parent_class
G_DEFINE_TYPE (GstLensParam, gst_lens_param, G_TYPE_OBJECT);

static void
gst_lens_param_init (GstLensParam * self)
{
}

static void
lens_param_dispose (GObject * obj)
{
  GstLensParam *self = GST_LENS_PARAM (obj);
  G_OBJECT_CLASS (gst_lens_param_parent_class)->dispose (obj);
}

GstLensParam *
lens_param_new (void)
{
  return g_object_new (GST_TYPE_LENS_PARAM, NULL);
}

static void
gst_lens_param_class_init (GstLensParamClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = lens_param_dispose;
}
