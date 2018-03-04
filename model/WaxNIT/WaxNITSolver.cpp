#include "model/WaxNIT/WaxNITSolver.hpp"

using namespace std;
using namespace wax_nit;

WaxNITSolver::WaxNITSolver(WaxNIT* _model) : basic2d::Basic2dSolver<WaxNIT>(_model)
{
	poro.open("snaps/poro.dat", ofstream::out);
	T.open("snaps/t.dat", ofstream::out);
	P.open("snaps/P.dat", ofstream::out);
	S.open("snaps/S.dat", ofstream::out);
	qcells.open("snaps/q_cells.dat", ofstream::out);
	pvd.open("snaps/WaxNIT.pvd", ofstream::out);
	pvd << "<VTKFile type = \"Collection\" version = \"1.0\" byte_order = \"LittleEndian\" header_type = \"UInt64\">\n";
	pvd << "\t<Collection>\n";
	
	CHOP_MULT = 0.1;
	MAX_SAT_CHANGE = 0.1;

	CONV_W2 = 1.e-4;		CONV_VAR = 1.e-10;
	MAX_ITER = 20;

	const int strNum = var_size * model->cellsNum;
	ind_i = new int[stencil * var_size * strNum];
	ind_j = new int[stencil * var_size * strNum];
	cols = new int[strNum];
	a = new double[stencil * var_size * strNum];
	ind_rhs = new int[strNum];
	rhs = new double[strNum];
}
WaxNITSolver::~WaxNITSolver()
{
	delete[] ind_i, ind_j, ind_rhs;
	delete[] cols;
	delete[] a, rhs;

	poro.close();
	T.close();
	P.close();
	S.close();
	qcells.close();
	pvd << "\t</Collection>\n";
	pvd << "</VTKFile>\n";
	pvd.close();
}
void WaxNITSolver::writeData()
{
	double p = 0.0, s_w = 0.0, s_o = 0.0, s_g = 0.0, m = 0.0, temp = 0.0;

	qcells << cur_t * t_dim / 3600.0;

	map<int, double>::iterator it;
	for (it = model->Qcell.begin(); it != model->Qcell.end(); ++it)
	{
		const auto& cell = model->cells[it->first];
		m += cell.u_next.m;
		temp += (cell.u_next.t - cell.props->t_init) * model->T_dim;
		p += cell.u_next.p * model->P_dim;
		s_w += cell.u_next.s_w;
		s_o += cell.u_next.s_o;
		s_g += cell.u_next.s_g;
		if (model->leftBoundIsRate)
			qcells << "\t" << it->second * model->Q_dim * 86400.0;
		else
			qcells << "\t" << model->getRate(it->first) * model->Q_dim * 86400.0;
	}

	poro << cur_t * t_dim / 3600.0 <<
		"\t" << m / (double)(model->Qcell.size()) << endl;
	T << cur_t * t_dim / 3600.0 <<
		"\t" << temp / (double)(model->Qcell.size()) << endl;
	P << cur_t * t_dim / 3600.0 << 
		"\t" << p / (double)(model->Qcell.size()) << endl;
	S << cur_t * t_dim / 3600.0 << 
		"\t" << s_w / (double)(model->Qcell.size()) << 
		"\t" << s_o / (double)(model->Qcell.size()) <<
		"\t" << s_g / (double)(model->Qcell.size()) <<
		"\t" << (1.0 - s_w - s_o - s_g) / (double)(model->Qcell.size()) << endl;

	pvd << "\t\t<DataSet part=\"0\" timestep=\"" + to_string(cur_t) + 
			"0\" file=\"WaxNIT_" + to_string(step_idx) + ".vtp\"/>\n";
	qcells << endl;
}
void WaxNITSolver::start()
{
	step_idx = 0;
	iterations = 8;

	fillIndices();
	solver.Init(var_size * model->cellsNum, 1.e-15, 1.e-15);

	model->setPeriod(curTimePeriod);

	while (cur_t < Tt)
	{
		control();
		if (model->isWriteSnaps)
			model->snapshot_all(step_idx++);
		doNextStep();
		copyTimeLayer();
		cout << "---------------------NEW TIME STEP---------------------" << endl;
	}
	if (model->isWriteSnaps)
		model->snapshot_all(step_idx);
	writeData();
}
void WaxNITSolver::copySolution(const Vector& sol)
{
	for (int i = 0; i < model->cellsNum; i++)
	{
		Variable& next = model->cells[i].u_next;
		next.m += sol[i * var_size];
		next.t += sol[i * var_size + 1];
		next.p += sol[i * var_size + 2];
		next.s_w += sol[i * var_size + 3];
		
		if (next.satur_gas && next.satur_wax)
		{
			next.s_o += sol[i * var_size + 4];
			next.s_g += sol[i * var_size + 5];
			next.p_bub = next.p;
			next.t_bub = next.t;
		}
		else if (next.satur_gas)
		{
			next.s_o += sol[i * var_size + 4];
			next.p_bub = next.p;
			next.s_g -= (sol[i * var_size + 3] + sol[i * var_size + 4]);
			next.t_bub += sol[i * var_size + 5];
		}
		else if (next.satur_wax)
		{
			next.s_o -= (sol[i * var_size + 3] + sol[i * var_size + 5]);
			next.p_bub += sol[i * var_size + 4];
			next.s_g += sol[i * var_size + 5];
			next.t_bub = next.t;
		} 
		else
		{
			next.s_o -= sol[i * var_size + 3] / 2.0;
			next.p_bub += sol[i * var_size + 4];
			next.s_g -= sol[i * var_size + 3] / 2.0;
			next.t_bub += sol[i * var_size + 5];
		}
	}
}
void WaxNITSolver::checkStability()
{
	auto barelyMobilLeft = [this](double s_cur, double s_crit) -> double
	{
		return s_crit + fabs(s_cur - s_crit) * CHOP_MULT;
	};
	auto barelyMobilRight = [this](double s_cur, double s_crit) -> double
	{
		return s_crit - fabs(s_crit - s_cur) * CHOP_MULT;
	};
	auto checkBubPoint = [](auto& cell, auto& next)
	{
		if (next.satur_gas)
		{
			if (next.s_o + next.s_w + next.s_g > 1.0 + 1000.0 * EQUALITY_TOLERANCE)
			{
				double qwe = next.s_o + next.s_w + next.s_g;
				next.satur_gas = false;
				next.s_o = 1.0 - next.s_w;
				next.p_bub = 0.999 * cell.u_iter.p_bub;
			}
		}
		else
		{
			if (next.p_bub > next.p + 100.0 * EQUALITY_TOLERANCE)
			{
				next.satur_gas = true;
				next.s_o = 0.999 * cell.u_iter.s_o;
				next.p_bub = next.p;
			}
		}
		/*if (next.satur_wax)
		{
			if (next.s_o + next.s_w + next.s_g > 1.0 + EQUALITY_TOLERANCE)
			{
				next.satur_gas = false;
				next.s_o = 1.0 - next.s_w;
				next.p_bub = 0.999 * cell.u_iter.p_bub;
			}
		}*/
	};
	auto checkCritPoints = [=, this](auto& next, auto& iter, auto& props)
	{
		// Oil
		if ((next.s_o - props.s_oc) * (iter.s_o - props.s_oc) < 0.0)
			next.s_o = barelyMobilLeft(next.s_o, props.s_oc);
		if ((next.s_o - (1.0 - props.s_wc - props.s_gc)) * (iter.s_o - (1.0 - props.s_wc - props.s_gc)) < 0.0)
			next.s_o = barelyMobilRight(next.s_o, 1.0 - props.s_wc - props.s_gc);
		// Water
		if ((next.s_w - props.s_wc) * (iter.s_w - props.s_wc) < 0.0)
			next.s_w = barelyMobilLeft(next.s_w, props.s_wc);
		if ((next.s_w - (1.0 - props.s_oc - props.s_gc)) * (iter.s_w - (1.0 - props.s_oc - props.s_gc)) < 0.0)
			next.s_w = barelyMobilRight(next.s_w, 1.0 - props.s_oc - props.s_gc);
		// Gas
		if ((1.0 - next.s_w - next.s_o - props.s_gc) * (1.0 - iter.s_w - iter.s_o - props.s_gc) < 0.0)
			if (fabs(next.s_o - iter.s_o) > fabs(next.s_w - iter.s_w))
				next.s_o = 1.0 - next.s_w - barelyMobilLeft(1.0 - next.s_o - next.s_w, props.s_gc);
			else
				next.s_w = 1.0 - next.s_o - barelyMobilLeft(1.0 - next.s_o - next.s_w, props.s_gc);
		if ((props.s_wc - next.s_w + props.s_oc - next.s_o) * (props.s_wc - iter.s_w + props.s_oc - iter.s_o) < 0.0)
			if (fabs(next.s_o - iter.s_o) > fabs(next.s_w - iter.s_w))
				next.s_o = 1.0 - next.s_w - barelyMobilRight(1.0 - next.s_o - next.s_w, 1.0 - props.s_oc - props.s_wc);
			else
				next.s_w = 1.0 - next.s_o - barelyMobilRight(1.0 - next.s_o - next.s_w, 1.0 - props.s_oc - props.s_wc);
	};
	auto checkMaxResidual = [=, this](auto& next, auto& iter)
	{
		if (fabs(next.s_w - iter.s_w) > MAX_SAT_CHANGE)
			next.s_w = iter.s_w + sign(next.s_w - iter.s_w) * MAX_SAT_CHANGE;
		if (fabs(next.s_o - iter.s_o) > MAX_SAT_CHANGE)
			next.s_o = iter.s_o + sign(next.s_o - iter.s_o) * MAX_SAT_CHANGE;
		if (fabs(next.s_g - iter.s_g) > MAX_SAT_CHANGE)
			next.s_g = iter.s_g + sign(next.s_g - iter.s_g) * MAX_SAT_CHANGE;
	};

	for (auto& cell : model->cells)
	{
		const Skeleton_Props& props = *cell.props;
		Variable& next = cell.u_next;
		const Variable& iter = cell.u_iter;

		checkBubPoint(cell, next);
		checkCritPoints(next, iter, props);
		checkMaxResidual(next, iter);
	}
}
void WaxNITSolver::solveStep()
{
	int cellIdx, varIdx;
	err_newton = 1.0;
	averValue(averValPrev);
	std::fill(dAverVal.begin(), dAverVal.end(), 1.0);
	iterations = 0;

	auto continueIterations = [this]()
	{
		bool result = false;
		for (const auto& val : dAverVal)
			result += (val > CONV_VAR);

		return result * (err_newton > CONV_W2) * (iterations < MAX_ITER);
	};

	while (continueIterations())
	{
		copyIterLayer();

		//writeMatrixes();
		fill();
		solver.Assemble(ind_i, ind_j, a, elemNum, ind_rhs, rhs);
		solver.Solve(PRECOND::ILU_SERIOUS);
		copySolution(solver.getSolution());
		//Solve(model->cellsNum_r + 1, WaxNIT::var_size * (model->cellsNum_z + 2), PRES);
		//construction_from_fz(model->cellsNum_r + 2, WaxNIT::var_size * (model->cellsNum_z + 2), PRES);
		
		checkStability();
		err_newton = convergance(cellIdx, varIdx);
		
		averValue(averVal);
		for (int i = 0; i < var_size; i++)
			dAverVal[i] = fabs(averVal[i] - averValPrev[i]);
		averValPrev = averVal;

		iterations++;
	}

	cout << "Newton Iterations = " << iterations << "\t cur_t = " << cur_t << endl;
}

