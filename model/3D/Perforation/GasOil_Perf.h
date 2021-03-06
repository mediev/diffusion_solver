#ifndef GASOIL_PERF_H_
#define GASOIL_PERF_H_

#include <vector>
#include <map>
#include <string>

#include "model/cells/Iterators.h"
#include "model/cells/Variables.hpp"
#include "model/cells/CylCellPerf.h"
#include "model/AbstractModel.hpp"
#include "util/Interpolate.h"
#include "util/utils.h"

namespace gasOil_perf
{
	typedef CylCellPerf<Var2phase> Cell;
	typedef Iterator<CylCellPerf<Var2phase> > Iterator;

	struct Skeleton_Props
	{
		// Porosity in STC
		double m; 
		// Density of skeleton matter in STC [kg/m3]
		double dens_stc;
		// Compessibility [1/Pa]
		double beta;
		// Permeability along radial direction [mD]
		double perm_r;
		// Permeability along vertical direction [mD]
		double perm_z;

		// Permeability of colmatage zone [mD]
		std::vector<double> perms_eff;
		// Radius of colmatage zone [m]
		std::vector<double> radiuses_eff;
		// Vector of skins
		std::vector<double> skins;
		double perm_eff;
		double radius_eff;
		double skin;

		// Top and bottom depth of perforation
		double h1, h2;
		// Height of formation [m]
		double height;

		int cellsNum_z;

		double p_out;
		double p_init;
		double p_bub;
		double s_init;
	};

	struct Fluid_Props
	{
		// Viscosity [cP]
		double visc;
		// Density of fluid in STC [kg/m3]
		double dens_stc;
		// Volume factor for well bore
		double b_bore;
		// Compessibility [1/Pa]
		double beta;
		// Relative fluid permeability
		Interpolate* kr;
		// Fluid volume factor
		Interpolate* b;
	};

	struct Properties
	{
		// Vector of start times of periods [sec]
		std::vector<double> timePeriods;
		// Vector of rates [m3/day]
		std::vector<double> rates;
		// Vector of BHPs [Pa]
		std::vector<double> pwf;

		// If left boundary condition would be 2nd type
		bool leftBoundIsRate;
		// If right boundary condition would be 1st type
		bool rightBoundIsPres;
	
		// Perforated tunnels
		/* Cell number && number of cells in depth */
		std::vector<std::pair<int,int> > perfTunnels;
		// Time step limits
		// Initial time step [sec]
		double ht;
		// Minimal time step [sec]
		double ht_min;
		// Maximum time step [sec]
		double ht_max;
		// During the time flow rate decreases 'e' times in well test [sec] 
		double alpha;

		// Inner radius of well [m]
		double r_w;
		// Radius of formation [m]
		double r_e;
	
		// Number of cells in radial direction
		int cellsNum_r;
		// Number of cells in axial direction
		int cellsNum_phi;
		// Number of cells in vertical direction
		int cellsNum_z;

		std::vector<Skeleton_Props> props_sk;
		Fluid_Props props_oil;
		Fluid_Props props_gas;

		double depth_point;

		// Data set (saturation, relative oil permeability)
		std::vector< std::pair<double,double> > kr_oil;
		// Data set (saturation, relative gas permeability)
		std::vector< std::pair<double,double> > kr_gas;

		// Data set (pressure, oil volume factor) ([Pa], [m3/m3])
		std::vector< std::pair<double,double> > B_oil;
		// Data set (pressure, gas volume factor) ([Pa], [m3/m3])
		std::vector< std::pair<double,double> > B_gas;

		// Data set (pressure, gas content in oil) ([Pa], [m3/m3])
		std::vector< std::pair<double,double> > Rs;
	};

