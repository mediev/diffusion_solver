#include "model/3D/GasOil_3D_NIT/GasOil_3D_NIT.h"
#include "util/utils.h"

#include <cassert>

using namespace std;
using namespace gasOil_3d_NIT;

GasOil_3D_NIT::GasOil_3D_NIT()
{
	isWriteSnaps = false;
}

GasOil_3D_NIT::~GasOil_3D_NIT()
{
}

void GasOil_3D_NIT::setProps(Properties& props)
{
	leftBoundIsRate = props.leftBoundIsRate;
	rightBoundIsPres = props.rightBoundIsPres;

	// Setting grid properties
	r_w = props.r_w;
	r_e = props.r_e;
	cellsNum_r = props.cellsNum_r;
	cellsNum_phi = props.cellsNum_phi;
	cellsNum_z = props.cellsNum_z;
	cellsNum = (cellsNum_r + 2) * (cellsNum_z + 2) * cellsNum_phi;

	// Setting skeleton properties
	perfIntervals = props.perfIntervals;
	
	skeletonsNum = props.props_sk.size();
	checkSkeletons(props.props_sk);
	props_sk = props.props_sk;
	for(int j = 0; j < skeletonsNum; j++)
	{
		props_sk[j].perm_r = MilliDarcyToM2( props_sk[j].perm_r );
		props_sk[j].perm_z = MilliDarcyToM2( props_sk[j].perm_z );
	}

	periodsNum = props.timePeriods.size();
	for(int i = 0; i < periodsNum; i++)
	{
		period.push_back( props.timePeriods[i] );
		if(leftBoundIsRate)
			rate.push_back( props.rates[i] / 86400.0 );
		else
			pwf.push_back( props.pwf[i] );
		for(int j = 0; j < skeletonsNum; j++)
		{
			if(props_sk[j].radiuses_eff[i] > props.r_w)
				props_sk[j].perms_eff.push_back( MilliDarcyToM2(props.props_sk[j].perm_r * log(props.props_sk[j].radiuses_eff[i] / props.r_w) / (log(props.props_sk[j].radiuses_eff[i] / props.r_w) + props.props_sk[j].skins[i]) ) );
			else
				props_sk[j].perms_eff.push_back( MilliDarcyToM2(props.props_sk[j].perm_r) );
		}
	}

	// Temporal properties
	ht = props.ht;
	ht_min = props.ht_min;
	ht_max = props.ht_max;

	// Oil properties
	props_oil = props.props_oil;
	props_oil.visc = cPToPaSec( props_oil.visc );

	// Gas properties
	props_gas = props.props_gas;
	props_gas.visc = cPToPaSec( props_gas.visc );

	alpha = props.alpha;
	depth_point = props.depth_point;
	L = props.L;

	makeDimLess();
	
	// Data sets
	props_oil.kr = setDataset(props.kr_oil, 1.0, 1.0);
	props_gas.kr = setDataset(props.kr_gas, 1.0, 1.0);

	props_oil.b = setDataset(props.B_oil, P_dim / BAR_TO_PA, 1.0);
	props_gas.b = setDataset(props.B_gas, P_dim / BAR_TO_PA, 1.0);
	Rs = setDataset(props.Rs, P_dim / BAR_TO_PA, 1.0);
	Prs = setInvDataset(props.Rs, 1.0, P_dim / BAR_TO_PA);
}

void GasOil_3D_NIT::checkSkeletons(const vector<Skeleton_Props>& props)
{
	vector<Skeleton_Props>::const_iterator it = props.begin();
	double tmp;
	int indxs = 0;

	assert( it->h2 - it->h1 == it->height );
	indxs += it->cellsNum_z;
	tmp = it->h2;
	++it;
	
	while(it != props.end())
	{
		assert( it->h1 == tmp );
		assert( it->h2 - it->h1 == it->height );
		indxs += it->cellsNum_z;
		tmp = it->h2;
		++it;
	}
	assert( indxs == cellsNum_z );
}

