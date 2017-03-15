#ifndef ACID2D_HPP_
#define ACID2D_HPP_

#include <vector>

#include "model/Acid/2d/Properties.hpp"
#include "model/cells/Variables.hpp"
#include "model/cells/CylCell2D.h"
#include "model/Basic2d/Basic2d.hpp"
#include "util/Interpolate.h"

namespace acid2d
{
	static const int R_AXIS = 0;
	static const int Z_AXIS = 1;

	static const int stencil = basic2d::stencil;
	static const int Lstencil = basic2d::Lstencil;
	static const int Rstencil = basic2d::Rstencil;
	static const int Vstencil = basic2d::Vstencil;

	typedef VarSimpleAcid Variable;
	typedef TapeVarSimpleAcid TapeVariable;
	typedef NewCylCell2D<VarSimpleAcid, Skeleton_Props> Cell;
	template <typename TVariable> using TCell = NewCylCell2D<TVariable, Skeleton_Props>;
	typedef CalciteReaction CurrentReaction;

	class Acid2d : public basic2d::Basic2d<Variable, Properties, Skeleton_Props, TCell, Acid2d>
	{
		template<typename> friend class Snapshotter;
		template<typename> friend class GRDECLSnapshotter;
		template<typename> friend class VTKSnapshotter;
		template<typename> friend class AbstractMethod;
		friend class Acid2dSolver;

	protected:

		CurrentReaction reac;
		std::vector<Skeleton_Props> props_sk;
		Liquid_Props props_l;
		Gas_Props props_g;

		void setProps(Properties& props);
		void makeDimLess();

		// Service functions
		inline double getTrans(const Cell& cell, const Cell& beta) const
		{
			double k1, k2, S;

			if (abs(cell.num - beta.num) == 1) {
				k1 = cell.props->getPermCoseni_z(cell.u_next.m).value();
				k2 = beta.props->getPermCoseni_z(beta.u_next.m).value();
				if (k1 == 0.0 && k2 == 0.0)
					return 0.0;
				S = 2.0 * M_PI * cell.r * cell.hr;
				return 2.0 * k1 * k2 * S / (k1 * beta.hz + k2 * cell.hz);
			}
			else {
				k1 = cell.props->getPermCoseni_r(cell.u_next.m).value();
				k2 = beta.props->getPermCoseni_r(beta.u_next.m).value();
				if (k1 == 0.0 && k2 == 0.0)
					return 0.0;
				S = 2.0 * M_PI * cell.hz * (cell.r + sign(beta.num - cell.num) * cell.hr / 2.0);
				return 2.0 * k1 * k2 * S / (k1 * beta.hr + k2 * cell.hr);
			}
		};
		inline double getLiquidVelocity(Cell& cell, int varNum, int axis)
		{
			Variable* var;
			switch (varNum)
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

			switch (axis)
			{
			case R_AXIS:
				return -cell.props->getPermCoseni_r(cell.u_next.m).value() * props_l.getKr(var->s, 0.0).value() / props_l.visc * getNablaP(cell, varNum, axis);
			case Z_AXIS:
				return -cell.props->getPermCoseni_z(cell.u_next.m).value() * props_l.getKr(var->s, 0.0).value() / props_l.visc * getNablaP(cell, varNum, axis);
			}
		};
		inline double getGasVelocity(Cell& cell, int varNum, int axis)
		{
			Variable* var;
			switch (varNum)
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

			switch (axis)
			{
			case R_AXIS:
				return -cell.props->getPermCoseni_r(cell.u_next.m).value() * props_l.getKr(var->s, 0.0).value() / props_g.visc * getNablaP(cell, varNum, axis);
			case Z_AXIS:
				return -cell.props->getPermCoseni_z(cell.u_next.m).value() * props_g.getKr(var->s, 0.0).value() / props_g.visc * getNablaP(cell, varNum, axis);
			}
		};

		void solve_eqMiddle(const Cell& cell);
		void solve_eqLeft(const Cell& cell);
		void solve_eqRight(const Cell& cell);
		void solve_eqVertical(const Cell& cell);

	public:
		Acid2d();
		~Acid2d();

		double* x;
		double* y;
		double** jac;

		//double getRate(int cur);
	};
};

#endif /* ACID2D_HPP_ */