	class GasOil_Perf : public AbstractModel<Var2phase, Properties, CylCellPerf, GasOil_Perf>
	{
		template<typename> friend class Snapshotter;
		template<typename> friend class GRDECLSnapshotter;
		template<typename> friend class VTKSnapshotter;
		template<typename> friend class AbstractSolver;
		friend class ParPerfSolver;
		template<typename> friend class MidStencil;
		template<typename> friend class LeftStencil;
		template<typename> friend class RightStencil;
		template<typename> friend class TopStencil;
		template<typename> friend class BotStencil;
		template<typename> friend class UsedStencils;

	protected:

		// Middle iterator
		Iterator* midIter;
		Iterator* midBegin;
		Iterator* midEnd;

		// Left iterator
		Iterator* leftIter;
		Iterator* leftBegin;
		Iterator* leftEnd;

		// Right iterator
		Iterator* rightIter;
		Iterator* rightBegin;
		Iterator* rightEnd;

		// Continuum properties
		int skeletonsNum;
		std::vector<Skeleton_Props> props_sk;
		Fluid_Props props_oil;
		Fluid_Props props_gas;

		// Number of cells in radial direction
		int cellsNum_r;
		// Number of cells in axial direction
		int cellsNum_phi;
		// Number of cells in vertical direction
		int cellsNum_z;

		// Structure to construct tunnelss
		std::vector<std::pair<int, int> > perfTunnels;
		// Border cells in tunnels
		std::vector<Cell> tunnelCells;
		void buildTunnels();
		void setUnused();
		std::map<int,int> tunnelNebrMap;
		std::map<int,std::pair<int, int> > nebrMap;

		inline Cell& getCell(int num)
		{
			Cell& cell = cells[num];
			//assert(cell.isUsed);

			return cell;
		};

		inline Cell& getCell(int num, int beta)
		{
			Cell& cell = cells[num];
			//assert(cell.isUsed);

			Cell& nebr = cells[beta];
			if (nebr.isUsed)
				return nebr;
			else
				return tunnelCells[tunnelNebrMap[num]];
		};

		inline const Cell& getCell(const int num) const
		{
			const Cell& cell = cells[num];
			//assert(cell.isUsed);

			return cell;
		};

		inline const Cell& getCell(const int num, const int beta) const
		{
			const Cell& cell = cells[num];
			//assert(cell.isUsed);

			const Cell& nebr = cells[beta];
			if (nebr.isUsed)
				return nebr;
			else
				return tunnelCells[ tunnelNebrMap.at(num) ];
		};

		// Gas content in oil
		Interpolate* Rs;
		Interpolate* Prs;

		// BHP will be converted to the depth
		double depth_point;
		// During the time flow rate decreases 'e' times in well test [sec] 
		double alpha;

		void setInitialState();
		// Set all properties
		void setProps(Properties& props);
		// Make all properties dimensionless
		void makeDimLess();
		// Build grid
		void buildGridLog();
		// Set perforated cells
		void setPerforated();
		// Set some deviation to rate distribution
		void setRateDeviation(int num, double ratio);
		// Check formations properties
		void checkSkeletons(const std::vector<Skeleton_Props>& props);

