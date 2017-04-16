#ifndef GASOILNITELLIPTIC_HPP_
#define GASOILNITELLIPTIC_HPP_

#include <vector>
#include <map>
#include <string>

#define ADOLC_ADVANCED_BRANCHING

#include "model/cells/Variables.hpp"
#include "model/GasOilNIT_Elliptic/Properties.hpp"
#include "model/cells/EllipticCell.hpp"
#include "model/AbstractModel.hpp"

#include <cassert>

namespace gasOilnit_elliptic
{
	static const int MU_AXIS = 0;
	static const int NU_AXIS = 1;
	static const int Z_AXIS = 2;

	static const int stencil = 7;
	static const int Lstencil = 3;
	static const int Rstencil = 2;
	static const int Vstencil = 2;

	typedef Var2phaseNIT Variable;
	typedef TapeVarGasOil TapeVariable;
	typedef EllipticCell<Variable, Skeleton_Props> Cell;
	typedef Cell::Point Point;
	typedef Cell::Type Type;
	template <typename TVariable> using TCell = EllipticCell<TVariable, Skeleton_Props>;

	class GasOilNIT_Elliptic : public AbstractModel<Variable, Properties, TCell, GasOilNIT_Elliptic>
	{
		template<typename> friend class Snapshotter;
		template<typename> friend class GRDECLSnapshotter;
		template<typename> friend class VTKSnapshotter;
		template<typename> friend class AbstractSolver;
		friend class GasOilNITEllipticSolver;
	protected:

		// Continuum properties
		int skeletonsNum;
		std::vector<Skeleton_Props> props_sk;
		std::vector<Skeleton_Props>::iterator sk_well;
		Oil_Props props_oil;
		Gas_Props props_gas;

		// Heat of phase transition [J/kg]
		double L;

		double l;
		int cellsNum_mu;
		int cellsNum_nu;
		int cellsNum_z;

		void setInitialState();
		void setProps(Properties& props);
		void makeDimLess();
		void buildGridLog();
		void setPerforated();
		void setUnused();
		void buildWellCells();
		void setRateDeviation(int num, double ratio);
		double solveH();

		std::vector<Cell> wellCells;
		std::map<int, int> wellNebrMap;
		std::map<int, std::pair<int, int> > nebrMap;

