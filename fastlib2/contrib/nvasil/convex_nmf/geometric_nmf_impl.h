/*
 * =====================================================================================
 * 
 *       Filename:  geometric_nmf_impl.h
 * 
 *    Description:  
 * 
 *        Version:  1.0
 *        Created:  07/02/2008 02:41:34 PM EDT
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 * 
 * =====================================================================================
 */

void GeometricNmf::Init(fx_module *module, 
			ArrayList<index_t> &rows,
			ArrayList<index_t> &columns,
      ArrayList<double>  &values) {
  module_=module;
  // we need to put the data back to a matrix to build the 
  // tree and etc
  rows_.InitCopy(rows);
	columns_.InitCopy(columns);
	values_.InitCopy(values);
	num_of_rows_=*std::max_element(rows_.begin(), rows_.end())+1;
	num_of_columns_=*std::max_element(columns_.begin(), columns_.end())+1;
  Matrix data_mat;
  data_mat.Init(num_of_rows_, num_of_columns_);
  data_mat.SetAll(0.0);
  index_t count=0;
  for(index_t i=0; i<data_mat.n_rows(); i++) {
    for(index_t j=0; j< data_mat.n_cols(); j++) {
      data_mat.set(i, j, values[count]);
      count++;
    }
  } 
  // we also need to take the logarithms of the values
  for(index_t i=0; i<values_.size(); i++) {
    values_[i]=log(values[i]);
  }
/*
  knns_ = fx_param_int(module_, "knns", 2);
  leaf_size_ = fx_param_int(module_, "leaf_size", 20);
  NOTIFY("Data loaded ...\n");
  NOTIFY("Nearest neighbor constraints ...\n");
  NOTIFY("Building tree with data ...\n");
  allknn_.Init(data_mat, leaf_size_, knns_); 
  NOTIFY("Tree built ...\n");
  NOTIFY("Computing neighborhoods ...\n");
  ArrayList<index_t> from_tree_neighbors;
  ArrayList<double>  from_tree_distances;
  allknn_.ComputeNeighbors(&from_tree_neighbors,
                           &from_tree_distances);
  NOTIFY("Neighborhoods computed...\n");
  NOTIFY("Consolidating neighbors...\n");
  MaxVarianceUtils::ConsolidateNeighbors(from_tree_neighbors,
      from_tree_distances,
      knns_,
      knns_,
      &nearest_neighbor_pairs_,
      &nearest_distances_,
      &num_of_nearest_pairs_);
  // Now compute the nearest neighbor log dot products
  nearest_dot_products_.Init(nearest_distances_.size());
  for(index_t i=0; i<nearest_distances_.size(); i++) {
    index_t p1=nearest_neighbor_pairs_[i].first;
    index_t p2=nearest_neighbor_pairs_[i].second;
    nearest_dot_products_[i]=log(la::Dot(data_mat.n_rows(),
        data_mat.GetColumnPtr(p1),
        data_mat.GetColumnPtr(p2)));
  }
  */
	new_dim_=fx_param_int(module_, "new_dim", 5); 
  desired_duality_gap_=fx_param_double(module_, "desired_duality_gap", 1e-4);
  gradient_tolerance_=fx_param_double(module_, "gradient_tolerance", 1);
  v_accuracy_=fx_param_double(module_, "v_accuracy", 1e-4);
	// It assumes an N x new_dim_ array, where N=num_rows+num_columns
  
  offset_h_ = num_of_rows_;
  offset_epsilon_=num_of_rows_+num_of_columns_;
  num_of_logs_=values_.size()*new_dim_;//+nearest_dot_products_.size();
}

void GeometricNmf::ComputeGradient(Matrix &coordinates, 
    Matrix *gradient) {
  gradient->SetAll(0.0);
  // compute the objective
  for(index_t i=0; i<num_of_rows_+num_of_columns_; i++) {
    for(index_t j=0; j<new_dim_; j++) {
      double grad=-sigma_*2*exp(-2*coordinates.get(j, i));
      gradient->set(j, i, grad);
    }
  }
  
  // mathcing dot products 
  for(index_t i=0; i<values_.size(); i++) {
    index_t w_i=rows_[i];
    index_t h_i=columns_[i]+offset_h_;
    double v=values_[i];
    double new_v=0;
    for(index_t j=0; j<new_dim_; j++) {
      double w=coordinates.get(j, w_i);
      double h=coordinates.get(j, h_i);
      new_v+=exp(w+h-v);
    }

    double denominator=1-new_v;
    DEBUG_ERR_MSG_IF(denominator<=1e-200, 
        "Something is wrong I cought a denominator out of the interior" );

    for(index_t j=0; j<new_dim_; j++) {
      double w=coordinates.get(j, w_i);
      double h=coordinates.get(j, h_i);
      double grad_w=gradient->get(j, w_i);
      double grad_h=gradient->get(j, h_i);    
      double grad=exp(w+h-v)/denominator;
      grad_w+=grad;
      grad_h+=grad;
      gradient->set(j, w_i, grad_w);
      gradient->set(j, h_i, grad_h);
    }
  }  

/*
  // neighborhood constraints
  for(index_t i=0; i<nearest_neighbor_pairs_.size(); i++) {
    index_t w_1=nearest_neighbor_pairs_[i].first;
    index_t w_2=nearest_neighbor_pairs_[i].second;
    double v=nearest_dot_products_[i];
    double new_v=0;
    for(index_t j=0; j<new_dim_; j++) {
      double w1=coordinates.get(j, w_1);
      double w2=coordinates.get(j, w_2);
      new_v+=exp(w1+w2-v);
    }

    double denominator=1-new_v;
    for(index_t j=0; j<new_dim_; j++) {
      double w1=coordinates.get(j, w_1);
      double w2=coordinates.get(j, w_2);
      double grad_w1=gradient->get(j, w_1);
      double grad_w2=gradient->get(j, w_2);    
      DEBUG_ERR_MSG_IF(fabs(denominator)<=1e-200, 
          "Something is wrong I cought a denominator close to zero" );
      double grad=exp(w1+w2-v)/denominator;
      grad_w1+=grad;
      grad_w2+=grad;
      gradient->set(j, w_1, grad_w1);
      gradient->set(j, w_2, grad_w2);
    }
  }
*/
  
}

