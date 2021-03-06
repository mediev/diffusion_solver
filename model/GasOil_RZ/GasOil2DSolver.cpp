#include "model/GasOil_RZ/GasOil2DSolver.h"

using namespace std;
using namespace gasOil_rz;

GasOil2DSolver::GasOil2DSolver(GasOil_RZ* _model) : AbstractSolver<GasOil_RZ>(_model)
{
	Initialize(model->cellsNum_r+2, 2*(model->cellsNum_z+2));

	plot_Pdyn.open("snaps/P_dyn.dat", ofstream::out);
	plot_Sdyn.open("snaps/S_dyn.dat", ofstream::out);
	plot_qcells.open("snaps/q_cells.dat", ofstream::out);

	t_dim = model->t_dim;

	n = model->Qcell.size();
	dq.Initialize(n);
	q.Initialize(n);

	dpdq.Initialize(n, n-1);
	mat.Initialize(n-1, n-1);
	b.Initialize(n-1);

	Tt = model->period[model->period.size()-1];
}

GasOil2DSolver::~GasOil2DSolver()
{
	plot_Pdyn.close();
	plot_Sdyn.close();
	plot_qcells.close();
}

void GasOil2DSolver::writeData()
{
	double p = 0.0, s = 0.0;

	plot_qcells << cur_t * t_dim / 3600.0;

	map<int,double>::iterator it;
	for(it = model->Qcell.begin(); it != model->Qcell.end(); ++it)
	{
		p += model->cells[it->first].u_next.p;
		s += model->cells[it->first].u_next.s;
		if( model->leftBoundIsRate )
			plot_qcells << "\t" << it->second * model->Q_dim * 86400.0;
		else
			plot_qcells << "\t" << model->getRate(it->first) * model->Q_dim * 86400.0;
	}

	plot_Pdyn << cur_t * t_dim / 3600.0 << "\t" << p / (double)(model->Qcell.size()) << endl;
	plot_Sdyn << cur_t * t_dim / 3600.0 << "\t" << s / (double)(model->Qcell.size()) << endl;

	plot_qcells << endl;
}

void GasOil2DSolver::control()
{
	writeData();

	if(cur_t >= model->period[curTimePeriod])
	{
		curTimePeriod++;
		model->ht = model->ht_min;
		model->setPeriod(curTimePeriod);
	}

	if(model->ht <= model->ht_max && iterations < 6)
		model->ht = model->ht * 1.5;
	else if(iterations > 6 && model->ht > model->ht_min)
		model->ht = model->ht / 1.5;

	if(cur_t + model->ht > model->period[curTimePeriod])
		model->ht = model->period[curTimePeriod] - cur_t;

	cur_t += model->ht;
}

void GasOil2DSolver::doNextStep()
{
	solveStep();

	if(n > 1 && model->Q_sum > EQUALITY_TOLERANCE)
	{
		double H0 = fabs( model->solveH() );
		if( H0 > 0.1 )
		{
			printWellRates();
			fillq();

			double mult = 0.9;
			double H = H0;			

			while(H > H0 / 50.0 || H > 0.05)
			{
				solveDq(mult);

				int i = 0;
				map<int,double>::iterator it = model->Qcell.begin();

				while(it != model->Qcell.end())
				{
					q[i] += mult * dq[i];
					it->second = q[i];
					i++;	++it;
				}
				
				solveStep();
				printWellRates();

				H = fabs( model->solveH() );
			}
		}
	}
}

void GasOil2DSolver::fillq()
{
	int i = 0;
	map<int,double>::iterator it = model->Qcell.begin();
	while(it != model->Qcell.end())
	{
		q[i++] = it->second;
		++it;
	}
}

void GasOil2DSolver::fillDq()
{
	for(int i = 0; i < n; i++)
		dq[i] = 0.0;
}

void GasOil2DSolver::solveDq(double mult)
{
	fillDq();
	filldPdQ(mult);
	solveStep();
	solveSystem();

	int i = 0;
	map<int,double>::iterator it;
	for(it = model->Qcell.begin(); it != model->Qcell.end(); ++it)
	{
		cout << "dq[" << it->first << "] = " << dq[i++] * model->Q_dim * 86400.0 << endl;
	}
	cout << endl;
}