		// Service functions
		inline double upwindIsCur(const Cell* cell, const Cell* nebr) const
		{
			if(cell->u_next.p < nebr->u_next.p)
				return 0.0;
			else
				return 1.0;
		};
		inline const Cell* getUpwindIdx(const Cell* cell, const Cell* nebr) const
		{
			assert(cell->isUsed);
			assert(nebr->isUsed);
			if(cell->u_next.p < nebr->u_next.p)
				return nebr;
			else
				return cell;
		};
		inline void getNeighborIdx(Cell& cur, Cell** const neighbor)
		{
			assert(cur.isUsed);
			neighbor[0] = &getCell(cur.num, cur.num - cellsNum_z - 2);
			neighbor[1] = &getCell(cur.num, cur.num + cellsNum_z + 2);
			neighbor[2] = &getCell(cur.num, cur.num - 1);
			neighbor[3] = &getCell(cur.num, cur.num + 1);
			if (cur.num < (cellsNum_r + 2) * (cellsNum_z + 2))
				neighbor[4] = &getCell(cur.num, cur.num + (cellsNum_r + 2) * (cellsNum_z + 2) * (cellsNum_phi - 1));
			else
				neighbor[4] = &getCell(cur.num, cur.num - (cellsNum_r + 2) * (cellsNum_z + 2));
			if (cur.num < (cellsNum_r + 2) * (cellsNum_z + 2) * (cellsNum_phi - 1))
				neighbor[5] = &getCell(cur.num, cur.num + (cellsNum_r + 2) * (cellsNum_z + 2));
			else
				neighbor[5] = &getCell(cur.num, cur.num - (cellsNum_r + 2) * (cellsNum_z + 2) * (cellsNum_phi - 1));
		};
		inline void getStencilIdx(int cur, Cell** const neighbor)
		{
			neighbor[0] = &getCell(cur);
			neighbor[1] = &getCell(cur - cellsNum_z - 2);
			neighbor[2] = &getCell(cur + cellsNum_z + 2);
			neighbor[3] = &getCell(cur - 1);
			neighbor[4] = &getCell(cur + 1);
			if (cur < (cellsNum_r + 2) * (cellsNum_z + 2))
				neighbor[5] = &getCell(cur + (cellsNum_r + 2) * (cellsNum_z + 2) * (cellsNum_phi - 1));
			else
				neighbor[5] = &getCell(cur - (cellsNum_r + 2) * (cellsNum_z + 2));
			if (cur < (cellsNum_r + 2) * (cellsNum_z + 2) * (cellsNum_phi - 1))
				neighbor[6] = &getCell(cur + (cellsNum_r + 2) * (cellsNum_z + 2));
			else
				neighbor[6] = &getCell(cur - (cellsNum_r + 2) * (cellsNum_z + 2) * (cellsNum_phi - 1));
		};
		inline int getIdx(int i)
		{
			if( i < 0 )
				return cellsNum + i;
			else if( i > cellsNum)
				return i - cellsNum;
			else
				return i;
		}
		inline int getSkeletonIdx(const Cell& cell) const
		{
			if (!cell.isTunnel)
			{
				int idx = 0;
				while (idx < props_sk.size())
				{
					if (cell.z <= props_sk[idx].h2 + EQUALITY_TOLERANCE)
						return idx;
					idx++;
				}
				exit(-1);
			}
			else
				return getSkeletonIdx( getCell(perfTunnels[cell.tunNum].first) );
		};