		inline const Cell& getUpwindCell(const Cell& cell, const Cell& nebr) const
		{
			assert(cell.isUsed);
			assert(nebr.isUsed);
			if (cell.u_next.p < nebr.u_next.p)
				return nebr;
			else
				return cell;
		};
		inline int getSkeletonIdx(const Cell& cell) const
		{
			int idx = 0;
			while (idx < props_sk.size())
			{
				if (cell.z <= props_sk[idx].h2 + EQUALITY_TOLERANCE)
					return idx;
				idx++;
			}
			exit(-1);
		};
		inline Cell& getCell(const int num)
		{
			return cells[num];
		};
		inline Cell& getCell(const int num, const int beta)
		{
			Cell& nebr = cells[beta];
			if (nebr.isUsed)
				return nebr;
			else
				return wellCells[wellNebrMap.at(num)];
		};
		inline int getCellIdx(const int num, const int beta)
		{
			Cell& nebr = cells[beta];
			if (nebr.isUsed)
				return beta;
			else
				return cellsNum + wellNebrMap.at(num);
		};
		inline const Cell& getCell(const int num) const
		{
			return cells[num];
		};
		inline const Cell& getCell(const int num, const int beta) const
		{
			const Cell& nebr = cells[beta];
			if (nebr.isUsed)
				return nebr;
			else
				return wellCells[wellNebrMap.at(num)];
		};
		inline void getNeighbors(const Cell& cell, Cell** const neighbor)
		{
			// FIXME: Only for inner cells
			assert(cell.isUsed);

			switch (cell.type)
			{
			case Type::MIDDLE:
				// Special neighbor search for center cells
				if (cell.num % ((cellsNum_mu + 2) * (cellsNum_z + 2)) > cellsNum_z + 1)
					neighbor[0] = &getCell(cell.num, cell.num - cellsNum_z - 2);
				else
				{
					int nu_idx = cell.num / ((cellsNum_z + 2) * (cellsNum_mu + 2));
					neighbor[0] = &getCell(cell.num, cellsNum + cell.num - (2 * nu_idx + 1) * (cellsNum_z + 2) * (cellsNum_mu + 2));
				}
				neighbor[1] = &getCell(cell.num, cell.num + cellsNum_z + 2);
				neighbor[2] = &getCell(cell.num, cell.num - 1);
				neighbor[3] = &getCell(cell.num, cell.num + 1);

				if (cell.num < (cellsNum_mu + 2) * (cellsNum_z + 2))
					neighbor[4] = &getCell(cell.num, cell.num + (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1));
				else
					neighbor[4] = &getCell(cell.num, cell.num - (cellsNum_mu + 2) * (cellsNum_z + 2));
				if (cell.num < (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1))
					neighbor[5] = &getCell(cell.num, cell.num + (cellsNum_mu + 2) * (cellsNum_z + 2));
				else
					neighbor[5] = &getCell(cell.num, cell.num - (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1));
				break;

			case Type::MIDDLE_SIDE:
				neighbor[0] = &getCell(cell.num, cell.num + cellsNum_z + 2);
				neighbor[1] = &getCell(cell.num, cell.num - 1);
				neighbor[2] = &getCell(cell.num, cell.num + 1);

				if (cell.num < (cellsNum_mu + 2) * (cellsNum_z + 2))
					neighbor[3] = &getCell(cell.num, cell.num + (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1));
				else
					neighbor[3] = &getCell(cell.num, cell.num - (cellsNum_mu + 2) * (cellsNum_z + 2));
				if (cell.num < (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1))
					neighbor[4] = &getCell(cell.num, cell.num + (cellsNum_mu + 2) * (cellsNum_z + 2));
				else
					neighbor[4] = &getCell(cell.num, cell.num - (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1));
				break;

			case Type::RIGHT:
				neighbor[0] = &getCell(cell.num - cellsNum_z - 2);
				break;

			case Type::TOP:
				neighbor[0] = &getCell(cell.num + 1);
				break;

			case Type::BOTTOM:
				neighbor[0] = &getCell(cell.num - 1);
				break;

			case Type::WELL_LAT:
				neighbor[0] = &cells[nebrMap[cell.num].first];
				neighbor[1] = &cells[nebrMap[cell.num].second];
				break;

			case Type::WELL_TOP:
				neighbor[0] = &cells[nebrMap[cell.num].first];
				neighbor[1] = &cells[nebrMap[cell.num].second];
				break;

			case Type::WELL_BOT:
				neighbor[0] = &cells[nebrMap[cell.num].first];
				neighbor[1] = &cells[nebrMap[cell.num].second];
				break;
			}
		};
		inline void getStencil(const Cell& cell, Cell** const neighbor)
		{
			// FIXME: Only for inner cells
			assert(cell.isUsed);
			switch (cell.type)
			{
			case Type::MIDDLE:
				neighbor[0] = &getCell(cell.num);
				// Special neighbor search for center cells
				if (cell.num % ((cellsNum_mu + 2) * (cellsNum_z + 2)) > cellsNum_z + 1)
					neighbor[1] = &getCell(cell.num, cell.num - cellsNum_z - 2);
				else
				{
					int nu_idx = cell.num / ((cellsNum_z + 2) * (cellsNum_mu + 2));
					neighbor[1] = &getCell(cell.num, cellsNum + cell.num - (2 * nu_idx + 1) * (cellsNum_z + 2) * (cellsNum_mu + 2));
				}
				neighbor[2] = &getCell(cell.num, cell.num + cellsNum_z + 2);
				neighbor[3] = &getCell(cell.num, cell.num - 1);
				neighbor[4] = &getCell(cell.num, cell.num + 1);

				if (cell.num < (cellsNum_mu + 2) * (cellsNum_z + 2))
					neighbor[5] = &getCell(cell.num, cell.num + (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1));
				else
					neighbor[5] = &getCell(cell.num, cell.num - (cellsNum_mu + 2) * (cellsNum_z + 2));
				if (cell.num < (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1))
					neighbor[6] = &getCell(cell.num, cell.num + (cellsNum_mu + 2) * (cellsNum_z + 2));
				else
					neighbor[6] = &getCell(cell.num, cell.num - (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1));
				break;

			case Type::MIDDLE_SIDE:
				neighbor[0] = &getCell(cell.num);
				neighbor[1] = &getCell(cell.num, cell.num + cellsNum_z + 2);
				neighbor[2] = &getCell(cell.num, cell.num - 1);
				neighbor[3] = &getCell(cell.num, cell.num + 1);

				if (cell.num < (cellsNum_mu + 2) * (cellsNum_z + 2))
					neighbor[4] = &getCell(cell.num, cell.num + (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1));
				else
					neighbor[4] = &getCell(cell.num, cell.num - (cellsNum_mu + 2) * (cellsNum_z + 2));
				if (cell.num < (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1))
					neighbor[5] = &getCell(cell.num, cell.num + (cellsNum_mu + 2) * (cellsNum_z + 2));
				else
					neighbor[5] = &getCell(cell.num, cell.num - (cellsNum_mu + 2) * (cellsNum_z + 2) * (cellsNum_nu - 1));
				break;

			case Type::RIGHT:
				neighbor[0] = &getCell(cell.num);
				neighbor[1] = &getCell(cell.num - cellsNum_z - 2);
				break;

			case Type::TOP:
				neighbor[0] = &getCell(cell.num);
				neighbor[1] = &getCell(cell.num + 1);
				break;

			case Type::BOTTOM:
				neighbor[0] = &getCell(cell.num);
				neighbor[1] = &getCell(cell.num - 1);
				break;

			case Type::WELL_LAT:
				neighbor[0] = &wellCells[cell.num];
				neighbor[1] = &cells[nebrMap[cell.num].first];
				neighbor[2] = &cells[nebrMap[cell.num].second];
				break;

			case Type::WELL_TOP:
				neighbor[0] = &wellCells[cell.num];
				neighbor[1] = &cells[nebrMap[cell.num].first];
				neighbor[2] = &cells[nebrMap[cell.num].second];
				break;

			case Type::WELL_BOT:
				neighbor[0] = &wellCells[cell.num];
				neighbor[1] = &cells[nebrMap[cell.num].first];
				neighbor[2] = &cells[nebrMap[cell.num].second];
				break;
			}
		};
		inline double getTrans(const Cell& cell, const Cell& beta) const
		{
			double k1, k2, S;

			if (fabs(cell.z - beta.z) > EQUALITY_TOLERANCE) {
				const double dz1 = fabs(cell.z - sk_well->h_well);
				const double dz2 = fabs(beta.z - sk_well->h_well);
				k1 = (dz1 > cell.props->radius_eff_z ? cell.props->perm_z : cell.props->perm_eff_z);
				k2 = (dz2 > beta.props->radius_eff_z ? beta.props->perm_z : beta.props->perm_eff_z);
				if (k1 == 0.0 && k2 == 0.0)
					return 0.0;
				S = Cell::getH(cell.mu, cell.nu) * Cell::getH(cell.mu, cell.nu) * cell.hmu * cell.hnu;
				return 2.0 * k1 * k2 * S / (k1 * beta.hz + k2 * cell.hz);
			}
			else if (fabs(cell.mu - beta.mu) > EQUALITY_TOLERANCE) {
				k1 = (cell.mu > cell.props->radius_eff_mu ? cell.props->perm_mu : cell.props->perm_eff_mu);
				k2 = (beta.mu > beta.props->radius_eff_mu ? beta.props->perm_mu : beta.props->perm_eff_mu);
				if (k1 == 0.0 && k2 == 0.0)
					return 0.0;

				const double mu = cell.mu + sign(beta.mu - cell.mu) * cell.hmu / 2.0;
				S = Cell::getH(mu, cell.nu) * cell.hnu * cell.hz;
				return 2.0 * k1 * k2 * S /
					(k1 * beta.hmu * Cell::getH(beta.mu, beta.nu)
						+ k2 * cell.hmu * Cell::getH(cell.mu, cell.nu));
			}
			else if (fabs(cell.nu - beta.nu) > EQUALITY_TOLERANCE) {
				k1 = (cell.mu > cell.props->radius_eff_mu ? cell.props->perm_mu : cell.props->perm_eff_mu);
				if (k1 == 0.0)
					return 0.0;
				const double nu = cell.nu + sign(beta.nu - cell.nu) * cell.hnu / 2.0;
				S = cell.hz * cell.hmu * Cell::getH(cell.mu, nu);
				return 2.0 * k1 * S /
					(beta.hnu * Cell::getH(beta.mu, beta.nu) +
						+cell.hnu * Cell::getH(cell.mu, cell.mu));
			}
		};
		inline void solveP_bub()
		{
			int idx;

			for (auto& cell : cells)
			{
				const Skeleton_Props* props = cell.props;
				Variable& next = cell.u_next;

				if (next.SATUR)
				{
					if ((next.s > 1.0 + EQUALITY_TOLERANCE) || (next.p > props_oil.p_sat))
					{
						next.SATUR = false;
						//							next.s = 1.0;
						next.p_bub -= 0.01 * next.p_bub;
					}
					else
						next.p_bub = next.p;
				}
				else
				{
					if (next.p_bub > next.p + EQUALITY_TOLERANCE)
					{
						next.SATUR = true;
						next.s -= 0.01;
						//							next.p_bub = next.p;
					}
					else
						next.s = 1.0;
				}
			}
		};

