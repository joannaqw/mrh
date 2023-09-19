/* -*- c++ -*- */

#ifndef DEVICE_H
#define DEVICE_H

#include <chrono>
#include <math.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

#include "pm.h"
#include "dev_array.h"

using namespace PM_NS;

#define _SIZE_GRID 32
#define _SIZE_BLOCK 256

#define OUTPUTIJ        1
#define INPUT_IJ        2

// pyscf/pyscf/lib/np_helper/np_helper.h
#define BLOCK_DIM    104

#define HERMITIAN    1
#define ANTIHERMI    2
#define SYMMETRIC    3

#define TRIU_LOOP(I, J) \
        for (j0 = 0; j0 < n; j0+=BLOCK_DIM) \
                for (I = 0, j1 = MIN(j0+BLOCK_DIM, n); I < j1; I++) \
                        for (J = MAX(I,j0); J < j1; J++)

extern "C" {
  void dsymm_(const char*, const char*, const int*, const int*,
	      const double*, const double*, const int*,
	      const double*, const int*,
	      const double*, double*, const int*);
  
  void dgemm_(const char * transa, const char * transb, const int * m, const int * n,
	      const int * k, const double * alpha, const double * a, const int * lda,
	      const double * b, const int * ldb, const double * beta, double * c,
	      const int * ldc);
}

class Device {
  
public :
  
  Device();
  ~Device();
  
  int get_num_devices();
  void get_dev_properties(int);
  void set_device(int);

  void init_get_jk(py::array_t<double>, py::array_t<double>, int, int, int, int);
  void set_mode_get_jk(int);
  void get_jk(int,
	      py::array_t<double>, py::array_t<double>, py::list &,
	      py::array_t<double>, py::array_t<double>,
	      int);
  void pull_get_jk(py::array_t<double>, py::array_t<double>);
  
  void orbital_response(py::array_t<double>,
			py::array_t<double>, py::array_t<double>, py::array_t<double>,
			py::array_t<double>, py::array_t<double>, py::array_t<double>,
			int, int, int);
  
private:

  class PM * pm;
  
  double host_compute(double *);
  int n;
  int size_data;

  size_t grid_size, block_size;
  
  // get_jk

  int size_rho;
  int size_vj;
  int size_vk;
  int size_buf;
  int size_fdrv;

  int blksize;
  int nao;
  int nset;

  int mode_getjk;
  
  double * rho;
  double * vj;
  double * _vktmp;

  double * buf_tmp;
  double * buf3;
  double * buf4;
  double * buf_fdrv;

  double * d_buf2;
  double * d_buf3;
  double * d_vkk;
  
  struct my_AO2MOEnvs {
    int natm;
    int nbas;
    int *atm;
    int *bas;
    double *env;
    int nao;
    int klsh_start;
    int klsh_count;
    int bra_start;
    int bra_count;
    int ket_start;
    int ket_count;
    int ncomp;
    int *ao_loc;
    double *mo_coeff;
    //        CINTOpt *cintopt;
    //        CVHFOpt *vhfopt;
  };

  void fdrv(double *, double *, double *,
	    int, int, int *, int *, int, double *);
  
  void ftrans(int,
	      double *, double *, double *,
	      struct my_AO2MOEnvs *);

  int fmmm(double *, double *, double *,
	   struct my_AO2MOEnvs *, int);
  
  void NPdsymm_triu(int, double *, int);
  void NPdunpack_tril(int, double *, double *, int);
    
#ifdef _SIMPLE_TIMER
  int t_array_count;
  double * t_array;

  int t_array_jk_count;
  double * t_array_jk;
#endif

  int num_threads;
  
#if defined(_GPU_CUDA)
  cublasHandle_t handle;
  cudaStream_t stream;
#endif
  
};

#endif