void GasOil_3D_NIT::makeDimLess()
{
	// Main units
	R_dim = r_w;
	t_dim = 3600.0;
	P_dim = BAR_TO_PA;
	T_dim = props_sk[0].t_init;

	// Temporal properties
	ht /= t_dim;
	ht_min /= t_dim;
	ht_max /= t_dim;

	// Grid properties
	r_w /= R_dim;
	r_e /= R_dim;

	// Skeleton properties
	for(int i = 0; i < skeletonsNum; i++)
	{
		props_sk[i].perm_r /= (R_dim * R_dim);
		props_sk[i].perm_z /= (R_dim * R_dim);
		
		props_sk[i].beta /= (1.0 / P_dim);
		props_sk[i].dens_stc /= (P_dim * t_dim * t_dim / R_dim / R_dim);
		props_sk[i].h1 = (props_sk[i].h1 - depth_point) / R_dim;
		props_sk[i].h2 = (props_sk[i].h2 - depth_point) / R_dim;
		props_sk[i].height /= R_dim;
		props_sk[i].p_init /= P_dim;
		props_sk[i].p_out /= P_dim;
		props_sk[i].p_bub /= P_dim;
		props_sk[i].t_init /= T_dim;

		props_sk[i].c = props_sk[i].c / R_dim / R_dim * T_dim * t_dim * t_dim;
		props_sk[i].lambda_r = props_sk[i].lambda_r * T_dim * t_dim / P_dim / R_dim / R_dim;
		props_sk[i].lambda_z = props_sk[i].lambda_z * T_dim * t_dim / P_dim / R_dim / R_dim;

		for(int j = 0; j < periodsNum; j++)
		{
			props_sk[i].perms_eff[j] /= (R_dim * R_dim);
			props_sk[i].radiuses_eff[j] /= R_dim;
		}
	}

	Q_dim = R_dim * R_dim * R_dim / t_dim;
	for(int i = 0; i < periodsNum; i++)
	{
		period[i] /= t_dim;
		if(leftBoundIsRate)
			rate[i] /= Q_dim;
		else
			pwf[i] /= P_dim;
	}

	// Oil properties
	props_oil.visc /= (P_dim * t_dim);
	props_oil.dens_stc /= (P_dim * t_dim * t_dim / R_dim / R_dim);
	props_oil.beta /= (1.0 / P_dim);

	props_oil.c = props_oil.c / R_dim / R_dim * T_dim * t_dim * t_dim;
	props_oil.lambda = props_oil.lambda * T_dim * t_dim / P_dim / R_dim / R_dim;
	props_oil.jt = props_oil.jt * P_dim / T_dim;
	props_oil.ad = props_oil.ad * P_dim / T_dim;

	// Gas properties
	props_gas.visc /= (P_dim * t_dim);
	props_gas.dens_stc /= (P_dim * t_dim * t_dim / R_dim / R_dim);

	props_gas.c = props_gas.c / R_dim / R_dim * T_dim * t_dim * t_dim;
	props_gas.lambda = props_gas.lambda * T_dim * t_dim / P_dim / R_dim / R_dim;
	props_gas.jt = props_gas.jt * P_dim / T_dim;
	props_gas.ad = props_gas.ad * P_dim / T_dim;

	// Rest properties
	alpha /= t_dim;
	//depth_point = 0.0;
	L = L / R_dim / R_dim * t_dim * t_dim;
}

