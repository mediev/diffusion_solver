#ifndef HYPREINTERFACE_HPP_
#define HYPREINTERFACE_HPP_

#include <mpi.h>
#include "_hypre_utilities.h"
#include "HYPRE_krylov.h"
#include "HYPRE.h"
#include "HYPRE_parcsr_ls.h"

class HypreSolver
{
public:
	typedef double* Vector;
protected:
	HYPRE_IJMatrix ij_matrix;
	HYPRE_ParCSRMatrix parcsr_matrix;
	HYPRE_IJVector b;
	HYPRE_ParVector par_b;
	HYPRE_IJVector x;
	HYPRE_ParVector par_x;
	HYPRE_Solver solver, precond;
private:
	int size, rank;
	int ilower, iupper, nrows;
	double *x_values, *x_sol;
	int* rows;
	double absTol, relTol, dropTol;

	bool first;
	int res;
public:

	HypreSolver();
	~HypreSolver();

	void Init(const int vecSize, const double _absTol, const double _relTol, const double _dropTol);
	void Assemble(int* cols, const int* ind_i, const int* ind_j, const double* a, const int counter, const int* ind_rhs, const double* rhs);
	void Solve();
	const Vector& getSolution();

	int itersNum;
	double init_res, final_res;
};

#endif /* HYPREINTERFACE_HPP_ */