		// Solving coefficients
		inline double getPoro(double p, Cell& cell) const
		{
			const int idx = getSkeletonIdx(cell);
			return props_sk[idx].m * (1.0 + props_sk[idx].beta * p );
		};
		inline double getPoro_dp(Cell& cell) const
		{
			const int idx = getSkeletonIdx(cell);
			return props_sk[idx].m * props_sk[idx].beta;
		};
		inline double getTrans(Cell& cell, Cell& beta)
		{
			double k1, k2, S;
			const int idx1 = getSkeletonIdx(cell);
			const int idx2 = getSkeletonIdx(beta);

			if( fabs(cell.z - beta.z) > EQUALITY_TOLERANCE ) {
				k1 = props_sk[idx1].perm_z;
				k2 = props_sk[idx2].perm_z;
				if(k1 == 0.0 && k2 == 0.0)
					return 0.0;
				S = cell.hphi * cell.r * cell.hr;
				return 2.0 * k1 * k2 * S / (k1 * beta.hz + k2 * cell.hz);
			} else if( fabs(cell.r - beta.r) > EQUALITY_TOLERANCE) {
				k1 = (cell.r > props_sk[idx1].radius_eff ? props_sk[idx1].perm_r : props_sk[idx1].perm_eff);
				k2 = (beta.r > props_sk[idx2].radius_eff ? props_sk[idx2].perm_r : props_sk[idx2].perm_eff);
				if(k1 == 0.0 && k2 == 0.0)
					return 0.0;
				S = cell.hphi * cell.hz * (cell.r + sign(beta.num - cell.num) * cell.hr / 2.0);
				return 2.0 * k1 * k2 * S / (k1 * beta.hr + k2 * cell.hr);
			} else if(fabs(cell.phi - beta.phi) > EQUALITY_TOLERANCE) {
				k1 = (cell.r > props_sk[idx1].radius_eff ? props_sk[idx1].perm_r : props_sk[idx1].perm_eff);
				if(k1 == 0.0)
					return 0.0;
				S = cell.hr * cell.hz;
				return 2.0 * k1 * S / (beta.hr + cell.hr);
			}
		};
		inline double getPerm_r(const Cell& cell) const
		{
			const int idx = getSkeletonIdx(cell);
			return (cell.r > props_sk[idx].radius_eff ? props_sk[idx].perm_r : props_sk[idx].perm_eff);
		};
		inline double getKr_oil(double sat_oil) const
		{
			if(sat_oil > 1.0)
				return 1.0;
			else
				return props_oil.kr->Solve(sat_oil);
		};
		inline double getKr_oil_ds(double sat_oil) const
		{
			if(sat_oil > 1.0)
				return props_oil.kr->DSolve(1.0);
			else
				return props_oil.kr->DSolve(sat_oil);
		};
		inline double getKr_gas(double sat_oil) const
		{
			if(sat_oil > 1.0)
				return 0.0;
			else
				return props_gas.kr->Solve(sat_oil);
		};
		inline double getKr_gas_ds(double sat_oil) const
		{
			if(sat_oil > 1.0)
				return props_gas.kr->DSolve(1.0);
			else
				return props_gas.kr->DSolve(sat_oil);
		};
		inline double getB_oil(double p, double p_bub, bool SATUR) const
		{
			if(SATUR)
				return props_oil.b->Solve(p);
			else
				return props_oil.b->Solve(p_bub) * (1.0 + props_oil.beta * (p_bub - p));
		};
		inline double getBoreB_oil(double p, double p_bub, bool SATUR) const
		{
			return props_oil.b_bore;
			//return getB_oil(p, p_bub, SATUR);
		};
		inline double getB_oil_dp(double p, double p_bub, bool SATUR) const
		{
			if(SATUR)
				return props_oil.b->DSolve(p);
			else
				return -props_oil.b->Solve(p_bub) * props_oil.beta;
		};
		inline double getB_gas(double p) const
		{
			return props_gas.b->Solve(p);
		};
		inline double getB_gas_dp(double p) const
		{
			return props_gas.b->DSolve(p);
		};
		inline double getRs(double p, double p_bub, bool SATUR) const
		{
			if(SATUR)
				return Rs->Solve(p);
			else
				return Rs->Solve(p_bub);
		};
		inline double getRs_dp(double p, double p_bub, bool SATUR) const
		{
			if(SATUR)
				return Rs->DSolve(p);
			else
				return 0.0;
		};
		inline double getPresFromRs(double rs) const
		{
			return Prs->Solve(rs);
		};
		inline void solveP_bub()
		{
			double factRs, dissGas;

			for (auto& cell: cells)
			{
				Var2phase& next = cell.u_next;
				Var2phase& prev = cell.u_prev;

				if (next.s > 1.0)
					next.s = 1.0;

				dissGas = (1.0 - next.s) * getB_oil(next.p, next.p_bub, next.SATUR) / ((1.0 - next.s) * getB_oil(next.p, next.p_bub, next.SATUR) + next.s * getB_gas(next.p));
				factRs = getRs(prev.p, prev.p_bub, next.SATUR) + dissGas;

				if (getRs(next.p, next.p, next.SATUR) > factRs)
				{
					next.p_bub = getPresFromRs(factRs);
					next.SATUR = false;
				}
				else {
					next.p_bub = next.p;
					next.SATUR = true;
				}
			}

			for (auto& cell: tunnelCells)
			{
				Var2phase& next = cell.u_next;
				Var2phase& prev = cell.u_prev;

				if (next.s > 1.0)
					next.s = 1.0;

				dissGas = (1.0 - next.s) * getB_oil(next.p, next.p_bub, next.SATUR) / ((1.0 - next.s) * getB_oil(next.p, next.p_bub, next.SATUR) + next.s * getB_gas(next.p));
				factRs = getRs(prev.p, prev.p_bub, next.SATUR) + dissGas;

				if (getRs(next.p, next.p, next.SATUR) > factRs)
				{
					next.p_bub = getPresFromRs(factRs);
					next.SATUR = false;
				}
				else {
					next.p_bub = next.p;
					next.SATUR = true;
				}
			}	
		};

