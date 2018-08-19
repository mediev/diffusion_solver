#ifndef ACIDRECFRACMODEL_HPP_
#define ACIDRECFRACMODEL_HPP_

#include <map>
#include "model/Acid/recfrac/Properties.hpp"
#include "model/cells/AcidVariables.hpp"
#include "model/cells/NewEllipticCell.hpp"
#include "model/cells/HexCell.hpp"
#include "model/Basic1d/Basic1d.hpp"
#include "util/Interpolate.h"

namespace acidrecfrac
{
	typedef JustAcid PoroVariable;
	typedef TapeJustAcid PoroTapeVariable;
	typedef JustAcid PoroVariable;
	typedef TapeJustAcid PoroTapeVariable;
	typedef VarFrac FracVariable;
	typedef TapeVarFrac FracTapeVariable;

	typedef HexCell<PoroVariable> PoroCell;
	typedef HexCell<FracVariable> FracCell;
	typedef PoroCell::Type PoroType;
	typedef FracCell::Type FracType;

	typedef CalciteReaction CurrentReaction;
	typedef CurrentReaction::REACTS REACTS;

	static const int var_poro_size = PoroVariable::size;
	static const int var_frac_size = FracVariable::size;
	//typedef FracCell::Face Face;
	//typedef std::unordered_map<new_cell::AdjancedCellIdx,Face,new_cell::IdxHasher> FaceMap;
	const int NEBRS_NUM = 6;

	class AcidRecFrac
	{
		template<typename> friend class Snapshotter;
		template<typename> friend class VTKSnapshotter;
		template<typename> friend class AbstractSolver;
		friend class AcidRecFracSolver; 
	public:
		static const int var_size = var_poro_size;
	protected:
		// Porous medium
		int skeletonsNum;
		std::vector<Skeleton_Props> props_sk;
		// Fracture
		FracProperties props_frac;
		// Fluids
		CurrentReaction reac;
		Water_Props props_w;
		Oil_Props props_o;
		Gas_Props props_g;
		
		// Grids
		int cellsNum_frac, cellsNum_poro, cellsNum_y_frac, cellsNum_y_poro, cellsNum_x, cellsNum_z;
		double Volume_frac, Volume_poro;
		//std::vector<PoroGrid> poro_grids;
		std::vector<PoroCell> cells_poro;
		std::vector<FracCell> cells_frac;
		double re;
		//FaceMap fmap_frac, fmap_poro, fmap_inter;
		
		// Temporary properties
		double ht, ht_min, ht_max;
		int periodsNum;
		double Q_sum, Pwf, c, height_perf;
		std::map<int, double> Qcell;
		std::vector<double> period, rate, pwf, cs;
		std::vector<bool> LeftBoundIsRate;
		bool leftBoundIsRate;
		bool rightBoundIsPres;
		// Snapshotter
		bool isWriteSnaps;
		Snapshotter<AcidRecFrac>* snapshotter;