void GasOil2DSolver::solveSystem()
{
	double s = 0.0, p1, p2;
	map<int,double>::iterator it;

	for(int i = 0; i < n-1; i++)
	{
		for(int j = 0; j < n-1; j++)
		{
			s = 0.0;
			for(int k = 0; k < n-1; k++)
				s += ( dpdq[k+1][j] - dpdq[k][j] ) * ( dpdq[k+1][i] - dpdq[k][i] );
			mat[i][j] = s;
		}
		
		s = 0.0;
		it = model->Qcell.begin();
		for(int k = 0; k < n-1; k++)
		{
			p1 = model->cells[ it->first ].u_next.p;
			p2 = model->cells[ (++it)->first ].u_next.p;
			s += ( p2 - p1 ) * ( dpdq[k+1][i] - dpdq[k][i] );
		}
		b[i] = -s;
	}

	MC_LU rateSystem(mat, b);
	rateSystem.LU_Solve();

	s = 0.0;
	for(int i = 0; i < n-1; i++)
	{
		dq[i+1] = rateSystem.ptResult[i];
		s += rateSystem.ptResult[i];
	}
	dq[0] = - s;
}

void GasOil2DSolver::filldPdQ(double mult)
{
	double p1, p2, ratio;
	ratio = mult * 0.001 / (double)(n);
	
	int i = 0, j = 0;
	map<int,double>::iterator it0 = model->Qcell.begin();
	map<int,double>::iterator it1 = model->Qcell.begin();
	map<int,double>::iterator it2 = it1;	++it2;
	while(it1 != model->Qcell.end())
	{
		j = 0;
		it2 = model->Qcell.begin();		++it2;
		while(it2 != model->Qcell.end())
		{
			model->setRateDeviation(it2->first, -ratio);
			model->setRateDeviation(it0->first, ratio);
			solveStep();
			p1 = model->cells[ it1->first ].u_next.p;

			model->setRateDeviation(it2->first, 2.0 * ratio);
			model->setRateDeviation(it0->first, -2.0 * ratio);
			solveStep();
			p2 = model->cells[ it1->first ].u_next.p;

			model->setRateDeviation(it2->first, -ratio);
			model->setRateDeviation(it0->first, ratio);

			dpdq[i][j++] = (p2 - p1) / ( 2.0 * ratio * model->Q_sum);

			++it2;
		}
		i++;
		++it1;
	}
}