		// Thermal functions
		inline double getRho_oil(double p, double p_bub, bool SATUR) const
		{
			return (props_oil.dens_stc + getRs(p, p_bub, SATUR) * props_gas.dens_stc) / getB_oil(p, p_bub, SATUR);
		};
		inline double getRho_gas(double p) const
		{
			return props_gas.dens_stc / getB_gas(p);
		};
		inline double getNablaP(Cell& cell, int varNum, int axis)
		{
			Cell* nebr1;
			Cell* nebr2;
			double h;
			double r_eff;
			int idx = getSkeletonIdx(cell);

			switch(axis)
			{
			case R_AXIS:
				nebr1 = &getCell(cell.num, cell.num - cellsNum_z - 2);
				nebr2 = &getCell(cell.num, cell.num + cellsNum_z + 2);
				
				r_eff = props_sk[idx].radius_eff;
				if ( (nebr1->r < r_eff) && (nebr2->r > r_eff) )
				{
					if (cell.r > r_eff)
						nebr1 = &cell;
					else
						nebr2 = &cell;
				}
				h = nebr2->r - nebr1->r;
				break;
			case PHI_AXIS:
				nebr1 = &getCell(cell.num, getIdx(cell.num - (cellsNum_r + 2) * (cellsNum_z + 2)) );
				nebr2 = &getCell(cell.num, getIdx(cell.num + (cellsNum_r + 2) * (cellsNum_z + 2)) );
				h = nebr1->r * (nebr2->phi - nebr1->phi);
				if(abs(nebr2->phi - nebr1->phi) > 2.0 * cell.hphi + EQUALITY_TOLERANCE)
					h = nebr1->r * (nebr2->phi - nebr1->phi + 2.0 * M_PI);
				break;
			case Z_AXIS:
				nebr1 = &getCell(cell.num, cell.num - 1);
				nebr2 = &getCell(cell.num, cell.num + 1);
				h = nebr2->z - nebr1->z;
				break;
			}

			switch(varNum)
			{
			case PREV:
				return (nebr2->u_prev.p - nebr1->u_prev.p ) / h;
			case ITER:
				return (nebr2->u_iter.p - nebr1->u_iter.p ) / h;
			case NEXT:
				return (nebr2->u_next.p - nebr1->u_next.p ) / h;
			}
		};
		inline double getOilVelocity(Cell& cell, int varNum, int axis)
		{
			const int idx = getSkeletonIdx( cell );

			Var2phase* var;
			switch(varNum)
			{
			case PREV:
				var = &cell.u_prev;
				break;
			case ITER:
				var = &cell.u_iter;
				break;
			case NEXT:
				var = &cell.u_next;
				break;
			}

			switch(axis)
			{
			case R_AXIS:
				return -getPerm_r(cell) * getKr_oil(var->s) / props_oil.visc * getNablaP(cell, varNum, axis);
			case PHI_AXIS:
				return -getPerm_r(cell) * getKr_oil(var->s) / props_oil.visc * getNablaP(cell, varNum, axis);
			case Z_AXIS:
				return -props_sk[idx].perm_z * getKr_oil(var->s) / props_oil.visc * getNablaP(cell, varNum, axis);
			}
		};
		inline double getGasVelocity(Cell& cell, int varNum, int axis)
		{
			const int idx = getSkeletonIdx( cell );

			Var2phase* var;
			switch(varNum)
			{
			case PREV:
				var = &cell.u_prev;
				break;
			case ITER:
				var = &cell.u_iter;
				break;
			case NEXT:
				var = &cell.u_next;
				break;
			}

			switch(axis)
			{
			case R_AXIS:
				return -getPerm_r(cell) * getKr_gas(var->s) / props_gas.visc * getNablaP(cell, varNum, axis);
			case PHI_AXIS:
				return -getPerm_r(cell) * getKr_gas(var->s) / props_gas.visc * getNablaP(cell, varNum, axis);
			case Z_AXIS:
				return -props_sk[idx].perm_z * getKr_gas(var->s) / props_gas.visc * getNablaP(cell, varNum, axis);
			}
		};

