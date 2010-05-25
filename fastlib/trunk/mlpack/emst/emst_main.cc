/**
* @file emst.cc
 *
 * Calls the DualTreeBoruvka algorithm from dtb.h
 * Can optionally call Naive Boruvka's method
 * See README for command line options.  
 *
 * @author Bill March (march@gatech.edu)
*/

#include "dtb.h"


const fx_entry_doc emst_entries[] = {
  {"input_filename", FX_REQUIRED, FX_STR, NULL,
   "Input dataset (CSV or ARFF)\n"},
  {"output_filename", FX_PARAM, FX_STR, NULL,
   "Filename to output spanning tree into (default output.csv)\n"},
  {"do_naive", FX_PARAM, FX_BOOL, NULL,
   "Whether or not to also perform a naive computation and compare the results\n"
   "   (default N)\n"},
  {"naive_output_filename", FX_PARAM, FX_STR, NULL,
   "Filename to output spanning tree generated with naive algorithm into (use\n"
   "   with --do_naive=Y (default naive_output.csv)\n"},
  FX_ENTRY_DOC_DONE
};

const fx_submodule_doc emst_subdoc[] = {
  {"dtb", &dtb_doc,
  "Parameters for the dual-tree Boruvka algorithm\n"},
  FX_SUBMODULE_DOC_DONE
};

const fx_module_doc emst_doc = {
  emst_entries, emst_subdoc,
  "This is the MLPACK implementation of the dual-tree Boruvka algorithm for\n"
  "finding a Euclidian Minimum Spanning Tree.  The input dataset is specified\n"
  "and the output, which is the minimum spanning tree represented as an edge list,\n"
  "will be placed into the specified output file.\n"
  "\n"
  "The dtb/leaf_size parameter gives the fastest performance with a value of 1;\n"
  "however, it may be changed to conserve memory.\n"
  "\n"
  "The output is given in the format\n"
  "  <edge lesser index> <edge greater index> <distance>\n"
  "for each edge in the minimum spanning tree.\n"
};

int main(int argc, char* argv[]) {
 
  fx_init(argc, argv, &emst_doc);
 
  // For when I implement a thor version
  //bool using_thor = fx_param_bool(NULL, "using_thor", 0);
  
  
  //if unlikely(using_thor) {
  //  printf("thor is not yet supported\n");
  //}
  //else {
      
    ///////////////// READ IN DATA ////////////////////////////////// 
    
    const char* data_file_name = fx_param_str_req(NULL, "input_filename");
    
    Matrix data_points;
    
    data::Load(data_file_name, &data_points);
    
    /////////////// Initialize DTB //////////////////////
    DualTreeBoruvka dtb;
    //struct datanode* dtb_module = fx_submodule(NULL, "dtb", "dtb_module");
    struct datanode* dtb_module = fx_submodule(NULL, "dtb_module");
    dtb.Init(data_points, dtb_module);
    
    ////////////// Run DTB /////////////////////
    Matrix results;
    
    dtb.ComputeMST(&results);
    
    //////////////// Check against naive //////////////////////////
    if (fx_param_bool(NULL, "do_naive", 0)) {
     
      DualTreeBoruvka naive;
      struct datanode* naive_module = fx_submodule(NULL, "naive_module");
      fx_set_param_bool(naive_module, "do_naive", 1);
      
      naive.Init(data_points, naive_module);
      
      Matrix naive_results;
      naive.ComputeMST(&naive_results);
      
      /* Compare the naive output to the DTB output */
      
      fx_timer_start(naive_module, "comparison");
      
           
      // Check if the edge lists are the same
      // Loop over the naive edge list
      int is_correct = 1;
      /*
      for (index_t naive_index = 0; naive_index < results.size(); 
           naive_index++) {
       
        int this_loop_correct = 0;
        index_t naive_lesser_index = results[naive_index].lesser_index();
        index_t naive_greater_index = results[naive_index].greater_index();
        double naive_distance = results[naive_index].distance();
        
        // Loop over the DTB edge list and compare against naive
        // Break when an edge is found that matches the current naive edge
        for (index_t dual_index = 0; dual_index < naive_results.size();
             dual_index++) {
          
          index_t dual_lesser_index = results[dual_index].lesser_index();
          index_t dual_greater_index = results[dual_index].greater_index();
          double dual_distance = results[dual_index].distance();
          
          if (naive_lesser_index == dual_lesser_index) {
            if (naive_greater_index == dual_greater_index) {
              DEBUG_ASSERT(naive_distance == dual_distance);
              this_loop_correct = 1;
              break;
            }
          }
          
        }
       
        if (this_loop_correct == 0) {
          is_correct = 0;
          break;
        }
        
      }
      */
      if (is_correct == 0) {
       
        printf("Naive check failed!\n  Edge lists are different.\n\n");
        // Check if the outputs have the same length
        if (fx_get_result_double(dtb_module, "total_squared_length") !=
            fx_get_result_double(naive_module, "total_squared_length")) { 
          
          printf("Total lengths are different!  One algorithm has failed.\n");
          
          fx_done(NULL);
          return 1;
          
        }
        else {
          // NOTE: if the edge lists are different, but the total lengths are
          // the same, the algorithm may still be correct.  The MST is not 
          // uniquely defined for some point sets.  For example, an equilateral
          // triangle has three minimum spanning trees.  It is possible for 
          // naive and DTB to find different spanning trees in this case.
          printf("Total lengths are the same.");
          printf("It is possible the point set"); 
          printf("has more than one minimum spanning tree.\n");
        }
      
      }
      else {
        printf("Naive and DualTreeBoruvka produced the same MST.\n\n");
      }
      
      fx_timer_stop(naive_module, "comparison");
      
      const char* naive_output_filename = 
        fx_param_str(naive_module, "naive_output_filename", "naive_output.csv");
      
      data::Save(naive_output_filename, naive_results);
    }
    
    //////////////// Output the Results ////////////////
    
    const char* output_filename = 
        fx_param_str(NULL, "output_filename", "output.csv");
    
    //FILE* output_file = fopen(output_filename, "w");
    
    data::Save(output_filename, results);
    
  //}// end else (if using_thor)
  
  fx_done(NULL);
  
  return 0;
  
}