/*void GasOil2DSolver::fillGrad(double mult)
{
	// Fill gradient
	map<int,double>::iterator it;
	double H1, H2, ratio;
	ratio = mult * 0.001 / (double)(n);
	int i = 0;
	for(it = model->Qcell.begin(); it != model->Qcell.end(); ++it)
	{
		model->setRateDeviation(it->first, -ratio);
		solveStep();
		H1 = model->solveH();
		
		model->setRateDeviation(it->first, 2.0 * ratio);
		solveStep();
		H2 = model->solveH();
		
		model->setRateDeviation(it->first, -ratio);

		gradH[i++] = (H1 - H2) / ( 2.0 * ratio * model->Q_sum );
	}

	gradH[i] = -model->solveRateDiff();
}

void GasOil2DSolver::fillGess()
{
	// Fill gessian
	int i = 0;
	int j = 0;
	double H1, H2, H3, H4, ratio, ratioLambda;
	double lambda0 = model->Lambda;
	ratio = 0.1 / (double)(n);
	ratioLambda = 0.1;
	map<int,double>::iterator it1, it2;

	for(it1 = model->Qcell.begin(); it1 != model->Qcell.end(); ++it1)
	{
		j = i;
		for(it2 = it1; it2 != model->Qcell.end(); ++it2)
		{
			model->setRateDeviation(it1->first, -ratio);	
			model->setRateDeviation(it2->first, -ratio);
			solveStep();
			H1 = model->solveH();
			
			model->setRateDeviation(it2->first, 2.0 * ratio);
			solveStep();
			H2 = model->solveH();

			model->setRateDeviation(it1->first, 2.0 * ratio);
			solveStep();
			H4 = model->solveH();

			model->setRateDeviation(it2->first, -2.0 * ratio);
			solveStep();
			H3 = model->solveH();

			model->setRateDeviation(it1->first, -ratio);
			model->setRateDeviation(it2->first, ratio);

			gessH[i][j++] = (H4 - H3 - H2 + H1) / ( 4.0 * ratio * model->Q_sum * ratio * model->Q_sum );
		}
		
		model->setRateDeviation(it1->first, -ratio);
		solveStep();
		H1 = model->solveRateDiff();
			
		model->setRateDeviation(it1->first, 2.0 * ratio);
		solveStep();
		H2 = model->solveRateDiff();

		gessH[i][j] = (H2 - H1) / ( 2.0 * ratio * model->Q_sum);

		i++;
	}
	
	j = 0;
	for(it2 = model->Qcell.begin(); it2 != model->Qcell.end(); ++it2)
	{
		model->Lambda = (1.0 - ratioLambda) * lambda0;
		model->setRateDeviation(it2->first, -ratio);
		solveStep();
		H1 = model->solveH();
			
		model->setRateDeviation(it2->first, 2.0 * ratio);
		solveStep();
		H2 = model->solveH();

		model->Lambda = (1.0 + ratioLambda) * lambda0;
		solveStep();
		H4 = model->solveH();

		model->setRateDeviation(it2->first, -2.0 * ratio);
		solveStep();
		H3 = model->solveH();

		model->Lambda = lambda0;
		model->setRateDeviation(it2->first, ratio);

		gessH[i][j++] = (H4 - H3 - H2 + H1) / ( 4.0 * ratio * model->Q_sum * ratioLambda * model->Lambda );
	}

	gessH[i][j] = 0.0;
}*/

void GasOil2DSolver::solveStep()
{
	int cellIdx, varIdx;
	double err_newton = 1.0;
	double averPresPrev = averValue(1);
	double averSatPrev = averValue(2);
	double averPres, averSat;
	double dAverPres = 1.0, dAverSat = 1.0;
	
	iterations = 0;
	while( err_newton > 1.e-4 && ( dAverSat > 1.e-8 || dAverPres > 1.e-4) && iterations < 8 )
	{	
		copyIterLayer();

		Solve(model->cellsNum_r+1, 2*(model->cellsNum_z+2), PRES);
		construction_from_fz(model->cellsNum_r+2, 2*(model->cellsNum_z+2), PRES);
		model->solveP_bub();
 
		err_newton = convergance(cellIdx, varIdx);

		averPres = averValue(1);					averSat = averValue(2);
		dAverPres = fabs(averPres - averPresPrev);	dAverSat = fabs(averSat - averSatPrev);
		averPresPrev = averPres;					averSatPrev = averSat;

//		if(varIdx == PRES)
//			cout << "BadPresValue[" << cellIdx  << "]: " << model->cells[cellIdx].u_next.p << endl;
//		else if(varIdx == SAT)
//			cout << "BadSatValue[" << cellIdx  << "]: " << model->cells[cellIdx].u_next.s << endl;

		iterations++;
	}

	cout << "Newton Iterations = " << iterations << endl;
}

void GasOil2DSolver::construction_from_fz(int N, int n, int key)
{
	vector<Cell>::iterator it;
	if (key == PRES)
	{
		for(int i = 0; i < N; i++)
		{
			for(int j = 0; j < model->cellsNum_z+2; j++)
			{
				Var2phase& var = model->cells[i*(model->cellsNum_z+2) + j].u_next;
				var.p = fz[i][2*j+1];
 				var.s = fz[i][2*j+2];
			}
		}
	}
}

