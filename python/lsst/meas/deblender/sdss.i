%define meas_deblender_sdss_DOCSTRING
"
Python interface to SDSS photo internals, for debugging purposes
"
%enddef

%feature("autodoc", "1");

%module(package="lsst.meas.deblender", docstring=meas_deblender_sdss_DOCSTRING) sdss

%include <typemaps.i>

%{

#include "phMeasureObj.h"
#include "phPeaks.h"
#include "phObjc.h"
#include "phSpanUtil.h"
#include "region.h"
#include "phObjc_p.h"
#include "phFake.h"
#include "phDgpsf.h"
#include "phExtract.h"

const struct cellgeom* cell_stats_get_cellgeom(const CELL_STATS* cs, int i) {
    return cs->geom + i;
}

const struct pstats*   cell_stats_get_cell(const CELL_STATS* cs, int i) {
    return cs->cells + i;
}

int phFakeGetCellId(double dx, double dy, int ncell, int* p_ri, int* p_si);

CELL_STATS* extractProfile(int* image, int W, int H, double cx, double cy,
                           double maxrad, double sky, double skysig);

void phCellProfSet(CELL_PROF *OUTPUT, const CELL_STATS *cstats,
                   int median, double sky, double gain, double dark_variance,
                   double sigma, double posErr, int use_difference, int sky_noise_only);

CELL_PROF getCellProfile(const CELL_STATS *cstats,
                         int median, double sky, double gain, double dark_variance,
                         double sigma, double posErr, int use_difference, int sky_noise_only);

float pstats_get_median(struct pstats* ps) {
    return ps->qt[1];
}

float cellprof_get_data(CELL_PROF* cp, int i) {
    return cp->data[i];
}
float cellprof_get_sig(CELL_PROF* cp, int i) {
    return cp->sig[i];
}



%}

%ignore phApoApertureEvalNaive;
%ignore phApoApertureEval;
%ignore phApoApertureMomentsEval;

/*
%include "phMeasureObj.h"
%include "phPeaks.h"
%include "phObjc.h"
%include "phSpanUtil.h"
%include "region.h"
%include "phObjc_p.h"
%include "phFake.h"
%include "phDgpsf.h"
*/

%include "phExtract.h"

const struct cellgeom* cell_stats_get_cellgeom(const CELL_STATS* cs, int i);
const struct pstats*   cell_stats_get_cell(const CELL_STATS* cs, int i);
float pstats_get_median(struct pstats* ps);
float cellprof_get_data(CELL_PROF* cp, int i);
float cellprof_get_sig(CELL_PROF* cp, int i);
float pstats_get_median(struct pstats* ps);

CELL_PROF getCellProfile(const CELL_STATS *cstats,
                         int median, double sky, double gain, double dark_variance,
                         double sigma, double posErr, int use_difference, int sky_noise_only);

int phFakeGetCellId(double dx, double dy, int ncell,
                    int* OUTPUT, int* OUTPUT);

// This is pretty ugly -- we malloc() an int array with length matching the python list--
// which must match W*H, but we don't check that here.  Caller beware!
// We *could* re-convert and check objN and objN+1 (defined by swig) corresponding to
// args W and H.

%typemap(in) (int* image) {
  int i, N;
  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }
  N = PySequence_Length($input);
  $1 = (int *)malloc(N * sizeof(int));
  for (i = 0; i < N; i++) {
    PyObject *o = PySequence_GetItem($input,i);
    if (PyNumber_Check(o)) {
      $1[i] = (int)PyInt_AsLong(o);
    } else {
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be ints");
      free($1);
      return NULL;
    }
  }
}
%typemap(freearg) (int* image) {
   if ($1) free($1);
}


CELL_STATS* extractProfile(int* image, int W, int H, double cx, double cy,
                           double maxrad, double sky, double skysig);