void GasOil_3D_NIT::buildGridLog()
{
	cells.reserve( cellsNum );

	Volume = 0.0;
	int counter = 0;
	int skel_idx = 0, cells_z = 0;

	double r_prev = r_w;
	double logMax = log(r_e / r_w);
	double logStep = logMax / (double)cellsNum_r;
	
	const double hphi = 2.0 * M_PI / (double)cellsNum_phi;
	double cm_phi = 0.0;
	double hz = 0.0;//(props_sk[skel_idx].h2 - props_sk[skel_idx].h1) / (double)(props_sk[skel_idx].cellsNum_z);
	double cm_z = props_sk[skel_idx].h1;
	double hr = r_prev * (exp(logStep) - 1.0);
	double cm_r = r_w;

	counter = 0;
	for(int k = 0; k < cellsNum_phi; k++)
	{
		skel_idx = 0;	cells_z = 0;

		r_prev = r_w;
		logMax = log(r_e / r_w);
		logStep = logMax / (double)cellsNum_r;

		hz = 0.0;
		cm_z = props_sk[0].h1;
		hr = r_prev * (exp(logStep) - 1.0);
		cm_r = r_w;

		// Left border
		cells.push_back( Cell(counter++, cm_r, cm_phi, cm_z, 0.0, hphi, 0.0) );
		for(int i = 0; i < cellsNum_z; i++)
		{
			hz = (props_sk[skel_idx].h2 - props_sk[skel_idx].h1) / (double)(props_sk[skel_idx].cellsNum_z);
			cm_z += (cells[cells.size()-1].hz + hz) / 2.0;

			cells.push_back( Cell(counter++, cm_r, cm_phi, cm_z, 0.0, hphi, hz) );
			cells_z++;

			if(cells_z >= props_sk[skel_idx].cellsNum_z)
			{
				cells_z = 0;
				skel_idx++;
			}
		}
		cells.push_back( Cell(counter++, cm_r, cm_phi, cm_z+hz/2.0, 0.0, hphi, 0.0) );

		// Middle cells
		for(int j = 0; j < cellsNum_r; j++)
		{
			skel_idx = 0;	cells_z = 0;
			cm_z = props_sk[0].h1;
			cm_r = r_prev * (exp(logStep) + 1.0) / 2.0;
			hr = r_prev * (exp(logStep) - 1.0);

			cells.push_back( Cell(counter++, cm_r, cm_phi, cm_z, hr, hphi, 0.0) );
			for(int i = 0; i < cellsNum_z; i++)
			{
				hz = (props_sk[skel_idx].h2 - props_sk[skel_idx].h1) / (double)(props_sk[skel_idx].cellsNum_z);
				cm_z += (cells[cells.size()-1].hz + hz) / 2.0;

				cells.push_back( Cell(counter++, cm_r, cm_phi, cm_z, hr, hphi, hz) );
				Volume += cells[cells.size()-1].V;
				cells_z++;

				if(cells_z >= props_sk[skel_idx].cellsNum_z)
				{
					cells_z = 0;
					skel_idx++;
				}
			}
			cells.push_back( Cell(counter++, cm_r, cm_phi, cm_z+hz/2.0, hr, hphi, 0.0) );

			r_prev = r_prev * exp(logStep);
		}

		// Right border
		cm_z = props_sk[0].h1;
		cm_r = r_e;
		
		cells.push_back( Cell(counter++, cm_r, cm_phi, cm_z, 0.0, hphi, 0.0) );
		skel_idx = 0;	cells_z = 0;
		for(int i = 0; i < cellsNum_z; i++)
		{
			hz = (props_sk[skel_idx].h2 - props_sk[skel_idx].h1) / (double)(props_sk[skel_idx].cellsNum_z);
			cm_z += (cells[cells.size()-1].hz + hz) / 2.0;

			cells.push_back( Cell(counter++, cm_r, cm_phi, cm_z, 0.0, hphi, hz) );
			cells_z++;

			if(cells_z >= props_sk[skel_idx].cellsNum_z)
			{
				cells_z = 0;
				skel_idx++;
			}
		}
		cells.push_back( Cell(counter++, cm_r, cm_phi, cm_z+hz/2.0, 0.0, hphi, 0.0) );
		cm_phi += hphi;
	}
}