void WaxNITSolver::fillIndices()
{
	int pres_counter = 0;
	int col_idx = 0;

	for (const auto& cell : model->cells)
	{
		cols[col_idx] = 0;
		auto& pres_idx = getMatrixStencil(cell);

		for (int i = 0; i < var_size; i++)
		{
			const int str_idx = var_size * cell.num + i;
			for (const int idx : pres_idx)
			{
				for (int j = 0; j < var_size; j++)
				{
					ind_i[pres_counter] = str_idx;			ind_j[pres_counter++] = var_size * idx + j;
				}
			}
		}

		cols[col_idx++] += var_size * pres_idx.size();
		pres_idx.clear();
	}

	elemNum = pres_counter;

	for (int i = 0; i < model->cellsNum * var_size; i++)
		ind_rhs[i] = i;
}
void WaxNITSolver::fill()
{
	int counter = 0;
	int nebr_idx;

	for (const auto& cell : model->cells)
	{
		model->setVariables(cell);

		auto& mat_idx = getMatrixStencil(cell);
		for (int i = 0; i < var_size; i++)
		{
			const int str_idx = var_size * cell.num + i;
			nebr_idx = 0;
			for (const int idx : mat_idx)
			{
				a[counter++] = model->jac[i][size * nebr_idx];
				a[counter++] = model->jac[i][size * nebr_idx + 1];
				a[counter++] = model->jac[i][size * nebr_idx + 2];
				a[counter++] = model->jac[i][size * nebr_idx + 3];
				if (cell.u_next.satur_gas)
					a[counter++] = model->jac[i][size * nebr_idx + 4];
				else
					a[counter++] = model->jac[i][size * nebr_idx + 6];
				if (cell.u_next.satur_wax)
					a[counter++] = model->jac[i][size * nebr_idx + 5];
				else
					a[counter++] = model->jac[i][size * nebr_idx + 7];

				nebr_idx++;
			}

			rhs[str_idx] = -model->y[i];
		}
		mat_idx.clear();
	}

}