void GeometricNmf::ComputeObjective(Matrix &coordinates, 
                                    double *objective) {
  *objective=0;
  for(index_t i=0; i<num_of_rows_+num_of_columns_; i++) {
    for(index_t j=0; j<new_dim_; j++) {
      *objective+=exp(-2*coordinates.get(j, i));
    }
  }
}

void GeometricNmf::ComputeFeasibilityError(Matrix &coordinates, 
                                           double *error) {
  // return duality gap instead
  *error=num_of_logs_/sigma_;
  //double lagrangian=ComputeLagrangian(coordinates);
  //double objective;
  //ComputeObjective(coordinates, &objective);
  //NOTIFY("sum of log barriers:%lg ", lagrangian-sigma_*objective);
}

double GeometricNmf::ComputeLagrangian(Matrix &coordinates) {
  double lagrangian=0;
  // from the objective functions
  ComputeObjective(coordinates, &lagrangian);
  lagrangian*=sigma_;
  
  // mathcing dot products and the objective
  double temp_prod=1;
  for(index_t i=0; i<values_.size(); i++) {
    index_t w_i=rows_[i];
    index_t h_i=columns_[i]+offset_h_;
    double v=values_[i];
    double new_v=0;
    for(index_t j=0; j<new_dim_; j++) {
      double w=coordinates.get(j, w_i);
      double h=coordinates.get(j, h_i);
      new_v+=exp(w+h-v);
    }
    if (1-new_v<=0) {
      return DBL_MAX;
    }
    if (temp_prod<1e-50 || temp_prod > 1e50) {
      lagrangian+=-log(temp_prod);
      temp_prod=1;
    } else {
      temp_prod*=1-new_v;
    }
  }
  lagrangian+=-log(temp_prod);
  temp_prod=1;  
/*
  // neighborhood constraints
  for(index_t i=0; i<nearest_neighbor_pairs_.size(); i++) {
    index_t w_1=nearest_neighbor_pairs_[i].first;
    index_t w_2=nearest_neighbor_pairs_[i].second;
    double v=nearest_dot_products_[i];
    double new_v=0;
    for(index_t j=0; j<new_dim_; j++) {
      double w1=coordinates.get(j, w_1);
      double w2=coordinates.get(j, w_2);
      new_v+=exp(w1+w2-v);
    }
    if (1-new_v<=0) {
      return DBL_MAX;    
    } 
    if (temp_prod<1e-50 || temp_prod > 1e50) {
      lagrangian+=-log(temp_prod);
      temp_prod=1;
    } else {
      temp_prod*=1-new_v;
    }
  }
  lagrangian+=-log(temp_prod);
  */
  return lagrangian;
}

void GeometricNmf::UpdateLagrangeMult(Matrix &coordinates) {

}

void GeometricNmf::Project(Matrix *coordinates) {
  //OptUtils::NonNegativeProjection(coordinates);
}

void GeometricNmf::set_sigma(double sigma) {
  sigma_=sigma;
}

void GeometricNmf::GiveInitMatrix(Matrix *init_data) {
  init_data->Init(new_dim_, num_of_rows_+num_of_columns_);
  double temp=(*std::min_element(values_.begin(), 
      values_.end())-1)/new_dim_;

  init_data->SetAll(temp);
}

bool GeometricNmf::IsDiverging(double objective) {
  return false;
}

bool GeometricNmf::IsOptimizationOver(Matrix &coordinates, 
    Matrix &gradient, double step) {
  double norm_gradient = la::Dot(gradient.n_elements(), 
      gradient.ptr(), gradient.ptr());
  //  one of our barriers is zero
  if (norm_gradient>=DBL_MAX) {
    return true;
  }
  if (num_of_logs_/sigma_ < desired_duality_gap_) {
    return true;
  } else {
    return false;
  }  
}

bool GeometricNmf::IsIntermediateStepOver(Matrix &coordinates, 
    Matrix &gradient, double step) {
  double norm_gradient = la::Dot(gradient.n_elements(), 
      gradient.ptr(), gradient.ptr());
  //NOTIFY("norm_gradient:%lg step:%lg , gradient_tolerance_:%lg", 
  //    norm_gradient, step, gradient_tolerance_);
  if (norm_gradient*step < gradient_tolerance_ || step==0.0) {
    return true;
  } else {
    return false;
  }
}
 