void GasOil_3D_NIT::setInitialState()
{
	vector<Cell>::iterator it;
	for(it = cells.begin(); it != cells.end(); ++it)
	{
		const Skeleton_Props& props = props_sk[ getSkeletonIdx(*it) ];
		it->u_prev.p = it->u_iter.p = it->u_next.p = props.p_init;
		it->u_prev.p_bub = it->u_iter.p_bub = it->u_next.p_bub = props.p_bub;
		it->u_prev.s = it->u_iter.s = it->u_next.s = props.s_init;
		it->u_prev.t = it->u_iter.t = it->u_next.t = props.t_init;
		if(props.p_bub > props.p_init)
			it->u_prev.SATUR = it->u_iter.SATUR = it->u_next.SATUR = true;
		else
			it->u_prev.SATUR = it->u_iter.SATUR = it->u_next.SATUR = false;
	}
}

void GasOil_3D_NIT::setPerforated()
{
	height_perf = 0.0;
	vector<pair<int,int> >::iterator it;
	for(it = perfIntervals.begin(); it != perfIntervals.end(); ++it)
	{
		for(int i = it->first; i <= it->second; i++)
		{
			Qcell[getIdx(i)] = 0.0;
			Cell& cell = cells[getIdx(i)];
			height_perf += cell.hphi * cell.r * cell.hz;
		}
	}
}

void GasOil_3D_NIT::setPeriod(int period)
{
	if(leftBoundIsRate)
		Q_sum = rate[period];
	else
	{
		Pwf = pwf[period];
		Q_sum = 0.0;
	}
	
	if(period == 0 || rate[period-1] < EQUALITY_TOLERANCE ) {
		map<int,double>::iterator it;
		for(it = Qcell.begin(); it != Qcell.end(); ++it)
		{
			Cell& cell = cells[ it->first ];
			it->second = Q_sum * cell.hphi * cell.r * cell.hz / height_perf;
		}
	} else {
		map<int,double>::iterator it;
		for(it = Qcell.begin(); it != Qcell.end(); ++it)
			it->second = it->second * Q_sum / rate[period-1];
	}

	for(int i = 0; i < skeletonsNum; i++)
	{
		props_sk[i].radius_eff = props_sk[i].radiuses_eff[period];
		props_sk[i].perm_eff = props_sk[i].perms_eff[period];
		props_sk[i].skin = props_sk[i].skins[period];
	}
}

void GasOil_3D_NIT::setRateDeviation(int num, double ratio)
{
	Qcell[num] += Q_sum * ratio;
}

double GasOil_3D_NIT::solve_eq1(int cur)
{
	int neighbor [6];
	getNeighborIdx(cur, neighbor);

	Cell& cell = cells[cur];
	Var2phaseNIT& next = cell.u_next;
	Var2phaseNIT& prev = cell.u_prev;
	
	double H = 0.0;
	H = ( getPoro(next.p, cell) * next.s / getB_oil(next.p, next.p_bub, next.SATUR) - 
				getPoro(prev.p, cell) * prev.s / getB_oil(prev.p, prev.p_bub, prev.SATUR) );

	for(int i = 0; i < 6; i++)
	{
		Cell& beta = cells[ neighbor[i] ];
		Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor[i]) ].u_next;

		H += ht / cell.V * getTrans(cell, beta) * (next.p - beta.u_next.p) *
			getKr_oil(upwd.s) / props_oil.visc / getB_oil(upwd.p, upwd.p_bub, upwd.SATUR);
	}

	return H;
}

