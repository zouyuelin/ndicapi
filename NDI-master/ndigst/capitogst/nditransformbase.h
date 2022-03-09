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

 /*
  * Basic NDI transformation types.
  */

#ifndef __NDI_TRANSFORM_BASE_H__
#define __NDI_TRANSFORM_BASE_H__

#include <math.h>

static double QUAT_MAX_NEG = -3.0E28;

typedef struct _Position Position;
typedef struct _Quat Quat;
typedef struct _LensParams LensParams;
typedef struct _ResolutionCharacteristics ResolutionCharacteristics;

struct _Position
{
  double    tx;
  double    ty;
  double    tz;
};

struct _Quat
{
  double q0;
  double qx;
  double qy;
  double qz;
};

/**
 * Data structure to hold video camera resolution characteristics
 */
struct _ResolutionCharacteristics
{
  int binX; /* Binning scale factor in horizontal direction. Changes with selected resolution. */
  int binY; /* Binning scale factor in vertical direction.Changes with selected resolution. */
  int left; /* Left coordinate of imaging window on the sensor [pixels]. Changes with selected resolution. */
  int top; /* Top coordinate of imaging window on the sensor[pixels].Changes with selected resolution. */
};

/**
 * Represents the lens parameters - distortion, pinhole, alignment, and image area binning and location.
 */
struct _LensParams
{
  /** Tracking to video camera transform - rotation */
  Quat vcu_align;

  /** Tracking to video camera transform - translation */
  Position vcu_trans;

  /** Lens distortion model, the Zhang distortion coefficients */
  double k1; 
  double k2;
  double k3;
  double p1;
  double p2;

  double u0; /* Horizontal position of pinhole in sensor space[pixels] */
  double v0; /* Vertical position of pinhole in sensor space[pixels] */
  double fu; /* Horizontal focal length in sensor space[pixels] */
  double fv; /* Vertical focal length in sensor space[pixels] */

  /** Image area parameters */
  ResolutionCharacteristics res_char;
};

#endif  // __NDI_TRANSFORM_BASE_H__