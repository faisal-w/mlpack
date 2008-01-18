/*
 * =====================================================================================
 * 
 *       Filename:  sparse_matrix.h
 * 
 *    Description:  
 * 
 *        Version:  1.0
 *        Created:  12/01/2007 04:12:00 PM EST
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 * 
 * =====================================================================================
 */

#ifndef SPARSE_MATRIX_H_
#define SPARSE_MATRIX_H_
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#ifndef USE_TRILINOS
#define USE_TRILINOS
#endif
#include <stdio.h>
#include <errno.h>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include "fastlib/fastlib.h"
#include "la/matrix.h"
// you need this because trillinos redifines it. It's ok
// if you don't have it, but you will get an annoying warning
#ifdef F77_FUNC
#undef F77_FUNC
#endif
#include "trilinos/include/Epetra_CrsMatrix.h"
#include "trilinos/include/Epetra_SerialComm.h"
#include "trilinos/include/Epetra_Map.h"
#include "trilinos/include/Epetra_Vector.h"
#include "trilinos/include/Epetra_MultiVector.h"
#include "trilinos/include/AnasaziBasicEigenproblem.hpp"
#include "trilinos/include/AnasaziEpetraAdapter.hpp"
#include "trilinos/include/AnasaziBlockKrylovSchurSolMgr.hpp"
#include "trilinos/include/AztecOO.h"
#include "trilinos/include/Ifpack_CrsIct.h"

/* class SparseMatrix created by Nick
 * This is a sparse matrix wrapper for trilinos Epetra_CrsMatrix
 * It is much simpler than Epetra_CrsMatrix. At this time 
 * it supports eigenvalues (Krylov method) and linear system solution
 * I have added matrix addition/subtraction multiplication 
 * I am also trying to add the submatrices
 * Note: There is a restriction on these matrices, the number of rows is
 * always greater or equal to the number of columns. The number of rows
 * is also called dimension. We pose this restriction because trilinos supports
 * square matrices only. In sparse matrices though this is not the problem since
 * an mxn matrix where  m>n can is equivalent to an mxm matrix where all the
 * elements with n<j<m are zero
 */

class Sparsem;
class SparseMatrix {
 public:
	 friend class Sparsem;
 // Some typedefs for oft-used data types
  typedef Epetra_MultiVector MV;
  typedef Epetra_Operator OP;
  typedef Anasazi::MultiVecTraits<double, Epetra_MultiVector> MVT;

  SparseMatrix() ;
	/** Constructor 
	  num_of_rows: number of rows
	  num_of_cols: number of columns
	  nnz_per_row: an estimate of the non zero elements per row
	                This doesn't need to be accurate. If you need
	                more it will automatically resize. Try to be as accurate
	                as you can because resizing costs. It is better if your
	                estimete if greater than the true non zero elements. So
	                it is better to overestimate than underestimate 
	*/
	SparseMatrix(const index_t num_of_rows, 
			         const index_t num_of_cols, 
			         const index_t nnz_per_row);
  /** Copy constructor */
  SparseMatrix(const SparseMatrix &other);
	SparseMatrix(std::string textfile); 
	~SparseMatrix(); 
	/** Use this initializer like the Constructor */
  void Init(const index_t num_of_rows, 
			      const index_t num_of_columns, 
			      const index_t nnz_per_row);
	/** This Initializer is like the previous one with the main difference that
	  for every row we give a seperate estimate for the non-zero elements.
	*/
  void Init(index_t num_of_rows, index_t num_of_columns, index_t *nnz_per_row);
	/** This Initializer fills the sparse matrix with data.
	    row_indices: row indices for non-zero elements
	    col_indices: column indices for non-zero elements
	    values     : values of non-zeros elements
	    If the dimension (number of rows)and the expected (nnz elements per row)
	    are set to a negative value, the function will automatically detect it 
  */
	void Init(const std::vector<index_t> &row_indices,
		       	const std::vector<index_t> &col_indices,
			      const Vector &values, 
			      index_t nnz_per_row, 
			      index_t dimension);
	/** The same as above but we use STL vector for values
	*/
  void Init(const std::vector<index_t> &row_indices,
		       	const std::vector<index_t> &col_indices,
			      const std::vector<double> &values, 
			      index_t nnz_per_row, 
			      index_t dimension);