double GasOil_3D_NIT::solve_eq1_dp(int cur)
{
	double upwind;
	int neighbor [6];
	getNeighborIdx(cur, neighbor);

	Cell& cell = cells[cur];
	Var2phaseNIT& next = cell.u_next;
	double Boil_upwd;
	double Boil = getB_oil(next.p, next.p_bub, next.SATUR);
	
	double H = 0.0;
	H = (next.s * getPoro_dp(cell) - 
		getPoro(next.p, cell) * next.s * getB_oil_dp(next.p, next.p_bub, next.SATUR) / Boil ) / Boil;

	for(int i = 0; i < 6; i++)
	{
		upwind = upwindIsCur(cur, neighbor[i]);
		Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor[i]) ].u_next;
		Boil_upwd = getB_oil(upwd.p, upwd.p_bub, upwd.SATUR);
		Cell& beta = cells[ neighbor[i] ];

		H += ht / cell.V * getTrans(cell, beta) * 
			( getKr_oil(upwd.s) / props_oil.visc / Boil_upwd - 
			upwind * (next.p - beta.u_next.p) * getKr_oil(upwd.s) / props_oil.visc / Boil_upwd / Boil_upwd * getB_oil_dp(upwd.p, upwd.p_bub, upwd.SATUR) );
	}
	return H;
}

double GasOil_3D_NIT::solve_eq1_ds(int cur)
{
	double upwind;	
	int neighbor [6];
	getNeighborIdx(cur, neighbor);

	Cell& cell = cells[cur];
	Var2phaseNIT& next = cell.u_next;
	
	double H = 0.0;
	H = getPoro(next.p, cell) / getB_oil(next.p, next.p_bub, next.SATUR);

	for(int i = 0; i < 6; i++)
	{
		upwind = upwindIsCur(cur, neighbor[i]);
		Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor[i]) ].u_next;
		Cell& beta = cells[ neighbor[i] ];

		H += ht / cell.V * getTrans(cell, beta) * 
			upwind * (next.p - beta.u_next.p) * getKr_oil_ds(upwd.s) / props_oil.visc / getB_oil(upwd.p, upwd.p_bub, upwd.SATUR);
	}

	return H;
}

double GasOil_3D_NIT::solve_eq1_dp_beta(int cur, int beta)
{
	Cell& cell = cells[cur];

	double upwind = upwindIsCur(cur, beta);
	Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, beta) ].u_next;
	double Boil_upwd = getB_oil(upwd.p, upwd.p_bub, upwd.SATUR);

	return -ht / cell.V * getTrans(cell, cells[beta]) * 
			( getKr_oil(upwd.s) / props_oil.visc / Boil_upwd + 
			(1.0 - upwind) * (cell.u_next.p - cells[beta].u_next.p) * getKr_oil(upwd.s) / props_oil.visc / Boil_upwd / Boil_upwd * getB_oil_dp(upwd.p, upwd.p_bub, upwd.SATUR) );
}

double GasOil_3D_NIT::solve_eq1_ds_beta(int cur, int beta)
{
	Cell& cell = cells[cur];

	double upwind = upwindIsCur(cur, beta);
	Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, beta) ].u_next;

	return ht / cell.V * getTrans(cell, cells[beta]) * (1.0 - upwind) * (cell.u_next.p - cells[beta].u_next.p) *
			getKr_oil_ds(upwd.s) / props_oil.visc / getB_oil(upwd.p, upwd.p_bub, upwd.SATUR);
}

double GasOil_3D_NIT::solve_eq2(int cur)
{
	int neighbor [6];
	getNeighborIdx(cur, neighbor);

	Cell& cell = cells[cur];
	Var2phaseNIT& next = cell.u_next;
	Var2phaseNIT& prev = cell.u_prev;

	double H = 0.0;
	H = getPoro(next.p, cell) * ( (1.0 - next.s) / getB_gas(next.p) + next.s * getRs(next.p, next.p_bub, next.SATUR) / getB_oil(next.p, next.p_bub, next.SATUR) ) -
				getPoro(prev.p, cell) * ( (1.0 - prev.s) / getB_gas(prev.p) + prev.s * getRs(prev.p, prev.p_bub, prev.SATUR) / getB_oil(prev.p, prev.p_bub, prev.SATUR) );

	for(int i = 0; i < 6; i++)
	{
		Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor[i]) ].u_next;
		Cell& beta = cells[ neighbor[i] ];

		H += ht / cell.V * getTrans(cell, beta) * (next.p - beta.u_next.p) * 
			( getKr_oil(upwd.s) * getRs(upwd.p, upwd.p_bub, upwd.SATUR) / props_oil.visc / getB_oil(upwd.p, upwd.p_bub, upwd.SATUR) +
			getKr_gas(upwd.s) / props_gas.visc / getB_gas(upwd.p) );
	}

	return H;
}

