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

#ifndef __LENS_PARAM_H__
#define __LENS_PARAM_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_LENS_PARAM \
  (gst_lens_param_get_type())
#define GST_LENS_PARAM(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_LENS_PARAM,GstLensParam))
#define GST_LENS_PARAM_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_LENS_PARAM,GstLensParamClass))
#define GST_IS_LENS_PARAM(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_LENS_PARAM))
#define GST_IS_LENS_PARAM_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_LENS_PARAM))

#define GST_TYPE_LENS_PARAM_ENTRY \
  (gst_lens_param_entry_get_type())


typedef struct _GstLensParam GstLensParam;
typedef struct _GstLensParamClass GstLensParamClass;

struct _GstLensParamClass {
  GObjectClass parent_class;
};

struct _GstLensParam {
  GObject parent;

/** 
 * Quaternion rotation from Camera Coordinate System to 
 * Video Coordinate System 
 */
  gdouble q0;
  gdouble qx;
  gdouble qy;
  gdouble qz;

/** 
 * Translation from Camera Coordinate System to Video Coordinate System [mm]
 */
  gdouble tx;
  gdouble ty;
  gdouble tz;

  /** Lens distortion model, the Zhang distortion coefficients*/
  gdouble k1;
  gdouble k2;
  gdouble k3;
  gdouble p1;
  gdouble p2;

  /** Camera parameters */
  gdouble u0; /* Horizontal position of pinhole in sensor space[pixels] */
  gdouble v0; /* Vertical position of pinhole in sensor space[pixels] */
  gdouble fu; /* Horizontal focal length in sensor space[pixels] */
  gdouble fv; /* Vertical focal length in sensor space[pixels] */

  /** Image area parameters */
  gint32 binX; /* Binning scale factor in horizontal direction. Changes with selected resolution. */
  gint32 binY; /* Binning scale factor in vertical direction.Changes with selected resolution. */
  gint32 left; /* Left coordinate of imaging window on the sensor [pixels]. Changes with selected resolution. */
  gint32 top; /* Top coordinate of imaging window on the sensor[pixels].Changes with selected resolution. */
};

GstLensParam    * lens_param_new (void);

#ifdef __cplusplus
extern "C" {
#endif

  GType gst_lens_param_get_type (void);

#ifdef __cplusplus
}
#endif

G_END_DECLS

#endif /* __LENS_PARAM_H__ */