		// First eqn
		double solve_eq1(int cur);
		double solve_eq1_dp(int cur, int beta);
		double solve_eq1_ds(int cur, int beta);
		double solve_eq1_dp_beta(int cur, int beta);
		double solve_eq1_ds_beta(int cur, int beta);

		// Second eqn
		double solve_eq2(int cur);
		double solve_eq2_dp(int cur, int beta);
		double solve_eq2_ds(int cur, int beta);
		double solve_eq2_dp_beta(int cur, int beta);
		double solve_eq2_ds_beta(int cur, int beta);

		/*-------------- Left cells ------------------*/

		inline double solve_eq1Left(int cur)
		{
			Cell& cell = tunnelCells[cur];
			Cell& nebr = getCell(nebrMap[cur].first);
			const Var2phase& next = cell.u_next;
			const Var2phase& upwd = getUpwindIdx(&cell, &nebr)->u_next;

			if (leftBoundIsRate)
				return getTrans(cell, nebr) * getKr_oil(upwd.s) / props_oil.visc / getBoreB_oil(next.p, next.p_bub, next.SATUR) * (nebr.u_next.p - next.p) - Qcell[cur];
			else
				return next.p - Pwf;
		}

		inline double solve_eq1Left_dp(int cur, int beta)
		{
			Cell& cell = tunnelCells[cur];
			Cell& nebr = getCell(nebrMap[cur].first);
			const Var2phase& next = cell.u_next;
			const Var2phase& upwd = getUpwindIdx(&cell, &nebr)->u_next;

			if (leftBoundIsRate)
				return -getTrans(cell, nebr) * getKr_oil(upwd.s) / getBoreB_oil(next.p, next.p_bub, next.SATUR) / props_oil.visc;
			else
				return 1.0;
		}

		inline double solve_eq1Left_ds(int cur, int beta)
		{
			Cell& cell = tunnelCells[cur];
			Cell& nebr = getCell(nebrMap[cur].first);
			const Var2phase& next = cell.u_next;
			const Var2phase& upwd = getUpwindIdx(&cell, &nebr)->u_next;

			if (leftBoundIsRate)
				return getTrans(cell, nebr) * upwindIsCur(&cell, &nebr) * getKr_oil_ds(upwd.s) / getBoreB_oil(next.p, next.p_bub, next.SATUR) / props_oil.visc * (nebr.u_next.p - next.p);
			else
				return 0.0;
		}

		inline double solve_eq1Left_dp_beta(int cur, int beta)
		{
			Cell& cell = tunnelCells[cur];
			Cell& nebr = getCell(nebrMap[cur].first);
			const Var2phase& next = cell.u_next;
			const Var2phase& upwd = getUpwindIdx(&cell, &nebr)->u_next;

			if (leftBoundIsRate)
				return getTrans(cell, nebr) * getKr_oil(upwd.s) / getBoreB_oil(next.p, next.p_bub, next.SATUR) / props_oil.visc;
			else
				return 0.0;
		}

		inline double solve_eq1Left_ds_beta(int cur, int beta)
		{
			Cell& cell = tunnelCells[cur];
			Cell& nebr = getCell(nebrMap[cur].first);
			const Var2phase& next = cell.u_next;
			const Var2phase& upwd = getUpwindIdx(&cell, &nebr)->u_next;

			if (leftBoundIsRate)
				return getTrans(cell, nebr) * (1.0 - upwindIsCur(&cell, &nebr)) * getKr_oil_ds(upwd.s) / getBoreB_oil(next.p, next.p_bub, next.SATUR) / props_oil.visc * (nebr.u_next.p - next.p);
			else
				return 0.0;
		}