double GasOil_3D_NIT::solve_eq2_dp(int cur)
{
	double upwind;	
	int neighbor [6];
	getNeighborIdx(cur, neighbor);

	Cell& cell = cells[cur];
	Var2phaseNIT& next = cell.u_next;
	double Boil_upwd, Bgas_upwd, rs_upwd;
	double Boil = getB_oil(next.p, next.p_bub, next.SATUR);
	double Bgas = getB_gas(next.p);
	double rs = getRs(next.p, next.p_bub, next.SATUR);
	
	double H = 0.0;
	H = ( (next.s * rs / Boil + (1.0 - next.s) / Bgas) * getPoro_dp(cell) - 
		getPoro(next.p, cell) * ( (1.0 - next.s) / Bgas / Bgas * getB_gas_dp(next.p) + 
		next.s * rs / Boil / Boil * getB_oil_dp(next.p, next.p_bub, next.SATUR) - 
		next.s / Boil * getRs_dp(next.p, next.p_bub, next.SATUR) ) );

	for(int i = 0; i < 6; i++)
	{
		upwind = upwindIsCur(cur, neighbor[i]);
		Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor[i]) ].u_next;
		Boil_upwd = getB_oil(upwd.p, upwd.p_bub, upwd.SATUR);
		Bgas_upwd = getB_gas(upwd.p);
		rs_upwd = getRs(upwd.p, upwd.p_bub, upwd.SATUR);
		Cell& beta = cells[ neighbor[i] ];

		H += ht / cell.V * getTrans(cell, beta) * 
			( getKr_oil(upwd.s) * rs_upwd / props_oil.visc / Boil_upwd + getKr_gas(upwd.s) / props_gas.visc / Bgas_upwd + 
			upwind * (next.p - beta.u_next.p) * 
			( getKr_oil(upwd.s) / props_oil.visc / Boil_upwd * (getRs_dp(upwd.p, upwd.p_bub, upwd.SATUR) - rs_upwd * getB_oil_dp(upwd.p, upwd.p_bub, upwd.SATUR) / Boil_upwd) - 
			getKr_gas(upwd.s) / props_gas.visc / Bgas_upwd / Bgas_upwd * getB_gas_dp(upwd.p) ));
	}

	return H;
}

double GasOil_3D_NIT::solve_eq2_ds(int cur)
{
	double upwind;	
	int neighbor [6];
	getNeighborIdx(cur, neighbor);

	Cell& cell = cells[cur];
	Var2phaseNIT& next = cell.u_next;
	
	double H = 0.0;
	H = getPoro(next.p, cell) * ( getRs(next.p, next.p_bub, next.SATUR) / getB_oil(next.p, next.p_bub, next.SATUR) - 1.0 / getB_gas(next.p) );

	for(int i = 0; i < 6; i++)
	{
		upwind = upwindIsCur(cur, neighbor[i]);
		Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor[i]) ].u_next;
		Cell& beta = cells[ neighbor[i] ];

		H += ht / cell.V * getTrans(cell, beta) * upwind * (next.p - beta.u_next.p) * 
			( getRs(upwd.p, upwd.p_bub, upwd.SATUR) * getKr_oil_ds(upwd.s) / props_oil.visc / getB_oil(upwd.p, upwd.p_bub, upwd.SATUR) + 
			getKr_gas_ds(upwd.s) / props_gas.visc / getB_gas(upwd.p) );
	}

	return H;
}