		inline double getPerm(const Cell& cell, const int axis) const
		{
			if (axis == MU_AXIS)
				return (cell.mu > cell.props->radius_eff_mu ? cell.props->perm_mu : cell.props->perm_eff_mu);
			else if (axis == NU_AXIS)
				return cell.props->perm_mu;
			else if (axis == Z_AXIS)
				return (fabs(cell.z - sk_well->h_well) > cell.props->radius_eff_z ? 
							cell.props->perm_z : 
							cell.props->perm_eff_z);
		};
		inline double getNablaP(Cell& cell, Cell** neighbor, const int axis)
		{
			Cell* nebr1;
			Cell* nebr2;
			double h, r_eff;

			if(axis == MU_AXIS)
			{
				if (cell.type == Type::MIDDLE_SIDE)
				{
					nebr1 = &cell;	nebr2 = neighbor[0];
				}
				else
				{
					nebr1 = neighbor[0];	nebr2 = neighbor[1];
					r_eff = cell.props->radius_eff_mu;
					if ((nebr1->mu < r_eff) && (nebr2->mu > r_eff))
					{
						if (cell.mu > r_eff)
							nebr1 = &cell;
						else
							nebr2 = &cell;
					}
				}

				h = Cell::getH(cell.mu, cell.nu) * (nebr2->mu - nebr1->mu);
			}
			else if (axis == NU_AXIS)
			{
				if (cell.type == Type::MIDDLE_SIDE)
				{
					nebr1 = neighbor[3];	nebr2 = neighbor[4];
				}
				else
				{
					nebr1 = neighbor[4];	nebr2 = neighbor[5];
				}

				if (abs(nebr2->nu - nebr1->nu) > 2.0 * cell.hnu + EQUALITY_TOLERANCE)
					h = Cell::getH(cell.mu, cell.nu) * (nebr2->nu - nebr1->nu + 2.0 * M_PI);
				else
					h = Cell::getH(cell.mu, cell.nu) * (nebr2->nu - nebr1->nu);
			}
			else if (axis == Z_AXIS)
			{
				if (cell.type == Type::MIDDLE_SIDE)
				{
					nebr1 = neighbor[1];	nebr2 = neighbor[2];
				}
				else
				{
					nebr1 = neighbor[2];	nebr2 = neighbor[3];

					r_eff = cell.props->radius_eff_z;
					const double dz_cur = fabs(cell.z - sk_well->h_well);
					const double dz1 = fabs(nebr1->z - sk_well->h_well);
					const double dz2 = fabs(nebr2->z - sk_well->h_well);
					if ((dz1 < r_eff) && (dz2 > r_eff))
					{
						if (dz_cur > r_eff)
							nebr1 = &cell;
						else
							nebr2 = &cell;
					}
				}

				h = nebr2->z - nebr1->z;
			}

			return (nebr2->u_next.p - nebr1->u_next.p) / h;
		};
		inline double getOilVelocity(Cell& cell, Cell** neighbor, const int axis)
		{
			return -getPerm(cell, axis) * props_oil.getKr(cell.u_next.s).value() /
						props_oil.getViscosity(cell.u_next.p).value() * getNablaP(cell, neighbor, axis);
		};
		inline double getGasVelocity(Cell& cell, Cell** neighbor, const int axis)
		{
			return -getPerm(cell, axis) * props_gas.getKr(cell.u_next.s).value() /
				props_gas.getViscosity(cell.u_next.p).value() * getNablaP(cell, neighbor, axis);
		};
		inline double getCn(const Cell& cell) const
		{
			return cell.props->getPoro(cell.u_next.p).value() *
						(cell.u_next.s * props_oil.getDensity(cell.u_next.p, cell.u_next.p_bub, cell.u_next.SATUR).value() * 
								props_oil.c +
						(1.0 - cell.u_next.s) * props_gas.getDensity(cell.u_next.p).value() * 
								props_gas.c) +
					(1.0 - cell.props->getPoro(cell.u_next.p).value()) * cell.props->getDensity(cell.u_next.p).value() * 
								cell.props->c;
		};
		inline double getAd(const Cell& cell) const
		{
			return cell.props->getPoro(cell.u_next.p).value() *
						(cell.u_next.s * props_oil.getDensity(cell.u_next.p, cell.u_next.p_bub, cell.u_next.SATUR).value() *
							props_oil.ad * props_oil.c +
						(1.0 - cell.u_next.s) * props_gas.getDensity(cell.u_next.p).value() *
							props_gas.ad * props_gas.c);
		};
		inline double getLambda(const Cell& cell, const int axis) const
		{
			if (axis == Z_AXIS)
				return cell.props->getPoro(cell.u_next.p).value() * 
							(cell.u_next.s * props_oil.lambda +	(1.0 - cell.u_next.s) * props_gas.lambda) +
						(1.0 - cell.props->getPoro(cell.u_next.p).value()) * cell.props->lambda_z;
			else
				return cell.props->getPoro(cell.u_next.p).value() *
					(cell.u_next.s * props_oil.lambda + (1.0 - cell.u_next.s) * props_gas.lambda) +
					(1.0 - cell.props->getPoro(cell.u_next.p).value()) * cell.props->lambda_r;
		};
		inline double getLambda(const Cell& cell1, const Cell& cell2) const
		{
			if (fabs(cell1.z - cell2.z) > EQUALITY_TOLERANCE) 
				return (cell1.hz * getLambda(cell2, Z_AXIS) + cell2.hz * getLambda(cell1, Z_AXIS)) / (cell1.hz + cell2.hz);
			else if (fabs(cell1.mu - cell2.mu) > EQUALITY_TOLERANCE)
				return (cell1.hmu * getLambda(cell2, MU_AXIS) + cell2.hmu * getLambda(cell1, MU_AXIS)) / 
					(cell1.hmu + cell2.hmu);
			else if (fabs(cell1.nu - cell2.nu) > EQUALITY_TOLERANCE)
				return (cell1.hnu * getLambda(cell2, NU_AXIS) + cell2.hnu * getLambda(cell1, NU_AXIS)) /
					(cell1.hnu + cell2.hnu);
		};
		inline double getJT(Cell& cell, Cell** neighbor, const int axis)
		{
			return props_oil.getDensity(cell.u_next.p, cell.u_next.p_bub, cell.u_next.SATUR).value() * 
						props_oil.c * props_oil.jt * getOilVelocity(cell, neighbor, axis) +
					props_gas.getDensity(cell.u_next.p).value() * 
						props_gas.c * props_gas.jt * getGasVelocity(cell, neighbor, axis);
		};
		inline double getA(Cell& cell, Cell** neighbor, const int axis)
		{
			return props_oil.getDensity(cell.u_next.p, cell.u_next.p_bub, cell.u_next.SATUR).value() * 
						props_oil.c * getOilVelocity(cell, neighbor, axis) +
					props_gas.getDensity(cell.u_next.p).value() * 
						props_gas.c * getGasVelocity(cell, neighbor, axis);
		};
		inline double getPhaseRate(const Cell& cell, Cell** neighbor) const
		{
			const Variable& next = cell.u_next;
			const Variable& prev = cell.u_prev;

			double H = 0.0;
			H = ( cell.props->getPoro(next.p).value() * next.s * props_oil.getDensity(next.p, next.p_bub, next.SATUR).value() - 
				cell.props->getPoro(next.p).value() * next.s * props_oil.getDensity(next.p, next.p_bub, next.SATUR).value() ) / ht;

			const int nebrNum = (cell.type == Type::MIDDLE) ? 6 : 5;
			for (int i = 0; i < nebrNum; i++)
			{
				const Cell& beta = *neighbor[i];
				const Variable& upwd = getUpwindCell(cell, beta).u_next;

				H += 1.0 / cell.V * getTrans(cell, beta) * (next.p - beta.u_next.p) *
					props_oil.getKr(upwd.s).value() / props_oil.getViscosity(upwd.p).value() * props_oil.getDensity(upwd.p, upwd.p_bub, upwd.SATUR).value();
			}

			return H;
		};

		void solve_eqMiddle(const Cell& cell);
		void solve_eqWell(const Cell& cell);
		void solve_eqRight(const Cell& cell);
		void solve_eqVertical(const Cell& cell);
		void setVariables(const Cell& cell);

	public:
		GasOilNIT_Elliptic();
		~GasOilNIT_Elliptic();

		double* x;
		double* y;
		double** jac;

		void setPeriod(int period);
		double getRate(int cur) const;

		static const int var_size = Variable::size - 1;
	};
};

#endif /* GASOILNITELLIPTIC_HPP_ */