void GasOil2DSolver::LeftBoundAppr(int MZ, int key)
{
	for(int i = 0; i < MZ; i++)
	{
		for (int j = 0 ; j < MZ; j++)
		{
			C[i][j] = 0.0;
			B[i][j] = 0.0;
			A[i][j] = 0.0;
		}
		C[i][i] = 1.0;
		B[i][i] = -1.0;
		RightSide[i][0] = 0.0;
	}

	map<int,double>::iterator it;
	int idx;
	if(key == PRES)
	{
		for(it = model->Qcell.begin(); it != model->Qcell.end(); ++it)
		{
			Cell& curr = model->cells[it->first];
			Cell& nebr = model->cells[it->first+model->cellsNum_z+2];
			idx = 2 * it->first;
			
			// First eqn
			C[idx][idx] = model->solve_eqLeft_dp(it->first);
			C[idx][idx+1] = model->solve_eqLeft_ds(it->first);
			B[idx][idx] = model->solve_eqLeft_dp_beta(it->first);
			B[idx][idx+1] = model->solve_eqLeft_ds_beta(it->first);
			RightSide[idx][0] = -model->solve_eqLeft(it->first) + 
								C[idx][idx] * curr.u_next.p + C[idx][idx+1] * curr.u_next.s +
								B[idx][idx] * nebr.u_next.p + B[idx][idx+1] * nebr.u_next.s;

			// Second eqn
			C[idx+1][idx+1] = 1.0;
			B[idx+1][idx+1] = (curr.r - nebr.r) / (model->cells[it->first+2*model->cellsNum_z+4].r - nebr.r) - 1.0;
			A[idx+1][idx+1] = -(curr.r - nebr.r) / (model->cells[it->first+2*model->cellsNum_z+4].r - nebr.r);
			RightSide[idx+1][0] = 0.0;
		}
	}

	construction_bz(MZ, 2);
}

void GasOil2DSolver::RightBoundAppr(int MZ, int key)
{
	for(int i = 0; i < MZ; i++)
	{
		for (int j = 0 ; j < MZ; j++)
		{
			C[i][j] = 0.0;
			B[i][j] = 0.0;
			A[i][j] = 0.0;
		}
		RightSide[i][0] = 0.0;
	}

	if(key == PRES)
	{
		int idx = 0;
		
		for(int i = (model->cellsNum_r+1)*(model->cellsNum_z+2); i < (model->cellsNum_r+2)*(model->cellsNum_z+2); i++)
		{
			Cell& curr = model->cells[i];
			Cell& nebr = model->cells[i-model->cellsNum_z-2];

			// First eqn
			A[idx][idx] = model->solve_eqRight_dp(i);
			A[idx][idx+1] = model->solve_eqRight_ds(i);
			B[idx][idx] = model->solve_eqRight_dp_beta(i);
			B[idx][idx+1] = model->solve_eqRight_ds_beta(i);
			RightSide[idx][0] = -model->solve_eqRight(i) + 
								A[idx][idx] * curr.u_next.p + A[idx][idx+1] * curr.u_next.s +
								B[idx][idx] * nebr.u_next.p + B[idx][idx+1] * nebr.u_next.s;

			if( model->rightBoundIsPres )
			{
				// Second eqn
				A[idx+1][idx+1] = 1.0;
				RightSide[idx+1][0] = model->props_sk[ model->getSkeletonIdx(model->cells[i]) ].s_init;
			} else {
				// Second eqn
				A[idx+1][idx+1] = 1.0;
				B[idx+1][idx+1] = -1.0;
				RightSide[idx+1][0] = 0.0;
			}

			idx += 2;
		}
	}

	construction_bz(MZ,1);
}