double GasOil_3D_NIT::solve_eq2_dp_beta(int cur, int beta)
{
	Cell& cell = cells[cur];

	double upwind = upwindIsCur(cur, beta);
	Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, beta) ].u_next;
	double Boil_upwd = getB_oil(upwd.p, upwd.p_bub, upwd.SATUR);
	double Bgas_upwd = getB_gas(upwd.p);
	double rs_upwd = getRs(upwd.p, upwd.p_bub, upwd.SATUR);

	return -ht / cell.V * getTrans(cell, cells[beta]) * 
			( getKr_oil(upwd.s) * rs_upwd / props_oil.visc / Boil_upwd + getKr_gas(upwd.s) / props_gas.visc / Bgas_upwd - 
			(1.0 - upwind) * (cells[cur].u_next.p - cells[beta].u_next.p) * 
			( getKr_oil(upwd.s) / props_oil.visc / Boil_upwd * (getRs_dp(upwd.p, upwd.p_bub, upwd.SATUR) - rs_upwd * getB_oil_dp(upwd.p, upwd.p_bub, upwd.SATUR) / Boil_upwd) - 
			getKr_gas(upwd.s) / props_gas.visc / Bgas_upwd / Bgas_upwd * getB_gas_dp(upwd.p) ));
}

double GasOil_3D_NIT::solve_eq2_ds_beta(int cur, int beta)
{
	Cell& cell = cells[cur];

	double upwind = upwindIsCur(cur, beta);
	Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, beta) ].u_next;

	return ht / cell.V * getTrans(cell, cells[beta]) * (1.0 - upwind) * (cell.u_next.p - cells[beta].u_next.p) *
		( getRs(upwd.p, upwd.p_bub, upwd.SATUR) * getKr_oil_ds(upwd.s) / props_oil.visc / getB_oil(upwd.p, upwd.p_bub, upwd.SATUR) +
		getKr_gas_ds(upwd.s) / props_gas.visc / getB_gas(upwd.p) );
}

double GasOil_3D_NIT::solve_eqLeft(int cur)
{
	const int neighbor = cur + cellsNum_z + 2;
	Var2phaseNIT& next = cells[cur].u_next;
	Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor) ].u_next;

	if( leftBoundIsRate )
		return getTrans(cells[cur], cells[neighbor]) * getKr_oil(upwd.s) / props_oil.visc / getBoreB_oil(next.p, next.p_bub, next.SATUR) * (cells[neighbor].u_next.p - next.p) - Qcell[cur];
	else
		return next.p - Pwf;
}

double GasOil_3D_NIT::solve_eqLeft_dp(int cur)
{
	const int neighbor = cur + cellsNum_z + 2;
	Var2phaseNIT& next = cells[cur].u_next;
	Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor) ].u_next;

	if( leftBoundIsRate )
		return -getTrans(cells[cur], cells[neighbor]) * getKr_oil(upwd.s) / getBoreB_oil(next.p, next.p_bub, next.SATUR) / props_oil.visc;
	else
		return 1.0;
}

double GasOil_3D_NIT::solve_eqLeft_ds(int cur)
{
	const int neighbor = cur + cellsNum_z + 2;
	Var2phaseNIT& next = cells[cur].u_next;
	Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor) ].u_next;

	if( leftBoundIsRate )
		return getTrans(cells[cur], cells[neighbor]) * upwindIsCur(cur, neighbor) * getKr_oil_ds(upwd.s) / getBoreB_oil(next.p, next.p_bub, next.SATUR) / props_oil.visc * (cells[neighbor].u_next.p - next.p);
	else
		return 0.0;
}

