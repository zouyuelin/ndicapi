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

#include <stdlib.h>
#include "nditransform.h"

/**
 * TransformPointByQuat:
 * @pointin: input point
 * @rotation: the quaternion used for the transform
 * @pointout: output transformed point
 *
 * transforms a 3D point by a quaternion
 */
void TransformPointByQuat (const Position *pointin, const Quat *rotation, Position *pointout)
{
  if ((pointin->tx < QUAT_MAX_NEG) || (pointin->ty < QUAT_MAX_NEG) || (pointin->tz < QUAT_MAX_NEG))
  {
    return;
  }
  Position cross;
  cross.tx = rotation->qy * pointin->tz - rotation->qz * pointin->ty;
  cross.ty = rotation->qz * pointin->tx - rotation->qx * pointin->tz;
  cross.tz = rotation->qx * pointin->ty - rotation->qy * pointin->tx;

  pointout->tx = pointin->tx + 2.0 * (rotation->q0 * cross.tx + rotation->qy * cross.tz - rotation->qz * cross.ty);
  pointout->ty = pointin->ty + 2.0 * (rotation->q0 * cross.ty + rotation->qz * cross.tx - rotation->qx * cross.tz);
  pointout->tz = pointin->tz + 2.0 * (rotation->q0 * cross.tz + rotation->qx * cross.ty - rotation->qy * cross.tx);
}

/**
 * TranslatePoint:
 * @translation: the translation amount
 * @pointtotranslate: input/output the point to translate, in place
 *
 * translates a 3D point in place
 */
void TranslatePoint (const Position *translation, Position *pointtotranslate)
{
  pointtotranslate->tx += translation->tx;
  pointtotranslate->ty += translation->ty;
  pointtotranslate->tz += translation->tz;
}

/**
 * CameraParametersMat:
 * @cam: output - the returned 3x3 matrix
 * @u0: Horizontal position of pinhole in sensor space [pixels]
 * @v0: Vertical position of pinhole in sensor space [pixels]
 * @fu: Horizontal focal length in sensor space [pixels]
 * @fv: Vertical focal length in sensor space [pixels]
 *
 * Creates a 3x3 transformation matrix given the pinhole camera parameters
 *
 */
void CameraParametersMat (double cam[MAT_ORDER_3][MAT_ORDER_3], double u0, double v0, double fu, double fv)
{
  cam[2][2] = 1.0;
  cam[0][2] = u0;
  cam[1][2] = v0;
  cam[0][0] = fu;
  cam[1][1] = fv;
  cam[0][1] = cam[1][0] = cam[2][0] = cam[2][1] = 0.;
}

/**
 * GetCameraParametersArray:
 * @cam: input - matrix holding the pinhole camera parameters
 * @params: output - the array holding the pinhole camera parameters
 *
 * places the pinhole camera parameters as a double array
 *
 */
void GetCameraParametersArray (double cam[MAT_ORDER_3][MAT_ORDER_3], double *params)
{
  params[0] = cam[0][2]; /* u0 */
  params[1] = cam[1][2]; /* v0 */
  params[2] = cam[0][0]; /* f0 */
  params[3] = cam[1][1]; /* fv */
}

/**
 * CreateCameraParametersMatFromNative:
 * @nativeCameraParameters: input - original camera transformation matrix
 * @cam: output - the new camera pinhole transformation matrix 
 * @transforms: structure holding the video camera resolution characteristics
 *
 * adjustes the original pinhole camera matrix to account for the video resolution 
 *
 */
void CreateCameraParametersMatFromNative (double nativeCameraParameters[MAT_ORDER_3][MAT_ORDER_3], double cam[MAT_ORDER_3][MAT_ORDER_3], ResolutionCharacteristics *transforms)
{
  double params[NO_OF_ZHANG_PARAMS];

  GetCameraParametersArray (nativeCameraParameters, params);
  /* Scale the camera parameters by the left, top and binning of the sensor */
  double u0 = (params[0] - transforms->left) / transforms->binX;
  double v0 = (params[1] - transforms->top) / transforms->binY;
  double fu = params[2] / transforms->binX;
  double fv = params[3] / transforms->binY;

  /* Create a new camera parameters object */
  CameraParametersMat (cam, u0, v0, fu, fv);
}