	/** Initialize from a text file in the following format
	    row column value \n
	    WARNING !!!!!!!
	    the text file must be sorted according to the rows
	    meaning that the rows should be in increasing order
	    you can do that easily in unix with the sort -n command
	*/
	void Init(std::string textfile);
	/** Copy function, used also by copy constructor
	*/
  void Copy(const SparseMatrix &other);
	/** Not implemented yet
	*/
  void Alias(const Matrix& other);

	void Destruct();
	/** Initialize the diagonal 
	*/
	void InitDiagonal(const Vector &vec); 
	/** Initialize the diagonal with a constant
	*/
	void InitDiagonal(const double value);
	/** It is recomended that you load the matrix row-wise, Before
	    you do that call StartLoadingRows()
  */
	void StartLoadingRows();
	/** All these functions load Rows, with the data in different format
	*/
  void LoadRow(index_t row, std::vector<index_t> &columns, Vector &values);
  void LoadRow(index_t row, index_t *columns, Vector &values);
  void LoadRow(index_t row, index_t num, index_t *columns, double  *values);
  void LoadRow(index_t row, std::vector<index_t> &columns, std::vector<double>  &values);
	/** When you are done call this it does some optimization in the storage, no
	    further asignment 
	*/
	void EndLoading();
	/** It makes the matrix symmetric. It scans the rows of the matrix and for every (i,j)
	    element (j,i) equal to (j,i)
	*/
  void MakeSymmetric();	
	/** if you know that the matrix is symmetric set the flag
	*/
	void set_symmetric(bool val) {
	  issymmetric_ = val;
	}
	/** Sets the diagonal with the values of the vector
	*/
  void SetDiagonal(const Vector &vector); 
	/** Sets the diagonal withe a scalar
	*/
	void SetDiagonal(const double scalar); 
	/** Not Implemented yet
	*/
  void SwapValues(SparseMatrix* other);
  /** Access values, It will fail if EndLoading() has been called
	*/
	double get(index_t r, index_t c) const;
	/** Set Values
	*/
  void   set(index_t r, index_t c, double v);
	/** scales the matrix with a scalar
	*/
	void Scale(double scalar) {
	  matrix_->Scale(scalar);
	}
	void Negate();
	/** The matrix will be scaled such that A(i,j) = x(j)*A(i,j) 
	    where i denotes the global row number of A and j denotes the  column number 
	*/		
	void ColumnScale(const Vector &vec) {
    Epetra_Vector temp(View, *map_, (double*)vec.ptr());
    matrix_->RightScale(temp);
	}
	/** The  matrix will be scaled such that A(i,j) = x(i)*A(i,j) 
	    where i denotes the row number of A and j denotes the column number of A.
	*/
	void RowScale(const Vector &vec) {
    Epetra_Vector temp(View, *map_, (double *)vec.ptr());
    matrix_->LeftScale(temp);
	}
	/** computes the L1 norm
	*/
	double L1Norm() {
	  return matrix_->NormOne();
	}
	/** L infinity norm
	*/
	double LInfNorm() {
		return matrix_->NormInf();
	}
	/** Computes the inverse of the sum of absolute values of the rows 
	    of the matrix 
	*/
	void InvRowsSums(Vector *result) {
	  Epetra_Vector temp(View, *map_, result->ptr());
		matrix_->InvRowSums(temp);
	}
	/** Computes the inv of max of absolute values of the rows of the matrix
	*/
	void InvRowMaxs(Vector *result) {
	  Epetra_Vector temp(View, *map_, result->ptr());
    matrix_->InvRowMaxs(temp);
	} 
	/** Computes the inverse of the sum of absolute values of the columns of the
	    matrix
	*/
	void InvColSums(Vector *result) {
    Epetra_Vector temp(View, *map_, result->ptr());
    matrix_->InvColSums(temp);
	}
  /** Computes the inv of max of absolute values of the columns of the matrix
	*/
	void InvColMaxs(Vector *result) {
	  Epetra_Vector temp(View, *map_, result->ptr());
    matrix_->InvColMaxs(temp);
	}
	/** Get the number of rows
	*/
  index_t num_of_rows() {
		return num_of_rows_;
	}
	/** Get the number of columns
	*/
  index_t num_of_columns() {
	  return num_of_columns_;
	}
	/** Dimension should be equal to the number of rows
	*/
	index_t dimension() {
	  return dimension_;
	}
	/** The number of non zero elements
	*/
	index_t nnz() {
	 return matrix_->NumGlobalNonzeros();
  }
	/** Apply a function on every non-zero element, very usefull for kernels
	    If you have entered a zero element then it will also be applied on it 
	    as well
  */
	template<typename FUNC>
	void ApplyFunction(FUNC &function);
  /** For debug purposes you can call it to print the matrix
	*/
	std::string Print() {
		std::ostringstream s1;
		matrix_->Print(s1);
		return s1.str();
	}
	void ToFile(std::string file);
	/** Computes the eignvalues with the Krylov Method
	    index_t num_of_eigvalues:   number of eigenvalues to compute
			     std::string eigtype:   Choose which eigenvalues to compute
                                  Choices are:
                                  LM - target the largest magnitude  
                                  SM - target the smallest magnitude 
                                  LR - target the largest real 
                                  SR - target the smallest real 
                                  LI - target the largest imaginary
                                  SI - target the smallest imaginary
		       Matrix *eigvectors:    The eigenvectors computed must not be initialized
					 std::vector<double> *real_eigvalues:  real part of the eigenvalues
					                                       must be initialized, but should not
																								 allocate space. The eigenvalues
																								 returned might actually be less 
																								 than the ones requested
																								 for example when the matrix has 
																								 rank n< eigenvalues requested
					 std::vector<double> *imag_eigvalues:  imaginary part of the eigenvalues
					                                       must be initialized. If the
																								 problem is symmetric there is 
																								 no need to initialize. 
																								 The same as real_eigvalues
																								 hold for the space allocated
																								 in the non-symmetric case    

	*/
	void Eig(index_t num_of_eigvalues, 
			     std::string eigtype, 
		       Matrix *eigvectors,  
					 std::vector<double> *real_eigvalues, 
					 std::vector<double> *imag_eigvalues);
  /** Linear System solution, Call Endloading First.
	*/
	void LinSolve(Vector &b, // must be initialized (space allocated)
			          Vector *x, // must be initialized (space allocated)
								double tolerance,
								index_t iterations);

