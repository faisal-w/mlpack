function f = ComputeGaussianSparseCodesObjective(D, s, Dt_t, lambda)
%function f = ComputeGaussianSparseCodesObjective(D, s, Dt_t, lambda)
%
% Compute the relevant part of the objective sparse codes in Poisson sparse coding
% Given:
%   Dictionary D
%   Candidate sparse code s
%   Dt_t is D transpose times t, for t the sufficient statistic for the point
%   l1-norm regularization parameter lambda 

f = -s' * Dt_t;

f = f + 0.5 * norm(D * s)^2;

f = f + lambda * sum(abs(s));