/**
 * AlignAndAddPoint:
 * @pointsinput: input array of 3D points to transform
 * @pointsoutput: the transformed array of points
 * @count: the number of points to transform
 * @ProjectToScreenParams: the object containing the transformation parameters
 *
 * rotates and translates an array of input points.
 *
 */
void AlignAndAddPoint (Position *pointsinput, Position *pointsoutput, int count, ProjectToScreenParams *tparams)
{
  for (int i = 0; i < count; i++)
  {
    /* vcu_align holds the quaternion for rotation */
    TransformPointByQuat (pointsinput, &tparams->ndi_lens_params.vcu_align, pointsoutput);
    /* vcu_trans holds the point used for translation */
    TranslatePoint (&tparams->ndi_lens_params.vcu_trans, pointsoutput);
    if (i < count - 1)
    {
      pointsinput++;
      pointsoutput++;
    }
  }
}

#define DIVIDE_TOLER 0.000000001
/**
 * projectPoints:
 * @pointsinput: input array of 3D points to transform
 * @pointsoutput: the transformed array of points
 * @count: the number of points to transform
 * @ProjectToScreenParams: the object containing the transformation parameters
 *
 * transforms the input 3D points into video space. The pinhole camera matrix is 
 * used as well as the distortion parameters.
 * See https://en.wikipedia.org/wiki/Camera_resectioning#Zhang and
 * https://en.wikipedia.org/wiki/Pinhole_camera_model
 *
 */
void projectPoints (Position *points_in, Position *points_out, int points_count, Position tvec, double cam[MAT_ORDER_3][MAT_ORDER_3], double distort[NO_OF_ZHANG_PARAMS])
{
  /*
   * We are using only 5 parameters for distortion, otherwise there would be an additional idist2 to calculate
   */
  double r2, r4, r6, a1, a2, a3, cdist;
  double xd, yd;

  for (int i = 0; i < points_count; i++)
  {
    Position Xi = points_in[i];
    Position Y;
    Y.tx = Xi.tx + tvec.tx;
    Y.ty = Xi.ty + tvec.ty;
    Y.tz = Xi.tz + tvec.tz;

    Y.tz = fabs (Y.tz) > DIVIDE_TOLER ? 1. / Y.tz : 1.;
    Y.tx = Y.tx * Y.tz;
    Y.ty = Y.ty * Y.tz;

    r2 = Y.tx * Y.tx + Y.ty * Y.ty;
    r4 = r2 * r2;
    r6 = r4 * r2;
    a1 = 2 * Y.tx * Y.ty;
    a2 = r2 + 2 * Y.tx * Y.tx;
    a3 = r2 + 2 * Y.ty * Y.ty;
    cdist = 1. + distort[0] * r2 + distort[1] * r4 + distort[4] * r6;

    xd = Y.tx * cdist + distort[2] * a1 + distort[3] * a2;
    yd = Y.ty * cdist + distort[2] * a3 + distort[3] * a1;

    points_out[i].tx = cam[0][0] * xd + cam[0][2];
    points_out[i].ty = cam[1][1] * yd + cam[1][2];
    points_out[i].tz = 0.;
  }
}

/**
 * TransformToolAndMarkers:
 * @toolpos: array of tool positions
 * @toolcount: number of tool points
 * @markers: array of markers
 * @markercount: number of markers
 * @tparams: the object containing the transformation parameters
 *
 * transforms the input tool position and marker position 3D points into 
 * video space. The points are transformed in place, the result is 2D 
 * with the z component 0
 *
 */
void TransformToolAndMarkers (Position *toolpos, int toolcount, Position *markers, int markercount, ProjectToScreenParams *tparams)
{
  Position trans = { 0, 0, 0 };
  int max_pnt_size = markercount > toolcount ? markercount : toolcount;

  Position *pnts_out = (Position *) malloc (max_pnt_size * sizeof (Position));
  AlignAndAddPoint (toolpos, pnts_out, toolcount, tparams);
  /* overwrite the originals */
  projectPoints (pnts_out, toolpos, toolcount, trans, tparams->cam_param, tparams->distort);

  AlignAndAddPoint (markers, pnts_out, markercount, tparams);
  /* overwrite the originals */
  projectPoints (pnts_out, markers, markercount, trans, tparams->cam_param, tparams->distort);

  free (pnts_out);
}