double GasOil_3D_NIT::solve_eqLeft_dp_beta(int cur)
{
	const int neighbor = cur + cellsNum_z + 2;
	Var2phaseNIT& next = cells[cur].u_next;
	Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor) ].u_next;

	if( leftBoundIsRate )
		return getTrans(cells[cur], cells[neighbor]) * getKr_oil(upwd.s) / getBoreB_oil(next.p, next.p_bub, next.SATUR) / props_oil.visc;
	else
		return 0.0;
}

double GasOil_3D_NIT::solve_eqLeft_ds_beta(int cur)
{
	const int neighbor = cur + cellsNum_z + 2;
	Var2phaseNIT& next = cells[cur].u_next;
	Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor) ].u_next;

	if( leftBoundIsRate )
		return getTrans(cells[cur], cells[neighbor]) * (1.0-upwindIsCur(cur, neighbor)) * getKr_oil_ds(upwd.s) / getBoreB_oil(next.p, next.p_bub, next.SATUR) / props_oil.visc * (cells[neighbor].u_next.p - next.p);
	else
		return 0.0;
}

double GasOil_3D_NIT::solve_eqRight(int cur)
{
	const Cell& cell = cells[cur];

	if( rightBoundIsPres )
		return cell.u_next.p - props_sk[getSkeletonIdx(cell)].p_out;
	else
		return cell.u_next.p - cells[cur - cellsNum_z - 2].u_next.p;
}

double GasOil_3D_NIT::solve_eqRight_dp(int cur)
{
	if( rightBoundIsPres )
		return 1.0;
	else
		return 1.0;
}

double GasOil_3D_NIT::solve_eqRight_ds(int cur)
{
	return 0.0;
}

double GasOil_3D_NIT::solve_eqRight_dp_beta(int cur)
{
	if( rightBoundIsPres )
		return 0.0;
	else
		return -1.0;
}

double GasOil_3D_NIT::solve_eqRight_ds_beta(int cur)
{
	return 0.0;
}

double GasOil_3D_NIT::solve_eq3(int cur)
{
	int neighbor [6];
	getNeighborIdx(cur, neighbor);

	Cell& cell = cells[cur];
	Var2phaseNIT& next = cell.u_next;
	Var2phaseNIT& prev = cell.u_prev;
	
	double H = 0.0;
	H = ( getPoro(next.p, cell) * next.s * getRho_oil(next.p, next.p_bub, next.SATUR) - 
		getPoro(prev.p, cell) * prev.s * getRho_oil(prev.p, prev.p_bub, prev.SATUR) ) / ht;

	for(int i = 0; i < 6; i++)
	{
		Cell& beta = cells[ neighbor[i] ];
		Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor[i]) ].u_next;

		H += 1.0 / cell.V * getTrans(cell, beta) * (next.p - beta.u_next.p) *
			getKr_oil(upwd.s) / props_oil.visc * getRho_oil(upwd.p, upwd.p_bub, upwd.SATUR);
	}

	return H;
}

double GasOil_3D_NIT::solveH()
{
	double H = 0.0;
	double p1, p0;

	map<int,double>::iterator it = Qcell.begin();
	for(int i = 0; i < Qcell.size()-1; i++)	
	{
		p0 = cells[ it->first ].u_next.p;
		p1 = cells[ (++it)->first ].u_next.p;

		H += (p1 - p0) * (p1 - p0) / 2.0;
	}

	return H;
}

double GasOil_3D_NIT::getRate(int cur)
{
	int neighbor = cur + cellsNum_z + 2;
	Var2phaseNIT& upwd = cells[ getUpwindIdx(cur, neighbor) ].u_next;
	Var2phaseNIT& next = cells[cur].u_next;
	return getTrans(cells[cur], cells[neighbor]) * getKr_oil(upwd.s) / props_oil.visc / getBoreB_oil(next.p, next.p_bub, next.SATUR) * (cells[neighbor].u_next.p - next.p);
}