		void buildFracGrid();
		void buildPoroGrid();
		void processGeometry();
		void setProps(Properties& props);
		void makeDimLess();
		void setInitialState();
		void setPerforated();
		void calculateTrans();
		// Service functions
		// Schemes
		PoroTapeVariable solvePoro(const PoroCell& cell);
		PoroTapeVariable solvePoroMid(const PoroCell& cell);
		PoroTapeVariable solvePoroLeft(const PoroCell& cell);
		PoroTapeVariable solvePoroRight(const PoroCell& cell);
		PoroTapeVariable solvePoroBorder(const PoroCell& cell);
		FracTapeVariable solveFrac(const FracCell& cell);
		FracTapeVariable solveFracIn(const FracCell& cell);
		FracTapeVariable solveFracMid(const FracCell& cell);
		FracTapeVariable solveFracBorder(const FracCell& cell);
		// Service calculations
		template <class TCell>
		inline int getUpwindIdx(const TCell& cell, const TCell& beta) const
		{
			if (cell.u_next.p < beta.u_next.p)
				return beta.num;
			else
				return cell.num;
		};
		inline const size_t getFracNebr(const size_t poro_idx)
		{
			return poro_idx / (cellsNum_y_poro + 2) * (cellsNum_y_frac + 1) + cellsNum_y_frac;
		};
		inline const size_t getPoroNebr(const size_t frac_idx)
		{
			return (frac_idx - cellsNum_y_frac) / (cellsNum_y_frac + 1) * (cellsNum_y_poro + 2);
		};
		inline adouble getAverage(adouble p1, const double l1, adouble p2, const double l2) const
		{
			return (p1 * (adouble)l2 + p2 * (adouble)l1) / (adouble)(l1 + l2);
		};
		inline adouble getPoroTrans(const PoroCell& cell, const PoroTapeVariable& next,	const PoroCell& beta, const PoroTapeVariable& nebr) const
		{
			adouble k1, k2;
			const auto& props = props_sk.back();
			k1 = props.getPermCoseni(next.m, next.p);
			k2 = props.getPermCoseni(nebr.m, nebr.p);
			if (k1 == 0.0 && k2 == 0.0)
				return 0.0;

			if (fabs(cell.x - beta.x) > EQUALITY_TOLERANCE)
				return 2.0 * k1 * k2 * cell.hy * cell.hz / (k1 * beta.hx + k2 * cell.hx);
			if (fabs(cell.y - beta.y) > EQUALITY_TOLERANCE)
				return 2.0 * k1 * k2 * cell.hx * cell.hz / (k1 * beta.hy + k2 * cell.hy);
			if (fabs(cell.z - beta.z) > EQUALITY_TOLERANCE)
				return 2.0 * k1 * k2 * cell.hx * cell.hy / (k1 * beta.hz + k2 * cell.hz);
		};
		inline adouble getReactionRate(const PoroTapeVariable& var, const Skeleton_Props& props) const
		{
			adouble m = props.getPoro(var.m, var.p);
			return var.sw * props_w.getDensity(var.p, var.xa, var.xw, var.xs) *
				(var.xa - props.xa_eqbm) *
				reac.getReactionRate(props.m_init, m) / reac.comps[REACTS::ACID].mol_weight;
		};
		inline const int getFracOut(const int idx) const
		{
			return int(idx / (cellsNum_y_frac + 1)) * (cellsNum_y_frac + 1) + cellsNum_y_frac;
		};
		inline const int getFirstPoro(const int idx) const
		{;
			return int(idx / (cellsNum_y_frac + 1)) * (cellsNum_y_poro + 2);
		};
		inline adouble getFlowLeak(const FracCell& cell) const 
		{
			const int idx = getFirstPoro(cell.num);
			const auto& poro_cell = cells_poro[idx];
			assert(poro_cell.type == PoroType::WELL_LAT);
			const auto& poro_beta = cells_poro[idx + 1];
			const auto& next = x_poro[idx];
			const auto& nebr = x_poro[idx + 1];
			const auto& props = props_sk.back();
			double tmp1 = nebr.p.value();
			double tmp2 = next.p.value();
			assert(tmp1 - tmp2 <= 0);
			return -props.getPermCoseni(next.m, next.p) * props_w.getKr(next.sw, next.m, &props) /
				props_w.getViscosity(next.p, next.xa, next.xw, next.xs) / next.m / next.sw *
				(nebr.p - next.p) * 2.0 / (poro_cell.hy + poro_beta.hy);
		};
		/*inline double getFracDistance(const int idx1, const int idx2) const
		{
			const auto& cell1 = cells_frac[idx1];
			const auto& cell2 = cells_frac[idx2];
			return point::distance(cell1.c, cell2.c, (cell1.c + cell2.c) / 2.0);
		};*/
		/*inline adouble getFlowLeakNew(const FracCell& cell)
		{
			const auto& grid = poro_grids[frac2poro[cells_frac[getRowOuter(cell.num)].num]];
			const auto& beta = cells_frac[grid.frac_nebr->num];
			assert(beta.type == FracType::FRAC_OUT);
			
			const auto& next = x_poro[grid.start_idx];
			const auto& nebr = x_frac[beta.num];
			
			return -props_frac.w2 * props_frac.w2 / 3.0 / props_w.visc * (next.p - nebr.p) / (grid.cells[0].x - beta.y);
		}
		inline adouble getQuadAppr(const std::array<adouble, 3> p, const std::array<double, 3> x, const double cur_x) const
		{
			const double den = (x[0] - x[1]) * (x[0] - x[2]) * (x[1] - x[2]);
			adouble a = (	p[2] * (x[0] - x[1]) +
							p[0] * (x[1] - x[2]) + 
							p[1] * (x[2] - x[0])				) / den;
			adouble b = (	p[2] * (x[1] * x[1] - x[0] * x[0]) + 
							p[0] * (x[2] * x[2] - x[1] * x[1]) + 
							p[1] * (x[0] * x[0] - x[2] * x[2])	) / den;
			adouble c = (	p[2] * x[0] * x[1] * (x[0] - x[1]) +
							x[2] * (p[0] * x[1] * (x[1] - x[2]) + p[1] * x[0] * (x[2] - x[0]))) / den;
			return a * cur_x * cur_x + b * cur_x + c;
		};*/
		inline void getPoroNeighborIdx(const int cur, int* const neighbor)
		{
			neighbor[0] = cur - 1;
			neighbor[1] = cur + 1;
			neighbor[2] = cur - (cellsNum_y_poro + 2) * (cellsNum_z + 2);
			neighbor[3] = cur + (cellsNum_y_poro + 2) * (cellsNum_z + 2);
			neighbor[4] = cur - (cellsNum_y_poro + 2);
			neighbor[5] = cur + (cellsNum_y_poro + 2);
		};
		inline void getFracNeighborIdx(const int cur, int* const neighbor)
		{
			neighbor[0] = cur - 1;
			if(cells_frac[cur].type == FracType::FRAC_OUT)
				neighbor[1] = getPoroNebr(cur);
			else
				neighbor[1] = cur + 1;
			neighbor[2] = cur - (cellsNum_y_frac + 1) * (cellsNum_z + 2);
			neighbor[3] = cur + (cellsNum_y_frac + 1) * (cellsNum_z + 2);
			neighbor[4] = cur - (cellsNum_y_frac + 1);
			neighbor[5] = cur + (cellsNum_y_frac + 1);
		};
		inline double upwindIsCur(const PoroCell& cell, const PoroCell& beta)
		{
			if (cell.u_next.p < beta.u_next.p)
				return 0.0;
			else
				return 1.0;
		};
	public:
		// Dimensions
		double t_dim, R_dim, P_dim, T_dim, Q_dim, grav;

		AcidRecFrac();
		~AcidRecFrac();

		void load(Properties& props)
		{
			setProps(props);
			setSnapshotter("", this);
			buildFracGrid();
			buildPoroGrid();
			processGeometry();
			setPerforated();
			setInitialState();

			snapshotter->dump_all(0);
		};
		void snapshot_all(int i) { snapshotter->dump_all(i); }
		void setSnapshotter(std::string type, AcidRecFrac* model)
		{
			snapshotter = new VTKSnapshotter<AcidRecFrac>();
			snapshotter->setModel(model);
			isWriteSnaps = true;
		};

		void setPeriod(int period);

		adouble* h;
		PoroTapeVariable* x_poro;
		FracTapeVariable* x_frac;

		int getCellsNum() { return cellsNum_frac + cellsNum_poro; };
		double getRate(int cur) const;
	};
};

#endif /* ACIDRECFRACMODEL_HPP_ */