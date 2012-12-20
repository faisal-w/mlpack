/**
 * @file lsh_test.cpp
 *
 * Unit tests for the 'LSHSearch' class.
 */
#include <mlpack/core.hpp>
#include <mlpack/core/metrics/lmetric.hpp>

#include "lsh_search.hpp"

using namespace std;
using namespace mlpack;
using namespace mlpack::neighbor;

PROGRAM_INFO("LSH test", " ");


int main (int argc, char *argv[])
{
  CLI::ParseCommandLine(argc, argv);
  math::RandomSeed(0);

  arma::mat rdata(2, 10);
  rdata << 3 << 2 << 4 << 3 << 5 << 6 << 0 << 8 << 3 << 1 << arma::endr << 
    0 << 3 << 4 << 7 << 8 << 4 << 1 << 0 << 4 << 3 << arma::endr;


  // Randomness present here -- seed = 0
  // Computing the hashwidth here.
  // CORRECT ANSWER: 'hashWidth' = 4.24777
  double hashWidth = 0;
  for (size_t i = 0; i < 10; i++)
  {
    size_t p1 = (size_t) math::RandInt(rdata.n_cols);
    size_t p2 = (size_t) math::RandInt(rdata.n_cols);

    hashWidth += metric::EuclideanDistance::Evaluate(rdata.unsafe_col(p1),
                                                     rdata.unsafe_col(p2));
  }
  hashWidth /= 10.0;

  Log::Info << "Hash width: " << hashWidth << endl;

  arma::mat qdata(2, 3);
  qdata << 3 << 2 << 0 << arma::endr << 5 << 3 << 4 << arma::endr;

  // INPUT TO LSH:
  // Number of points: 10
  // Number of dimensions: 2
  // Number of projections per table: 'numProj' = 3
  // Number of hash tables: 'numTables' = 2
  // hashWidth (computed): 'hashWidth' = 4.24777
  // Second hash size: 'secondHashSize' = 11
  // Size of the bucket: 'bucketSize' = 3

  // Randomness present in LSH -- seed = 0
  // Things obtained by random sampling listed in the sequences 
  // as they will be obtained in the 'LSHSearch::BuildHash()' private function 
  // in 'LSHSearch' class.
  //
  // 1. The weights of the second hash obtained as:
  //    secondHashWeights = arma::floor(arma::randu(3) * 11.0);
  //    COR.SOL.: secondHashWeights = [9, 4, 8];
  //
  // 2. The offsets for all the 3 projections in each of the 2 tables:
  //    offsets.randu(3, 2)
  //    COR.SOL.: [0.7984 0.3352; 0.9116 0.7682; 0.1976 0.2778]
  //    offsets *= hashWidth
  //    COR.SOL.: [3.3916 1.4240; 3.8725 3.2633; 0.8392 1.1799]
  //
  // 3. The  (2 x 3) projection matrices for the 2 tables:
  //    projMat.randn(2, 3)
  //    COR.SOL.: Proj. Mat 1: [2.7020 0.0187 0.4355; 1.3692 0.6933 0.0416]
  //    COR.SOL.: Proj. Mat 2: [-0.3961 -0.2666 1.1001; 0.3895 -1.5118 -1.3964]
  LSHSearch<> *lsh_test = new LSHSearch<>(rdata, qdata, 3,2, hashWidth, 11,3);

  // Given this, the 'LSHSearch::bucketRowInHashTable' should be:
  // COR.SOL.: [2 11 4 7 6 3 11 0 5 1 8]
  //
  // The 'LSHSearch::bucketContentSize' should be:
  // COR.SOL.: [2 0 1 1 3 1 0 3 3 3 1]
  // 
  // The final hash table 'LSHSearch::secondHashTable' should be 
  // of size (3 x 9) with the following content: 
  // COR.SOL.: 
  // [0 2 4; 1 7 8; 3 9 10; 5 10 10; 6 10 10; 0 5 6; 1 2 8; 3 10 10; 4 10 10]

  arma::Mat<size_t> neighbors;
  arma::mat distances;

  lsh_test->Search(2, neighbors, distances);

  // The private function 'LSHSearch::ReturnIndicesFromTable(0, refInds)'
  // should hash the query 0 into the following buckets:
  // COR.SOL.: Table 1 Bucket 7, Table 2 Bucket 0, refInds = [0 2 3 4 9]
  //
  // The private function 'LSHSearch::ReturnIndicesFromTable(1, refInds)'
  // should hash the query 1 into the following buckets:
  // COR.SOL.: Table 1 Bucket 9, Table 2 Bucket 4, refInds = [1 2 7 8]
  //
  // The private function 'LSHSearch::ReturnIndicesFromTable(2, refInds)'
  // should hash the query 2 into the following buckets:
  // COR.SOL.: Table 1 Bucket 0, Table 2 Bucket 7, refInds = [0 2 3 4 9]

  // After search
  // COR.SOL.: 'neighbors' = [2 1 9; 3 8 2]
  // COR.SOL.: 'distances' = [2 0 2; 4 2 16]

//   Log::Info << "Neighbors: " << std::endl << neighbors << std::endl <<
//     "Distances: " << std::endl << distances << std::endl;

  arma::Mat<size_t> true_neighbors(2, 3);
  true_neighbors << 2 << 1 << 9 << arma::endr << 3 << 8 << 2 << arma::endr;
  arma::mat true_distances(2, 3);
  true_distances << 2 << 0 << 2 << arma::endr << 4 << 2 << 16 << arma::endr;

  for (size_t i = 0; i < 3; i++)
  {
    for (size_t j = 0; j < 2; j++)
    {
      assert(neighbors(j, i) == true_neighbors(j, i));
      assert(distances(j, i) == true_distances(j, i));
    }
  }

  Log::Warn << "Expected neighbor results obtained!!" << std::endl;

  delete lsh_test;

  return 0;
}