/*void WaxNITSolver::construction_from_fz(int N, int n, int key)
{
	vector<Cell>::iterator it;
	if (key == PRES)
	{
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < model->cellsNum_z + 2; j++)
			{
				Variable& next = model->cells[i*(model->cellsNum_z + 2) + j].u_next;
				next.m += fz[i][var_size * j + 1];
				next.t += fz[i][var_size * j + 2];
				next.p += fz[i][var_size * j + 3];
				next.s_w += fz[i][var_size * j + 4];
				if (next.SATUR)
				{
					next.s_o += fz[i][var_size * j + 5];
					next.p_bub = next.p;
				}
				else
				{
					next.s_o -= fz[i][var_size * j + 4];
					next.p_bub += fz[i][var_size * j + 5];
				}
			}
		}
	}
}
void WaxNITSolver::MiddleAppr(int current, int MZ, int key)
{
	for (int i = 0; i < MZ; i++)
	{
		for (int j = 0; j < MZ; j++)
		{
			A[i][j] = 0.0;
			B[i][j] = 0.0;
			C[i][j] = 0.0;
		}
		RightSide[i][0] = 0.0;
	}

	if (key == PRES)
	{
		int idx = 0;
		int cell_idx = current * (model->cellsNum_z + 2);

		// Top cell
		model->setVariables(model->cells[cell_idx]);

		for (int i = 0; i < var_size; i++)
		{
			RightSide[idx + i][0] = -model->y[i];
			for (int j = 0; j < var_size - 1; j++)
			{
				B[idx + i][idx + j] = model->jac[i][j];
				B[idx + i][idx + j + var_size] = model->jac[i][j + size];
			}
			// s_o / p_bub
			if (model->cells[cell_idx].u_next.SATUR)
				B[idx + i][idx + var_size - 1] = model->jac[i][var_size - 1];
			else
				B[idx + i][idx + var_size - 1] = model->jac[i][var_size];
			if (model->cells[cell_idx + 1].u_next.SATUR)
				B[idx + i][idx + var_size - 1 + var_size] = model->jac[i][size + var_size - 1];
			else
				B[idx + i][idx + var_size - 1 + var_size] = model->jac[i][size + var_size];
		}
		idx += var_size;

		// Middle cells
		for (cell_idx = current * (model->cellsNum_z + 2) + 1; cell_idx < (current + 1) * (model->cellsNum_z + 2) - 1; cell_idx++)
		{
			model->setVariables(model->cells[cell_idx]);

			for (int i = 0; i < var_size; i++)
			{
				RightSide[idx + i][0] = -model->y[i];
				for (int j = 0; j < var_size - 1; j++)
				{
					B[idx + i][idx + j] = model->jac[i][j];
					C[idx + i][idx + j] = model->jac[i][j + size];
					A[idx + i][idx + j] = model->jac[i][j + size * 2];
					B[idx + i][idx + j - var_size] = model->jac[i][j + size * 3];
					B[idx + i][idx + j + var_size] = model->jac[i][j + size * 4];
				}
				// s_o / p_bub
				if (model->cells[cell_idx].u_next.SATUR)
					B[idx + i][idx + var_size - 1] = model->jac[i][var_size - 1];
				else
					B[idx + i][idx + var_size - 1] = model->jac[i][var_size];
				if (model->cells[cell_idx - model->cellsNum_z - 2].u_next.SATUR)
					C[idx + i][idx + var_size - 1] = model->jac[i][size + var_size - 1];
				else
					C[idx + i][idx + var_size - 1] = model->jac[i][size + var_size];
				if (model->cells[cell_idx + model->cellsNum_z + 2].u_next.SATUR)
					A[idx + i][idx + var_size - 1] = model->jac[i][size * 2 + var_size - 1];
				else
					A[idx + i][idx + var_size - 1] = model->jac[i][size * 2 + var_size];
				if (model->cells[cell_idx - 1].u_next.SATUR)
					B[idx + i][idx + var_size - 1 - var_size] = model->jac[i][size * 3 + var_size - 1];
				else
					B[idx + i][idx + var_size - 1 - var_size] = model->jac[i][size * 3 + var_size];
				if (model->cells[cell_idx + 1].u_next.SATUR)
					B[idx + i][idx + var_size - 1 + var_size] = model->jac[i][size * 4 + var_size - 1];
				else
					B[idx + i][idx + var_size - 1 + var_size] = model->jac[i][size * 4 + var_size];
			}

			idx += var_size;
		}

		// Bottom cell
		model->setVariables(model->cells[cell_idx]);

		for (int i = 0; i < var_size; i++)
		{
			RightSide[idx + i][0] = -model->y[i];
			for (int j = 0; j < var_size - 1; j++)
			{
				B[idx + i][idx + j] = model->jac[i][j];
				B[idx + i][idx + j - var_size] = model->jac[i][j + size];
			}
			// s_o / p_bub
			if (model->cells[cell_idx].u_next.SATUR)
				B[idx + i][idx + var_size - 1] = model->jac[i][var_size - 1];
			else
				B[idx + i][idx + var_size - 1] = model->jac[i][var_size];
			if (model->cells[cell_idx - 1].u_next.SATUR)
				B[idx + i][idx + var_size - 1 - var_size] = model->jac[i][Variable::size + var_size - 1];
			else
				B[idx + i][idx + var_size - 1 - var_size] = model->jac[i][Variable::size + var_size];
		}
	}

	construction_bz(MZ, 2);
}
void WaxNITSolver::LeftBoundAppr(int MZ, int key)
{
	for (int i = 0; i < MZ; i++)
	{
		for (int j = 0; j < MZ; j++)
		{
			C[i][j] = 0.0;
			B[i][j] = 0.0;
			A[i][j] = 0.0;
		}
		C[i][i] = 1.0;
		B[i][i] = -1.0;

		const Variable& next = model->cells[int(i / var_size)].u_next;
		const Variable& nebr = model->cells[int(i / var_size) + model->cellsNum_z + 2].u_next;

		if (i % var_size == var_size - 1)
			RightSide[i][0] = -next.values[var_size - 1 + !next.SATUR] + nebr.values[var_size - 1 + !nebr.SATUR];
		else
			RightSide[i][0] = -next.values[i % var_size] + nebr.values[i % var_size];
	}

	map<int, double>::iterator it;
	int idx;
	if (key == PRES)
	{
		for (it = model->Qcell.begin(); it != model->Qcell.end(); ++it)
		{
			idx = var_size * it->first;
			model->setVariables(model->cells[it->first]);

			for (int i = 0; i < var_size; i++)
			{
				RightSide[idx + i][0] = -model->y[i];
				for (int j = 0; j < var_size - 1; j++)
				{
					C[idx + i][idx + j] = model->jac[i][j];
					B[idx + i][idx + j] = model->jac[i][j + size];
					A[idx + i][idx + j] = model->jac[i][j + 2 * size];
				}
				// s_o / p_bub
				if (model->cells[it->first].u_next.SATUR)
					C[idx + i][idx + var_size - 1] = model->jac[i][var_size - 1];
				else
					C[idx + i][idx + var_size - 1] = model->jac[i][var_size];
				if (model->cells[it->first + model->cellsNum_z + 2].u_next.SATUR)
					B[idx + i][idx + var_size - 1] = model->jac[i][size + var_size - 1];
				else
					B[idx + i][idx + var_size - 1] = model->jac[i][size + var_size];
				if (model->cells[it->first + 2 * model->cellsNum_z + 4].u_next.SATUR)
					A[idx + i][idx + var_size - 1] = model->jac[i][2 * size + var_size - 1];
				else
					A[idx + i][idx + var_size - 1] = model->jac[i][2 * size + var_size];
			}
		}
	}

	construction_bz(MZ, 2);
}
void WaxNITSolver::RightBoundAppr(int MZ, int key)
{
	for (int i = 0; i < MZ; i++)
	{
		for (int j = 0; j < MZ; j++)
		{
			C[i][j] = 0.0;
			B[i][j] = 0.0;
			A[i][j] = 0.0;
		}
		RightSide[i][0] = 0.0;
	}

	if (key == PRES)
	{
		int idx = 0;

		for (int cell_idx = (model->cellsNum_r + 1)*(model->cellsNum_z + 2); cell_idx < (model->cellsNum_r + 2)*(model->cellsNum_z + 2); cell_idx++)
		{
			model->setVariables(model->cells[cell_idx]);

			for (int i = 0; i < var_size; i++)
			{
				RightSide[idx + i][0] = -model->y[i];
				for (int j = 0; j < var_size - 1; j++)
				{
					A[idx + i][idx + j] = model->jac[i][j];
					B[idx + i][idx + j] = model->jac[i][j + size];
				}
				// s_o / p_bub
				if (model->cells[cell_idx].u_next.SATUR)
					A[idx + i][idx + var_size - 1] = model->jac[i][var_size - 1];
				else
					A[idx + i][idx + var_size - 1] = model->jac[i][var_size];
				if (model->cells[cell_idx - model->cellsNum_z - 2].u_next.SATUR)
					B[idx + i][idx + var_size - 1] = model->jac[i][size + var_size - 1];
				else
					B[idx + i][idx + var_size - 1] = model->jac[i][size + var_size];
			}

			idx += var_size;
		}
	}

	construction_bz(MZ, 1);
}
void WaxNITSolver::writeMatrixes()
{
	const int MZ = (model->cellsNum_z + 2) * var_size;

	// Left
	mat_a.open("snaps/a_left.dat", ofstream::out);
	mat_b.open("snaps/b_left.dat", ofstream::out);
	mat_c.open("snaps/c_left.dat", ofstream::out);
	rhs_os.open("snaps/rhs_left.dat", ofstream::out);

	LeftBoundAppr(MZ, PRES);
	for (int i = 0; i < MZ; i++)
	{
		for (int j = 0; j < MZ; j++)
		{
			mat_a << i << "\t" << j << "\t" << A[i][j] << endl;
			mat_b << i << "\t" << j << "\t" << B[i][j] << endl;
			mat_c << i << "\t" << j << "\t" << C[i][j] << endl;
		}
		rhs_os << i << "\t" << RightSide[i][0] << endl;
	}
	mat_a.close();		mat_b.close();		mat_c.close();		rhs_os.close();

	// Middle
	for (int k = 0; k < model->cellsNum_r; k++)
	{
		const string afilename = "snaps/a_" + to_string(k + 1) + ".dat";
		const string bfilename = "snaps/b_" + to_string(k + 1) + ".dat";
		const string cfilename = "snaps/c_" + to_string(k + 1) + ".dat";
		const string rhsfilename = "snaps/rhs_" + to_string(k + 1) + ".dat";
		mat_a.open(afilename.c_str(), ofstream::out);
		mat_b.open(bfilename.c_str(), ofstream::out);
		mat_c.open(cfilename.c_str(), ofstream::out);
		rhs_os.open(rhsfilename.c_str(), ofstream::out);

		MiddleAppr(k + 1, MZ, PRES);
		for (int i = 0; i < MZ; i++)
		{
			for (int j = 0; j < MZ; j++)
			{
				mat_a << i << "\t" << j << "\t" << A[i][j] << endl;
				mat_b << i << "\t" << j << "\t" << B[i][j] << endl;
				mat_c << i << "\t" << j << "\t" << C[i][j] << endl;
			}
			rhs_os << i << "\t" << RightSide[i][0] << endl;
		}

		mat_a.close();		mat_b.close();		mat_c.close();		rhs_os.close();
	}

	// Right
	mat_a.open("snaps/a_right.dat", ofstream::out);
	mat_b.open("snaps/b_right.dat", ofstream::out);
	mat_c.open("snaps/c_right.dat", ofstream::out);
	rhs_os.open("snaps/rhs_right.dat", ofstream::out);

	RightBoundAppr(MZ, PRES);
	for (int i = 0; i < MZ; i++)
	{
		for (int j = 0; j < MZ; j++)
		{
			mat_a << i << "\t" << j << "\t" << A[i][j] << endl;
			mat_b << i << "\t" << j << "\t" << B[i][j] << endl;
			mat_c << i << "\t" << j << "\t" << C[i][j] << endl;
		}
		rhs_os << i << "\t" << RightSide[i][0] << endl;
	}
	mat_a.close();		mat_b.close();		mat_c.close();		rhs_os.close();
};*/