#include <values.h>

#include "series_expansion_aux.h"

const Vector& SeriesExpansionAux::get_inv_multiindex_factorials() const {
  return inv_multiindex_factorials_;
}

int SeriesExpansionAux::get_max_total_num_coeffs() const {
  return list_total_num_coeffs_[max_order_];
}

const ArrayList < int > &SeriesExpansionAux::get_multiindex(int pos) const {
  return multiindex_mapping_[pos];
}

const ArrayList < int > *SeriesExpansionAux::get_multiindex_mapping() const {
  return multiindex_mapping_.begin();
}

const Vector& SeriesExpansionAux::get_neg_inv_multiindex_factorials() const {
  return neg_inv_multiindex_factorials_;
}

double SeriesExpansionAux::get_n_choose_k(int n, int k) const {
  return n_choose_k_.get(n, (int) math::ClampNonNegative(k));
}

double SeriesExpansionAux::get_n_multichoose_k_by_pos(int n, int k) const {
  return multiindex_combination_.get(n, k);
}

int SeriesExpansionAux::get_total_num_coeffs(int order) const {
  return list_total_num_coeffs_[order];
}

int SeriesExpansionAux::ComputeMultiindexPosition
(const ArrayList<int> &multiindex) const {

  int dim = multiindex.size();
  int mapping_sum = 0;
  int index = 0;

  for(index_t j = 0; j < dim; j++) {
    mapping_sum += multiindex[j];
  }
  for(index_t j = 0; j < dim; j++) {
    index += (int) get_n_choose_k(mapping_sum + dim - j - 1, dim - j);
    mapping_sum -= multiindex[j];
  }
  
  return index;
}

void SeriesExpansionAux::Init(int max_order, int dim) {

  int p, k, t, tail, i, j;
  ArrayList<int> heads;
  ArrayList<int> cinds;

  // initialize max order and dimension
  dim_ = dim;
  max_order_ = max_order;
  
  // compute the list of total number of coefficients for p-th order expansion
  list_total_num_coeffs_.Init(max_order + 1);
  for(p = 0; p <= max_order; p++) {
    list_total_num_coeffs_[p] = nchoosek(p + dim, dim);
  }

  // allocate space for inverse factorial and 
  // negative inverse factorials and multiindex mapping and n_choose_k 
  // and multiindex_combination precomputed factors
  inv_multiindex_factorials_.Init(list_total_num_coeffs_[max_order]);  
  neg_inv_multiindex_factorials_.Init(list_total_num_coeffs_[max_order]);
  multiindex_mapping_.Init(list_total_num_coeffs_[max_order]);
  (multiindex_mapping_[0]).Init(dim_);
  for(j = 0; j < dim; j++) {
    (multiindex_mapping_[0])[j] = 0;
  }
  n_choose_k_.Init(max_order + dim + 1, max_order + dim + 1);
  n_choose_k_.SetZero();
  multiindex_combination_.Init(list_total_num_coeffs_[max_order],
			       list_total_num_coeffs_[max_order]);

  // initialization of temporary variables for computation...
  heads.Init(dim + 1);
  cinds.Init(list_total_num_coeffs_[max_order]);

  for(i = 0; i < dim; i++) {
    heads[i] = 0;
  }
  heads[dim] = MAXINT;
  cinds[0] = 0;
  
  // compute inverse factorial and negative inverse factorials and
  // multiindex mappings...
  inv_multiindex_factorials_[0] = 1.0;
  neg_inv_multiindex_factorials_[0] = 1.0;
  for(k = 1, t = 1, tail = 1; k <= max_order; k++, tail = t) {
    for(i = 0; i < dim; i++) {
      int head = (int) heads[i];
      heads[i] = t;
      for(j = head; j < tail; j++, t++) {
	cinds[t] = (j < heads[i + 1]) ? cinds[j] + 1 : 1;
	inv_multiindex_factorials_[t] = 
	  inv_multiindex_factorials_[j] / cinds[t];
	neg_inv_multiindex_factorials_[t] =
	  -neg_inv_multiindex_factorials_[j] / cinds[t];
	
	(multiindex_mapping_[t]).Copy(multiindex_mapping_[j]);
	(multiindex_mapping_[t])[i] = (multiindex_mapping_[t])[i] + 1;
      }
    }
  }

  // compute n choose k's
  for(j = 0; j <= max_order + dim; j++) {
    for(k = 0; k <= max_order + dim; k++) {
      n_choose_k_.set(j, k, nchoosek(j, k));
    }
  }

  // initialize multiindex_combination matrix beta choose alpha
  for(j = 0; j < list_total_num_coeffs_[max_order_]; j++) {
    
    // beta mapping
    ArrayList<int> beta_mapping = multiindex_mapping_[j];

    for(k = 0; k < list_total_num_coeffs_[max_order_]; k++) {

      // alpha mapping
      ArrayList<int> alpha_mapping = multiindex_mapping_[k];

      // initialize the factor to 1
      multiindex_combination_.set(j, k, 1);

      for(i = 0; i < dim; i++) {
	multiindex_combination_.set
	  (j, k, multiindex_combination_.get(j, k) * 
	   n_choose_k_.get(beta_mapping[i], alpha_mapping[i]));

	if(multiindex_combination_.get(j, k) == 0)
	  break;
      }
    }
  }
}

void SeriesExpansionAux::PrintDebug(const char *name, FILE *stream) const {

  fprintf(stream, "----- SERIESEXPANSIONAUX %s ------\n", name);
  fprintf(stream, "Max order: %d, dimension: %d\n", max_order_, dim_);

  fprintf(stream, "Multiindex mapping: ");
  for (index_t i = 0; i < multiindex_mapping_.size(); i++) {

    DEBUG_ASSERT_MSG(ComputeMultiindexPosition(multiindex_mapping_[i]) == i,
		     "REIMPLEMENT ComputeMultiindexPosition function!");
    fprintf(stream, "( ");
    for(index_t j = 0; j < dim_; j++) {
      fprintf(stream, "%d ", multiindex_mapping_[i][j]);
    }
    fprintf(stream, "): %g %g ", inv_multiindex_factorials_[i],
	    neg_inv_multiindex_factorials_[i]);
  }
  fprintf(stream, "\n");
}