	/** Use this for the general case
	*/
  void LinSolve(Vector &b, Vector *x) {
	  LinSolve(b, x, 1E-9, 1000);
	}
  void IncompleteCholesky(index_t level_fill,
		                      double drop_tol,
			 									  SparseMatrix *u, 
													Vector       *d,
													double *condest);   	

 private:
	index_t dimension_;
	index_t num_of_rows_;
	index_t num_of_columns_;
	Epetra_SerialComm comm_;
	bool issymmetric_;
	Epetra_Map *map_;
  Teuchos::RCP<Epetra_CrsMatrix> matrix_;
  index_t *my_global_elements_;
	void Init(const Epetra_CrsMatrix &other); 
  void Load(const std::vector<index_t> &rows, 
			      const std::vector<index_t> &columns, 
						const Vector &values);
	void AllRowsLoad(Vector &rows, Vector &columns);

};

class Sparsem {
 public:
  static inline void Add(const SparseMatrix &a, 
			                   const SparseMatrix &b, 
												 SparseMatrix *result);

	static inline void Subtract(const SparseMatrix &a,
			                        const SparseMatrix &b,
														  SparseMatrix *result);

	/** Multiplication of two matrices A*B in matlab notation 
	    If B is symmetric then it is much faster, because we can
	    multiply rows. Otherwise we have to compute the transpose
	    As an advise multiplication of two sparse matrices might 
	    lead to a dense one, so please be carefull
	 */
	static inline void Multiply(const SparseMatrix &a,
			                        const SparseMatrix &b,
															SparseMatrix *result);
	/* Computes the result = A * A^T
	*/
	static inline void MultiplyT(SparseMatrix &a,
															SparseMatrix *result);

	/** The transpose flag should be set to true if 
	    we want to use the transpose of mat, otherwise
	    set it to false.
	*/
	static inline void Multiply(const SparseMatrix &mat,
			                        const Vector &vec,
															Vector *result,
															bool transpose_flag);
	/** Multiply the matrix with a scalar
	*/
	static inline void Multiply(const SparseMatrix &mat,
			                        const double scalar,
															SparseMatrix *result);
	
	/** element wise multiplication of the matrices
	    A.*B in matlab notation
	*/
	static inline void DotMultiply(const SparseMatrix &a,
		                             const SparseMatrix &b,
		                             SparseMatrix *result);
}; 
#include "sparse/sparse_matrix_impl.h"
#endif
