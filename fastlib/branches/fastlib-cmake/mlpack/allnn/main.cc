/* MLPACK 0.2
 *
 * Copyright (c) 2008, 2009 Alexander Gray,
 *                          Garry Boyer,
 *                          Ryan Riegel,
 *                          Nikolaos Vasiloglou,
 *                          Dongryeol Lee,
 *                          Chip Mappus, 
 *                          Nishant Mehta,
 *                          Hua Ouyang,
 *                          Parikshit Ram,
 *                          Long Tran,
 *                          Wee Chin Wong
 *
 * Copyright (c) 2008, 2009 Georgia Institute of Technology
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * =====================================================================================
 *
 *       Filename:  main.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/14/2008 07:15:55 PM EDT
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 *
 * =====================================================================================
 */

#include <string>
#include "allnn.h"

int main (int argc, char *argv[]) {
  fx_module *fx_root=fx_init(argc, argv, NULL);
  AllNN allnn;
  Matrix data_for_tree;
   std::string filename;

  //std::string filename=fx_param_str_req(fx_root, "file");

  boost_po::options_description desc("Allowed options");
  desc.add_options()
      ("leaf_size", boost_po::value<int>()->default_value(20), "  The maximum number of points to store at a leaf.\n")
      ("file", boost_po::value<std::string>(), "  The reference file name.\n");

  boost_po::store(boost_po::parse_command_line(argc, argv, desc), vm);
  boost_po::notify(vm);

  if( 0 == vm.count("file")) {
     cerr << "Required parameter leaf_size not entered" << endl;
     exit(1);
  }
  NOTIFY("Loading file...");
  data::Load(filename.c_str(), &data_for_tree);
  NOTIFY("File loaded...");
  allnn.Init(data_for_tree, fx_root);
  //GenVector<index_t> resulting_neighbors_tree;
  //GenVector<double> resulting_distances_tree;
  NOTIFY("Computing Neighbors...");
  allnn.ComputeNeighbors(NULL, NULL);
  NOTIFY("Neighbors Computed...");
  fx_done(fx_root);
}