		inline double solve_eq2Left(int cur)
		{
			Cell& cell = tunnelCells[cur];
			Cell& nebr1 = getCell(nebrMap[cur].first);
			Cell& nebr2 = getCell(nebrMap[cur].second);

			if( fabs(nebr2.r - nebr1.r) > EQUALITY_TOLERANCE)
				return (nebr2.u_next.s - nebr1.u_next.s) / (nebr2.r - nebr1.r) - (nebr1.u_next.s - cell.u_next.s) / (nebr1.r - cell.r);
			else if(fabs(nebr2.z - nebr1.z) > EQUALITY_TOLERANCE)
				return (nebr2.u_next.s - nebr1.u_next.s) / (nebr2.z - nebr1.z) - (nebr1.u_next.s - cell.u_next.s) / (nebr1.z - cell.z);
			else if (fabs(nebr2.phi - nebr1.phi) > EQUALITY_TOLERANCE)
				return (nebr2.u_next.s - nebr1.u_next.s) / (nebr2.phi - nebr1.phi) / nebr1.r - (nebr1.u_next.s - cell.u_next.s) / (nebr1.phi - cell.phi) / nebr1.r;
		}

		inline double solve_eq2Left_dp(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Left_ds(int cur, int beta)
		{
			Cell& cell = tunnelCells[cur];
			Cell& nebr = getCell(nebrMap[cur].first);

			if (fabs(cell.r - nebr.r) > EQUALITY_TOLERANCE)
				return 1.0 / (nebr.r - cell.r);
			else if (fabs(cell.z - nebr.z) > EQUALITY_TOLERANCE)
				return 1.0 / (nebr.z - cell.z);
			else if (fabs(cell.phi - nebr.phi) > EQUALITY_TOLERANCE)
				return 1.0 / (nebr.phi - cell.phi) / nebr.r;
		}

		inline double solve_eq2Left_dp_beta(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Left_ds_beta(int cur, int beta)
		{
			Cell& cell = tunnelCells[cur];
			Cell& nebr1 = getCell(nebrMap[cur].first);
			Cell& nebr2 = getCell(nebrMap[cur].second);

			if (beta == nebr1.num)
			{
				if (fabs(cell.r - nebr1.r) > EQUALITY_TOLERANCE)
					return -1.0 / (nebr2.r - nebr1.r) - 1.0 / (nebr1.r - cell.r);
				else if (fabs(cell.z - nebr1.z) > EQUALITY_TOLERANCE)
					return -1.0 / (nebr2.z - nebr1.z) - 1.0 / (nebr1.z - cell.z);
				else if (fabs(cell.phi - nebr1.phi) > EQUALITY_TOLERANCE)
					return -1.0 / (nebr2.phi - nebr1.phi) / nebr1.r - 1.0 / (nebr1.phi - cell.phi) / nebr1.r;
			}
			else
			{
				if (fabs(cell.r - nebr1.r) > EQUALITY_TOLERANCE)
					return 1.0 / (nebr2.r - nebr1.r);
				else if (fabs(cell.z - nebr1.z) > EQUALITY_TOLERANCE)
					return 1.0 / (nebr2.z - nebr1.z);
				else if (fabs(cell.phi - nebr1.phi) > EQUALITY_TOLERANCE)
					return 1.0 / (nebr2.phi - nebr1.phi) / nebr1.r;
			}
		}

		/*-------------- Right cells ------------------*/

		inline double solve_eq1Right(int cur)
		{
			const Cell& cell = getCell(cur);

			if (rightBoundIsPres)
				return cell.u_next.p - props_sk[getSkeletonIdx(cell)].p_out;
			else
				return cell.u_next.p - getCell(cur - cellsNum_z - 2).u_next.p;
		}

		inline double solve_eq1Right_dp(int cur, int beta)
		{
			if (rightBoundIsPres)
				return 1.0;
			else
				return 1.0;
		}

		inline double solve_eq1Right_ds(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq1Right_dp_beta(int cur, int beta)
		{
			if (rightBoundIsPres)
				return 0.0;
			else
				return -1.0;
		}

		inline double solve_eq1Right_ds_beta(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Right(int cur)
		{
			if (rightBoundIsPres)
			{
				return getCell(cur).u_next.s - props_sk[getSkeletonIdx(getCell(cur))].s_init;
			}
			else {
				return getCell(cur).u_next.s - getCell(cur - cellsNum_z - 2).u_next.s;
			}
		}

		inline double solve_eq2Right_dp(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Right_ds(int cur, int beta)
		{
			return 1.0;
		}

		inline double solve_eq2Right_dp_beta(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Right_ds_beta(int cur, int beta)
		{
			if (rightBoundIsPres)
			{
				return 0.0;
			}
			else {
				return -1.0;
			}
		}

		/*-------------- Top cells ------------------*/

		inline double solve_eq1Top(int cur)
		{
			return getCell(cur).u_next.p - getCell(cur + 1).u_next.p;
		}

		inline double solve_eq1Top_dp(int cur, int beta)
		{
			return 1.0;
		}

		inline double solve_eq1Top_ds(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq1Top_dp_beta(int cur, int beta)
		{
			return -1.0;
		}

		inline double solve_eq1Top_ds_beta(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Top(int cur)
		{
			return getCell(cur).u_next.s - getCell(cur + 1).u_next.s;
		}

		inline double solve_eq2Top_dp(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Top_ds(int cur, int beta)
		{
			return 1.0;
		}

		inline double solve_eq2Top_dp_beta(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Top_ds_beta(int cur, int beta)
		{
			return -1.0;
		}

		/*-------------- Bot cells ------------------*/

		inline double solve_eq1Bot(int cur)
		{
			return getCell(cur).u_next.p - getCell(cur - 1).u_next.p;
		}

		inline double solve_eq1Bot_dp(int cur, int beta)
		{
			return 1.0;
		}

		inline double solve_eq1Bot_ds(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq1Bot_dp_beta(int cur, int beta)
		{
			return -1.0;
		}

		inline double solve_eq1Bot_ds_beta(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Bot(int cur)
		{
			return getCell(cur).u_next.s - getCell(cur - 1).u_next.s;
		}

		inline double solve_eq2Bot_dp(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Bot_ds(int cur, int beta)
		{
			return 1.0;
		}

		inline double solve_eq2Bot_dp_beta(int cur, int beta)
		{
			return 0.0;
		}

		inline double solve_eq2Bot_ds_beta(int cur, int beta)
		{
			return -1.0;
		}

		// Finds functional
		double solveH();

		FillFoo middleFoo;
		FillFoo rightFoo;
		FillFoo leftFoo;
		FillFoo topFoo;
		FillFoo botFoo;

	public:
		GasOil_Perf();
		~GasOil_Perf();
	
		void setPeriod(int period);
		double getRate(int cur);


		inline Iterator getMidIter()
		{
			return Iterator(*midIter);
		};
		inline const Iterator& getMidBegin()
		{
			return *midBegin;
		};
		inline const Iterator& getMidEnd()
		{
			return *midEnd;
		};

		inline Iterator getLeftIter()
		{
			return Iterator(*leftIter);
		};
		inline const Iterator& getLeftBegin()
		{
			return *leftBegin;
		};
		inline const Iterator& getLeftEnd()
		{
			return *leftEnd;
		};

		inline Iterator getRightIter()
		{
			return Iterator(*rightIter);
		};
		inline const Iterator& getRightBegin()
		{
			return *rightBegin;
		};
		inline const Iterator& getRightEnd()
		{
			return *rightEnd;
		};

	};
};

#endif /* GASOIL_PERF_H_ */