template<typename MultiTreeProblem>
double MultiTreeDepthFirst<MultiTreeProblem>::LeaveOneOutTuplesBase_
(const ArrayList<Tree *> &nodes) {
  
  // Compute the total number of tuples formed among the nodes.
  Tree *current_node = nodes[0];
  int numerator = current_node->count();
  int denominator = 1;
  double total_num_tuples = numerator;
  for(index_t i = 1; i < MultiTreeProblem::order; i++) {
    if(current_node == nodes[i]) {
      if(numerator == 1) {
	total_num_tuples = 0;
	return total_num_tuples;
      }
      else {
	numerator--;
	denominator++;
	total_num_tuples *= ((double) numerator) / ((double) denominator);
      }
    }
    else {
      current_node = nodes[i];
      numerator = current_node->count();
      denominator = 1;
      total_num_tuples *= ((double) numerator) / ((double) denominator);
    }
  }

  for(index_t i = 0; i < MultiTreeProblem::order; i++) {
    int numerator = nodes[i]->count();
    int equal_count = 0;
    for(index_t j = i; j >= 0; j--) {
      if(nodes[j] == nodes[i]) {
	equal_count++;
      }
      else {
	break;
      }
    }
    for(index_t j = i + 1; j < MultiTreeProblem::order; j++) {
      if(nodes[j] == nodes[i]) {
	equal_count++;
      }
      else {
	break;
      }
    }
    total_n_minus_one_tuples_[i] += total_num_tuples /
      ((double) numerator) * ((double) equal_count);
  }
  
  return total_num_tuples;
}

template<typename MultiTreeProblem>
double MultiTreeDepthFirst<MultiTreeProblem>::RecursiveLeaveOneOutTuples_
(ArrayList<Tree *> &nodes, index_t examine_index_start) {
  
  // Test if all the nodes are equal or disjoint.
  bool equal_or_disjoint_flag = true;
  for(index_t i = examine_index_start + 1; i < MultiTreeProblem::order; i++) {

    // If there is a conflict, then return immediately.
    if(nodes[i]->end() <= nodes[i - 1]->begin()) {
      return 0;
    }
    
    // If there is a subsumption, then record the first index that
    // happens so.
    if(equal_or_disjoint_flag) {
      if(first_node_indices_strictly_surround_second_node_indices_
	 (nodes[i - 1], nodes[i])) {
	examine_index_start = i - 1;
	equal_or_disjoint_flag = false;
      }
      else if(first_node_indices_strictly_surround_second_node_indices_
	      (nodes[i], nodes[i - 1])) {
	examine_index_start = i;
	equal_or_disjoint_flag = false;
      }
    }
  }

  // If everything is either disjoint, or equal, then we can call the
  // base case.
  if(equal_or_disjoint_flag) {
    return LeaveOneOutTuplesBase_(nodes);
  }
  else {
    Tree *node_saved = nodes[examine_index_start];
    nodes[examine_index_start] = node_saved->left();
    double left_count = RecursiveLeaveOneOutTuples_(nodes,
						    examine_index_start);
    nodes[examine_index_start] = node_saved->right();
    double right_count = RecursiveLeaveOneOutTuples_(nodes,
						     examine_index_start);
    nodes[examine_index_start] = node_saved;
    return left_count + right_count;
  }

}

template<typename MultiTreeProblem>
void MultiTreeDepthFirst<MultiTreeProblem>::Heuristic_
(Tree *nd, Tree *nd1, Tree *nd2, Tree **partner1, Tree **partner2) {
  
  double d1 = nd->bound().MinDistanceSq(nd1->bound());
  double d2 = nd->bound().MinDistanceSq(nd2->bound());
  
  // Prioritized traversal based on the squared distance bounds.
  if(d1 <= d2) {
    *partner1 = nd1;
    *partner2 = nd2;
  }
  else {
    *partner1 = nd2;
    *partner2 = nd1;
  }
}