void GasOil2DSolver::MiddleAppr(int current, int MZ, int key)
{
	for(int i = 0; i < MZ; i++)
	{
		for(int j = 0; j < MZ; j++)
		{
			A[i][j] = 0.0;
			B[i][j] = 0.0;
			C[i][j] = 0.0;
		}
		RightSide[i][0] = 0.0;
	}

	if(key == PRES)
	{
		int idx = 0;
		int i = current * (model->cellsNum_z+2);
		
		// Top cell
		TopAppr(i, key);
		idx+=2;

		// Middle cells
		for(i = current * (model->cellsNum_z+2) + 1; i < (current+1) * (model->cellsNum_z+2) - 1; i++)
		{
			Var2phase& next = model->cells[i].u_next;

			// First eqn
			C[idx][idx] = model->solve_eq1_dp_beta(i, i - model->cellsNum_z - 2);
			C[idx][idx+1] = model->solve_eq1_ds_beta(i, i - model->cellsNum_z - 2);
			B[idx][idx-2] = model->solve_eq1_dp_beta(i, i-1);
			B[idx][idx-1] = model->solve_eq1_ds_beta(i, i-1);
			B[idx][idx] = model->solve_eq1_dp(i);
			B[idx][idx+1] = model->solve_eq1_ds(i);
			B[idx][idx+2] = model->solve_eq1_dp_beta(i, i+1);
			B[idx][idx+3] = model->solve_eq1_ds_beta(i, i+1);
			A[idx][idx] = model->solve_eq1_dp_beta(i, i + model->cellsNum_z + 2);
			A[idx][idx+1] = model->solve_eq1_ds_beta(i, i + model->cellsNum_z + 2);
			RightSide[idx][0] = -model->solve_eq1(i) + 
								C[idx][idx] * model->cells[i-model->cellsNum_z-2].u_next.p + C[idx][idx+1] * model->cells[i-model->cellsNum_z-2].u_next.s + 
								B[idx][idx-2] * model->cells[i-1].u_next.p + B[idx][idx-1] * model->cells[i-1].u_next.s +
								B[idx][idx] * model->cells[i].u_next.p + B[idx][idx+1] * model->cells[i].u_next.s + 
								B[idx][idx+2] * model->cells[i+1].u_next.p + B[idx][idx+3] * model->cells[i+1].u_next.s + 
								A[idx][idx] * model->cells[i+model->cellsNum_z+2].u_next.p + A[idx][idx+1] * model->cells[i+model->cellsNum_z+2].u_next.s;			
			// Second eqn
			C[idx+1][idx] = model->solve_eq2_dp_beta(i, i - model->cellsNum_z - 2);
			C[idx+1][idx+1] = model->solve_eq2_ds_beta(i, i - model->cellsNum_z - 2);
			B[idx+1][idx-2] = model->solve_eq2_dp_beta(i, i-1);
			B[idx+1][idx-1] = model->solve_eq2_ds_beta(i, i-1);
			B[idx+1][idx] = model->solve_eq2_dp(i);
			B[idx+1][idx+1] = model->solve_eq2_ds(i);
			B[idx+1][idx+2] = model->solve_eq2_dp_beta(i, i+1);
			B[idx+1][idx+3] = model->solve_eq2_ds_beta(i, i+1);
			A[idx+1][idx] = model->solve_eq2_dp_beta(i, i + model->cellsNum_z + 2);
			A[idx+1][idx+1] = model->solve_eq2_ds_beta(i, i + model->cellsNum_z + 2);
			RightSide[idx+1][0] = -model->solve_eq2(i) + 
								C[idx+1][idx] * model->cells[i-model->cellsNum_z-2].u_next.p + C[idx+1][idx+1] * model->cells[i-model->cellsNum_z-2].u_next.s + 
								B[idx+1][idx-2] * model->cells[i-1].u_next.p + B[idx+1][idx-1] * model->cells[i-1].u_next.s +
								B[idx+1][idx] * model->cells[i].u_next.p + B[idx+1][idx+1] * model->cells[i].u_next.s + 
								B[idx+1][idx+2] * model->cells[i+1].u_next.p + B[idx+1][idx+3] * model->cells[i+1].u_next.s + 
								A[idx+1][idx] * model->cells[i+model->cellsNum_z+2].u_next.p + A[idx+1][idx+1] * model->cells[i+model->cellsNum_z+2].u_next.s;
			idx += 2;
		}

		// Bottom cell
		BottomAppr(i, key);
	}

	construction_bz(MZ,2);
}

void GasOil2DSolver::TopAppr(int i, int key)
{
	if(key == PRES)
	{
		// First eqn
		B[0][0] = 1.0;
		B[0][2] = -1.0;

		// Second eqn
		B[1][1] = 1.0;
		B[1][3] = -1.0;
	}
}

void GasOil2DSolver::BottomAppr(int i, int key)
{
	if(key == PRES)
	{
		int idx = 2 * (model->cellsNum_z + 2) - 2;

		// First eqn
		B[idx][idx] = 1.0;
		B[idx][idx-2] = -1.0;

		// Second eqn
		B[idx+1][idx+1] = 1.0;
		B[idx+1][idx-1] = -1.0;
	}
}