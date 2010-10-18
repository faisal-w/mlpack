/** @file dense_matrix.h
 *
 *  @author Dongryeol Lee (dongryel@cc.gatech.edu)
 */

#ifndef CORE_TABLE_DENSE_MATRIX_H
#define CORE_TABLE_DENSE_MATRIX_H

#include "dense_point.h"
#include "memory_mapped_file.h"

namespace core {
namespace table {
class DenseMatrix {

  private:
    double *ptr_;

    int n_rows_;

    int n_cols_;

  public:

    int n_rows() const {
      return n_rows_;
    }

    int n_cols() const {
      return n_cols_;
    }

    void swap_cols(int first_col, int second_col) {
      double *first_ptr = ptr_ + first_col * n_rows_;
      double *second_ptr = ptr_ + second_col * n_rows_;
      for(int i = 0; i < n_rows_; first_ptr++, second_ptr++, i++) {
        std::swap(*first_ptr, *second_ptr);
      }
    }

    void set(int row, int col, double val) {
      ptr_[col * n_rows_ + row] = val;
    }

    double get(int row, int col) const {
      return ptr_[col * n_rows_ + row];
    }

    void Reset() {
      ptr_ = NULL;
      n_rows_ = n_cols_ = 0;
    }

    DenseMatrix() {
      Reset();
    }

    ~DenseMatrix() {
      if(core::table::global_m_file_) {
        core::table::global_m_file_->Deallocate(ptr_);
      }
      else {
        delete[] ptr_;
      }
      Reset();
    }

    void Init(
      int n_rows_in, int n_cols_in) {

      ptr_ = (core::table::global_m_file_) ?
             (double *)core::table::global_m_file_->Allocate(
               n_rows_in * n_cols_in * sizeof(double)) :
             new double[n_rows_in * n_cols_in];
      n_rows_ = n_rows_in;
      n_cols_ = n_cols_in;
    }

    double *GetColumnPtr(int column_id) {
      return ptr_ + column_id * n_rows_;
    }

    void CopyColumnVector(int column_id, double *point_out) const {
      const double *ptr = ptr_ + column_id * n_rows_;
      for(int i = 0; i < n_rows_; ptr++, i++) {
        point_out[i] = (*ptr);
      }
    }

    void MakeColumnVector(int column_id, std::vector<double> *point_out) const {
      point_out->resize(n_rows_);
      const double *ptr = ptr_ + column_id * n_rows_;
      for(int i = 0; i < n_rows_; ptr++, i++) {
        (*point_out)[i] = (*ptr);
      }
    }

    void MakeColumnVector(
      int i, core::table::DenseConstPoint *point_out) const {
      point_out->Alias(
        ptr_ + i * n_rows_, n_rows_);
    }

    void MakeColumnVector(int i, core::table::DensePoint *point_out) {
      point_out->Alias(
        ptr_ + i * n_rows_, n_rows_);
    }
};
};
};

#endif
