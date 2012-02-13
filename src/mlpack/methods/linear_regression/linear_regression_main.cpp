/**
 * @file linear_regression_main.cpp
 * @author James Cline
 *
 * Main function for least-squares linear regression.
 */
#include <mlpack/core.hpp>
#include "linear_regression.hpp"

PROGRAM_INFO("Simple Linear Regression Prediction",
    "An implementation of simple linear regression using ordinary least "
    "squares. This solves the problem\n\n"
    "  y = X * b + e\n\n"
    "where X (--input_file) and y (the last row of --input_file, or "
    "--input_responses) are known and b is the desired variable.  The "
    "calculated b is saved to disk (--output_file).\n"
    "\n"
    "Optionally, the calculated value of b is used to predict the responses for"
    " another matrix X' (--test_file):\n\n"
    "   y' = X' * b\n\n"
    "and these predicted responses, y', are saved to a file "
    "(--output_predictions).");

PARAM_STRING("input_file", "File containing X (regressors).", "i", "");
PARAM_STRING("input_responses", "Optional file containing y (responses). If "
    "not given, the responses are assumed to be the last row of the input "
    "file.", "r", "");

PARAM_STRING("model_file", "File containing existing model (parameters).", "m",
    "");

PARAM_STRING("output_file", "File where parameters (b) will be saved.",
    "o", "parameters.csv");

PARAM_STRING("test_file", "File containing X' (test regressors).", "t", "");
PARAM_STRING("output_predictions", "If --test_file is specified, this file is "
    "where the predicted responses will be saved.", "p", "predictions.csv");

using namespace mlpack;
using namespace mlpack::regression;
using namespace arma;
using namespace std;

int main(int argc, char* argv[])
{
  // Handle parameters
  CLI::ParseCommandLine(argc, argv);

  const string modelName = CLI::GetParam<string>("model_file");
  const string outputFile = CLI::GetParam<string>("output_file");
  const string outputPredictions = CLI::GetParam<string>("output_predictions");
  const string responseName = CLI::GetParam<string>("input_responses");
  const string testName = CLI::GetParam<string>("test_file");
  const string trainName = CLI::GetParam<string>("input_file");

  mat regressors;
  mat responses;

  LinearRegression lr;

  bool computeModel;

  if (trainName.empty())
  {
    if (modelName.empty())
    {
      Log::Fatal << "You must specify either --input_file or --model_file." << std::endl;
      exit(1);
    }
    else
    {
      computeModel = false;
    }
  }
  else if (modelName.empty())
  {
    computeModel = true;
  }
  else
  {
      Log::Fatal << "You must specify either --input_file or --model_file, not both." << std::endl;
      exit(1);
  }

  if(!computeModel && testName.empty())
  {
    Log::Fatal << "When specifying --model_file, you must also specify --test_file." << std::endl;
    exit(1);
  }

  if (computeModel)
  {
    Timer::Start("load_regressors");
    data::Load(trainName.c_str(), regressors, true);
    Timer::Stop("load_regressors");

    // Are the responses in a separate file?
    if (responseName.empty())
    {
      // The initial predictors for y, Nx1
      responses = trans(regressors.row(regressors.n_rows - 1));
      regressors.shed_row(regressors.n_rows - 1);
    }
    else
    {
      // The initial predictors for y, Nx1
      Timer::Start("load_responses");
      data::Load(responseName.c_str(), responses, true);
      Timer::Stop("load_responses");

      if (responses.n_rows == 1)
        responses = trans(responses); // Probably loaded backwards, but that's ok.

      if (responses.n_cols > 1)
        Log::Fatal << "The responses must have one column.\n";

      if (responses.n_rows != regressors.n_cols)
        Log::Fatal << "The responses must have the same number of rows as the "
            "training file.\n";
    }

    Timer::Start("regression");
    lr = LinearRegression(regressors, responses.unsafe_col(0));
    Timer::Stop("regression");

    // Save the parameters.
    data::Save(outputFile.c_str(), lr.Parameters(), true);
  }

  // Did we want to predict, too?
  if (!testName.empty() )
  {

    if (!computeModel)
    {
      Timer::Start("load_model");
      lr = LinearRegression(modelName);
      Timer::Stop("load_model");
    }

    arma::mat points;
    Timer::Stop("load_test_points");
    data::Load(testName.c_str(), points, true);
    Timer::Stop("load_test_points");

    arma::vec predictions;
    Timer::Start("prediction");
    lr.Predict(points, predictions);
    Timer::Stop("prediction");

    // Save predictions.
    predictions = arma::trans(predictions);
    data::Save(outputPredictions.c_str(), predictions, true);
  }
}