template<typename MultiTreeProblem>
void MultiTreeDepthFirst<MultiTreeProblem>::MultiTreeDepthFirstBase_
(const ArrayList<Matrix *> &sets, ArrayList<Tree *> &nodes,
 typename MultiTreeProblem::MultiTreeQueryResult &query_results,
 double total_num_tuples) {

  MultiTreeHelper_<0, MultiTreeProblem::order>::NestedLoop
    (globals_, sets, nodes, query_results);

  // Add the postponed information to each point, without causing any
  // duplicate information transmission.
  for(index_t i = 0; i < MultiTreeProblem::order; i++) {
    if(i > 0 && nodes[i] == nodes[i - 1]) {
      continue;
    }
    
    Tree *qnode = nodes[i];

    // Clear the summary statistics of the current query node so that
    // we can refine it to better bounds.
    qnode->stat().summary.StartReaccumulate();

    for(index_t q = qnode->begin(); q < qnode->end(); q++) {

      // Apply postponed to each point.
      query_results.ApplyPostponed(qnode->stat().postponed, q);

      // Refine statistics.
      qnode->stat().summary.Accumulate(query_results, q);

      // Increment the number of (n - 1) tuples pruned.
      query_results.n_pruned[q] += total_n_minus_one_tuples_[i];
    }

    // Clear postponed information.
    qnode->stat().postponed.SetZero();
  }
}

template<typename MultiTreeProblem>
void MultiTreeDepthFirst<MultiTreeProblem>::CopyNodeSet_
(const ArrayList<Tree *> &source_list, ArrayList<Tree *> *destination_list) {

  destination_list->Init(source_list.size());

  for(index_t i = 0; i < MultiTreeProblem::order; i++) {
    (*destination_list)[i] = source_list[i];
  }
}

template<typename MultiTreeProblem>
void MultiTreeDepthFirst<MultiTreeProblem>::MultiTreeDepthFirstCanonical_
(const ArrayList<Matrix *> &sets, ArrayList<Tree *> &nodes,
 typename MultiTreeProblem::MultiTreeQueryResult &query_results,
 double total_num_tuples) {

  if(MultiTreeProblem::ConsiderTupleExact(globals_, query_results,
					  nodes, total_num_tuples,
					  total_n_minus_one_tuples_root_,
					  total_n_minus_one_tuples_)) {
    return;
  }
  else if(MultiTreeProblem::ConsiderTupleProbabilistic
	  (globals_, query_results, sets, nodes, total_num_tuples,
	   total_n_minus_one_tuples_root_, total_n_minus_one_tuples_)) {
    return;
  }

  // Recurse to every combination...
  MultiTreeHelper_<0, MultiTreeProblem::order>::RecursionLoop
    (sets, nodes, total_num_tuples, false, query_results, this);    
  return;
}

template<typename MultiTreeProblem>
void MultiTreeDepthFirst<MultiTreeProblem>::PreProcessTree_
(Tree *node, const ArrayList<double> &squared_distances,
 const ArrayList<double> &squared_fn_distances) {

  if(node->is_leaf()) {
    node->stat().min_squared_nn_dist = DBL_MAX;
    node->stat().max_squared_fn_dist = 0;
    for(index_t q = node->begin(); q < node->end(); q++) {
      node->stat().min_squared_nn_dist = 
	std::min(node->stat().min_squared_nn_dist, squared_distances[q]);
      node->stat().max_squared_fn_dist =
	std::max(node->stat().max_squared_fn_dist, squared_fn_distances[q]);
    }
  }
  else {
    PreProcessTree_(node->left(), squared_distances, squared_fn_distances);
    PreProcessTree_(node->right(), squared_distances, squared_fn_distances);
    node->stat().min_squared_nn_dist =
      std::min(node->left()->stat().min_squared_nn_dist,
	       node->right()->stat().min_squared_nn_dist);
    node->stat().max_squared_fn_dist =
      std::max(node->left()->stat().max_squared_fn_dist,
	       node->right()->stat().max_squared_fn_dist);
  }
}

template<typename MultiTreeProblem>
void MultiTreeDepthFirst<MultiTreeProblem>::PostProcessTree_
(Tree *node, typename MultiTreeProblem::MultiTreeQueryResult &query_results) {
  
  if(node->is_leaf()) {
    for(index_t i = node->begin(); i < node->end(); i++) {
      query_results.ApplyPostponed(node->stat().postponed, i);
      query_results.PostProcess(i);
    }
  }
  else {

    // Push down postponed contributions to the left and the right.
    node->left()->stat().postponed.ApplyPostponed(node->stat().postponed);
    node->right()->stat().postponed.ApplyPostponed(node->stat().postponed);
    
    PostProcessTree_(node->left(), query_results);
    PostProcessTree_(node->right(), query_results);
  }

  node->stat().postponed.SetZero();
}
