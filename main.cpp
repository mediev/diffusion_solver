#include <new>
#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <iostream>
#include <valarray>

#include "gtest/gtest.h"

#include "util/utils.h"
#include "method/mcmath.h"
#include "Scene.h"
#include "Scene_OMP.h"

#include "model/Oil1D/Oil1D.h"
#include "model/Gas1D/Gas1D.h"
#include "model/Gas1D/Gas1D_simple.h"
#include "model/Gas1D/Gas1DSolver.h"
#include "model/Oil1D_NIT/Oil1D_NIT.h"
#include "model/Oil_RZ/Oil_RZ.h"
#include "model/Oil_RZ_NIT/Oil_RZ_NIT.h"
#include "model/GasOil_RZ/GasOil_RZ.h"
#include "model/GasOil_RZ_NIT/GasOil_RZ_NIT.h"

#include "model/3D/GasOil_3D/GasOil_3D.h"
#include "model/3D/GasOil_3D_NIT/GasOil_3D_NIT.h"
#include "model/3D/Perforation/GasOil_Perf.h"
#include "model/3D/Perforation/Oil_Perf_NIT.h"
#include "model/3D/Perforation/GasOil_Perf_NIT.h"

using namespace std;

/*gasOil_rz_NIT::Properties* getProps()
{
	gasOil_rz_NIT::Properties* props = new gasOil_rz_NIT::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_z = 1;

	props->timePeriods.push_back(134640.0);
	props->timePeriods.push_back(321813.0);
	props->timePeriods.push_back(494420.0);
	props->timePeriods.push_back(674042.0);
	props->timePeriods.push_back(1965038.0);
	
	props->leftBoundIsRate = true;
	props->rightBoundIsPres = true;

	props->rates.push_back(0.0);
	props->rates.push_back(33.0);
	props->rates.push_back(10.7);
	props->rates.push_back(46.8599);
	props->rates.push_back(0.0);

	props->ht = 1000.0;
	props->ht_min = 100.0;
	props->ht_max  = 100000.0;
	
	props->alpha = 7200.0;

	props->perfIntervals.push_back( make_pair(1, 1) );

	props->r_w = 0.1524;
	props->r_e = 765.0;


	gasOil_rz_NIT::Skeleton_Props tmp;
	tmp.cellsNum_z = 1;
	tmp.m = 0.28;
	tmp.p_init = tmp.p_out = tmp.p_bub = 169.0 * 101325.0;
	tmp.s_init = 0.999;
	tmp.t_init = 302.058;
	tmp.h1 = 1500.0;
	tmp.h2 = 1508.0;
	tmp.height = 8.0;
	tmp.perm_r = 380.0;
	tmp.perm_z = 0.0;
	tmp.dens_stc = 2200.0;
	tmp.beta = 6.0 * 1.0e-10;
	
	tmp.skins.push_back(0.0);
	tmp.skins.push_back(3.6);
	tmp.skins.push_back(11.0);
	tmp.skins.push_back(5.6);
	tmp.skins.push_back(5.6);

	tmp.radiuses_eff.push_back(3.0);
	tmp.radiuses_eff.push_back(3.0);
	tmp.radiuses_eff.push_back(3.0);
	tmp.radiuses_eff.push_back(1.2);
	tmp.radiuses_eff.push_back(1.2);
	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda_r = tmp.lambda_z = 41.0;
	props->props_sk.push_back( tmp );

	props->depth_point = 1500.0;

	// Thermal properties
	props->props_oil.visc = 9.614;
	props->props_oil.b_bore = 1.1;
	props->props_oil.dens_stc = 855.0;
	props->props_oil.beta = 1.43 * 1.e-9;
	props->props_oil.jt = 4.3 * 1.e-6;
	props->props_oil.ad = 1.0 * 1.e-6;
	props->props_oil.c = 1880.0;
	props->props_oil.lambda = 0.16;

	props->props_gas.visc = 0.02833;
	props->props_gas.dens_stc = 0.8;
	props->props_gas.jt = -1.4 * 1.e-5;
	props->props_gas.ad = 1.0 * 1.e-6;
	props->props_gas.c = 3200.0;
	props->props_gas.lambda = 0.06;

	props->L = -50.0*1.e3;
	
	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/koil.txt");
	setDataFromFile(props->kr_gas, "props/kgas.txt");
	
	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/Boil.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/Rs.txt");

	return props;
}*/

/*gasOil_rz_NIT::Properties* getProps()
{
	gasOil_rz_NIT::Properties* props = new gasOil_rz_NIT::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_z = 1;

	props->timePeriods.push_back(134640.0);
	props->timePeriods.push_back(321813.0);
	props->timePeriods.push_back(494420.0);
	props->timePeriods.push_back(674042.0);
	props->timePeriods.push_back(1965038.0);
	
	props->leftBoundIsRate = true;
	props->rightBoundIsPres = true;

	props->rates.push_back(0.0);
	props->rates.push_back(33.0);
	props->rates.push_back(10.7);
	props->rates.push_back(46.8599);
	props->rates.push_back(0.0);

	props->ht = 1000.0;
	props->ht_min = 100.0;
	props->ht_max  = 100000.0;
	
	props->alpha = 7200.0;

	props->perfIntervals.push_back( make_pair(1, 1) );

	props->r_w = 0.1524;
	props->r_e = 765.0;


	gasOil_rz_NIT::Skeleton_Props tmp;
	tmp.cellsNum_z = 1;
	tmp.m = 0.28;
	tmp.p_init = tmp.p_out = tmp.p_bub = 169.0 * 101325.0;
	tmp.s_init = 0.999;
	tmp.t_init = 302.058;
	tmp.h1 = 1500.0;
	tmp.h2 = 1508.0;
	tmp.height = 8.0;
	tmp.perm_r = 380.0;
	tmp.perm_z = 0.0;
	tmp.dens_stc = 2200.0;
	tmp.beta = 6.0 * 1.0e-10;
	
	tmp.skins.push_back(0.0);
	tmp.skins.push_back(3.6);
	tmp.skins.push_back(11.0);
	tmp.skins.push_back(5.6);
	tmp.skins.push_back(5.6);

	tmp.radiuses_eff.push_back(0.5);
	tmp.radiuses_eff.push_back(0.5);
	tmp.radiuses_eff.push_back(0.5);
	tmp.radiuses_eff.push_back(0.5);
	tmp.radiuses_eff.push_back(0.5);
	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda_r = tmp.lambda_z = 6.0;
	props->props_sk.push_back( tmp );

	props->depth_point = 1500.0;

	// Thermal properties
	props->props_oil.visc = 9.614;
	props->props_oil.b_bore = 1.1;
	props->props_oil.dens_stc = 855.0;
	props->props_oil.beta = 1.43 * 1.e-9;
	props->props_oil.jt = 4.0 * 1.e-7;
	props->props_oil.ad = 2.1 * 1.e-7;
	props->props_oil.c = 1880.0;
	props->props_oil.lambda = 0.16;

	props->props_gas.visc = 0.02833;
	props->props_gas.dens_stc = 0.8;
	props->props_gas.jt = -1.6 * 1.e-6;
	props->props_gas.ad = 3.6 * 1.e-6;
	props->props_gas.c = 3200.0;
	props->props_gas.lambda = 0.06;

	props->L = -50.0*1.e3;
	
	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/koil.txt");
	setDataFromFile(props->kr_gas, "props/kgas.txt");
	
	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/Boil.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/Rs.txt");

	return props;
}*/

gasOil_rz_NIT::Properties* getProps()
{
	gasOil_rz_NIT::Properties* props = new gasOil_rz_NIT::Properties();

	// Number of cells in radial direction
	props->cellsNum_r = 500;
	// in vertical direction
	props->cellsNum_z = 3;

	// Time periods for current value of boundary condition
	props->timePeriods.push_back(86400.0 * 50.0);
	props->timePeriods.push_back(86400.0 * 100.0);

	// well BC type
	props->leftBoundIsRate = false;
	// reservoir BC type
	props->rightBoundIsPres = true;
	// well BC values
	props->pwf.push_back(60.0 * 1.e+5);
	props->pwf.push_back(200.0 * 1.e+5);

	// Initial time step
	props->ht = 0.1;
	props->ht_min = 0.01;
	props->ht_max = 100000.0;

	//props->alpha = 7200.0;
	//props->wellboreDuration = 86400.0;

	// Range of cells that are perforated
	props->perfIntervals.push_back(make_pair(1, 3));

	// Well radius
	props->r_w = 0.05;
	// Reservoir radius
	props->r_e = 1000.0;

	// Reservoir properties
	gasOil_rz_NIT::Skeleton_Props tmp;
	// Number of cells
	tmp.cellsNum_z = 3;
	// Porosity
	tmp.m = 0.1;
	// Pressures
	tmp.p_init = tmp.p_out = tmp.p_bub = 200.0 * 1.0e+5;
	// Initial temperature
	tmp.t_init = 0.0;
	// Initial oil saturation
	tmp.s_init = 0.999;
	// Depth 
	tmp.h1 = 0.0;
	tmp.h2 = 0.1;
	// Reservoir thickness
	tmp.height = 0.1;
	// Permeabilities
	tmp.perm_r = 500.0;
	tmp.perm_z = 0.0;// 500.0;
	// Density
	tmp.dens_stc = 2000.0;
	// Compressibility
	tmp.beta = 4.35113e-10;
	// Skins
	tmp.skins.push_back(0.0);
	tmp.skins.push_back(0.0);
	// Radiuses of damaged zone
	tmp.radiuses_eff.push_back(props->r_w);
	tmp.radiuses_eff.push_back(props->r_w);

	// thermal props
	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda_r = 6.0;
	tmp.lambda_z = 0.0;
	props->props_sk.push_back(tmp);

	//props->depth_point = 0.0;

	// viscosity
	props->props_oil.visc = 1.0;
	// volume factor for well
	props->props_oil.b_bore = 1.0;
	// Density
	props->props_oil.dens_stc = 736.0;
	// Compressibility
	props->props_oil.beta = 1.0 * 1.e-9;

	// Thermal properties of fluid
	props->props_oil.jt = 4.0 * 1.e-7;
	props->props_oil.ad = 2.0 * 1.e-7;
	props->props_oil.c = 1880.0;
	props->props_oil.lambda = 0.16;

	// Viscosity
	props->props_gas.visc = 0.01;
	props->props_gas.dens_stc = 1.0;
	
	props->props_gas.jt = -1.6 * 1.e-6;
	props->props_gas.ad = 2.0 * 1.e-6;
	props->props_gas.c = 3200.0;
	props->props_gas.lambda = 0.06;


	props->L = -50.0*1.e3;

	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/koil.txt");
	setDataFromFile(props->kr_gas, "props/kgas.txt");

	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/new/Boil100.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/new/Rs100.txt");

	return props;
}

/*gasOil_rz::Properties* getProps()
{
	gasOil_rz::Properties* props = new gasOil_rz::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_z = 5;

	props->timePeriods.push_back(1296000.0);
	props->timePeriods.push_back(2000000.0);
	
	props->leftBoundIsRate = true;
	props->rightBoundIsPres = true;
	props->rates.push_back(90.0);
	props->rates.push_back(0.0);
 
	props->skins.push_back(0.0); 
	props->skins.push_back(0.0); 

	props->radius.push_back(0.1524);
	props->radius.push_back(0.1524);

	props->ht = 10.0;
	props->ht_min = 1.0;
	props->ht_max  = 100000.0;
	
	props->alpha = 7200.0;

	props->perfIntervals.push_back( make_pair(1, 3) );

	props->r_w = 0.1524;
	props->r_e = 3000.0;

	props->p_init = 160.0 * 1.0e+5;
	props->p_out = 160.0 * 1.0e+5;
	props->p_sat = 160.0 * 1.0e+5;
	props->s_init = 0.999;
	props->h1 = 1500.0;
	props->h2 = 1505.0;
	props->depth_point = 1500.0;

	props->perm_r.reserve(props->cellsNum_z+2);
	props->perm_z.reserve(props->cellsNum_z+2);
	double perm = 50.0;
	for(int i = 0; i < props->cellsNum_z+2; i++)
	{
		/*if(i < 10)
			perm = 50.0;
		else
			perm = 1000.0;
		props->perm_r.push_back( perm );
		props->perm_z.push_back( 0.0 );//perm / 50.0 );
	}

	props->m = 0.01;
	props->visc_gas = 0.02833;
	props->visc_oil = 0.25137;
	props->b_oil_bore = 1.56;
	props->dens_oil_stc = 855.0;
	props->dens_gas_stc = 0.72275;
	props->dens_sk_stc = 2000.0;
	props->beta_oil = 1.282*1.E-9;
	props->beta_sk = 6.41*1.E-10;
	
	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/koil.txt");
	setDataFromFile(props->kr_gas, "props/kgas.txt");
	
	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/Boil.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/Rs.txt");

	return props;
}*/

/*gasOil_rz::Properties* getProps()
{
	gasOil_rz::Properties* props = new gasOil_rz::Properties();

	props->cellsNum_r = 50;
	props->cellsNum_z = 1;

	props->timePeriods.push_back(0.1 * 86400.0);
	props->timePeriods.push_back(0.2 * 86400.0);
	//props->timePeriods.push_back(15.0 * 86400.0);
	//props->timePeriods.push_back(30.0 * 86400.0);

	props->leftBoundIsRate = false;
	props->rightBoundIsPres = true;
	//props->rates.push_back(20.0);
	props->pwf.push_back(100.0 * 1.E+5);
	props->pwf.push_back(200.0 * 1.E+5);
	//props->pwf.push_back(100.0 * 1.E+5);
	//props->pwf.push_back(200.0 * 1.E+5);

	props->ht = 1.0;
	props->ht_min = 1.0;
	props->ht_max = 100000.0;

	props->alpha = 7200.0;

	props->r_w = 0.05;
	props->r_e = 1000.0;

	props->perfIntervals.push_back(make_pair(1, 1));

	gasOil_rz::Skeleton_Props tmp;
	tmp.cellsNum_z = 1;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = tmp.p_bub = 200.0 * 1.0e+5;
	tmp.s_init = 1.0;
	tmp.h1 = 0.0;
	tmp.h2 = 0.1;
	tmp.height = 0.1;
	tmp.perm_r = 100.0;
	tmp.perm_z = 5.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;

	tmp.skins.push_back(0.0);
	tmp.skins.push_back(0.0);
	//tmp.skins.push_back(0.0);
	//tmp.skins.push_back(0.0);

	tmp.radiuses_eff.push_back(props->r_w);
	tmp.radiuses_eff.push_back(props->r_w);
	//tmp.radiuses_eff.push_back(props->r_w);
	//tmp.radiuses_eff.push_back(props->r_w);

	props->props_sk.push_back(tmp);

	props->depth_point = 0.0;

	props->props_oil.visc = 1.64;
	props->props_oil.b_bore = 1.0;
	props->props_oil.dens_stc = 736.0;
	props->props_oil.beta = 1.0 * 1.e-9;

	props->props_gas.visc = 0.02833;
	props->props_gas.dens_stc = 0.8;

	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/koil.txt");
	setDataFromFile(props->kr_gas, "props/kgas.txt");

	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/Boil.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/Rs.txt");

	return props;
}*/

/*gasOil_rz::Properties* getProps()
{
	gasOil_rz::Properties* props = new gasOil_rz::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_z = 5;

	props->timePeriods.push_back(121993.0);
	props->timePeriods.push_back(321813.0);
	props->timePeriods.push_back(494420.0);
	props->timePeriods.push_back(674042.0);
	props->timePeriods.push_back(1965038.0);
	
	props->rates.push_back(0.0);
	props->rates.push_back(33.0);
	props->rates.push_back(10.7);
	props->rates.push_back(46.8599);
	props->rates.push_back(0.0);
 
	props->skins.push_back(0.0); 
	props->skins.push_back(3.5); 
	props->skins.push_back(11.0);
	props->skins.push_back(5.6);
	props->skins.push_back(5.6);

	props->radius.push_back(3.0);
	props->radius.push_back(3.0);
	props->radius.push_back(3.0);
	props->radius.push_back(1.2);
	props->radius.push_back(1.2);

	props->ht = 10.0;
	props->ht_min = 1.0;
	props->ht_max  = 100000.0;
	
	props->alpha = 7200.0;

	props->perfIntervals.push_back( make_pair(2, 4) );

	props->r_w = 0.1524;
	props->r_e = 765.0;

	props->p_init = 169.0 * 101325.0;
	props->p_sat = 169.0 * 101325.0;
	props->s_init = 0.999;
	props->h1 = 1500.0;
	props->h2 = 1513.33;
	props->depth_point = 1500.0;

	props->perm_r.reserve(props->cellsNum_z+2);
	props->perm_z.reserve(props->cellsNum_z+2);
	double perm = 380.0;
	for(int i = 0; i < props->cellsNum_z+2; i++)
	{
		/*if(i < 10)
			perm = 50.0;
		else
			perm = 1000.0;
		props->perm_r.push_back( perm );
		props->perm_z.push_back( perm / 5.0 );
	}

	props->m = 0.28;
	props->visc_gas = 0.02833;
	props->visc_oil = 9.614;
	props->b_oil_bore = 1.1;
	props->dens_oil_stc = 855.0;
	props->dens_gas_stc = 0.72275;
	props->dens_sk_stc = 2000.0;
	props->beta_oil = 1.0*1.E-9;
	props->beta_sk = 1.0*1.E-9;

	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/koil.txt");
	setDataFromFile(props->kr_gas, "props/kgas.txt");
	
	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/Boil.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/Rs.txt");

	return props;
}*/

/*oil_rz::Properties* getProps()
{
	oil_rz::Properties* props = new oil_rz::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_z = 5;

	props->timePeriods.push_back(10.0 * 365.0 * 86400.0);
	
	props->leftBoundIsRate = true;
	props->rightBoundIsPres = false;
	props->rates.push_back(100.0);

	props->ht = 1000.0;
	props->ht_min = 100.0;
	props->ht_max  = 5000000.0;
	
	props->alpha = 7200.0;

	props->perfIntervals.push_back( make_pair(1, 1) );
	props->perfIntervals.push_back( make_pair(3, 3) );
	props->perfIntervals.push_back( make_pair(5, 5) );

	props->r_w = 0.1;
	props->r_e = 2500.0;

	oil_rz::Skeleton_Props tmp;
	tmp.cellsNum_z = 1;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = 160.0 * 1.0e+5;
	tmp.h1 = 1500.0;
	tmp.h2 = 1530.0;
	tmp.height = 30.0;
	tmp.perm_r = 100.0;
	tmp.perm_z = 10.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	props->props_sk.push_back( tmp );

	tmp.cellsNum_z = 1;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = 0.0 * 1.0e+5;
	tmp.h1 = 1530.0;
	tmp.h2 = 1540.0;
	tmp.height = 10.0;
	tmp.perm_r = 0.0;
	tmp.perm_z = 0.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	props->props_sk.push_back( tmp );

	tmp.cellsNum_z = 1;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = 180.0 * 1.0e+5;
	tmp.h1 = 1540.0;
	tmp.h2 = 1560.0;
	tmp.height = 20.0;
	tmp.perm_r = 100.0;
	tmp.perm_z = 10.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	props->props_sk.push_back( tmp );

	tmp.cellsNum_z = 1;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = 0.0 * 1.0e+5;
	tmp.h1 = 1560.0;
	tmp.h2 = 1570.0;
	tmp.height = 10.0;
	tmp.perm_r = 0.0;
	tmp.perm_z = 0.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	props->props_sk.push_back( tmp );

	tmp.cellsNum_z = 1;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = 200.0 * 1.0e+5;
	tmp.h1 = 1570.0;
	tmp.h2 = 1600.0;
	tmp.height = 30.0;
	tmp.perm_r = 100.0;
	tmp.perm_z = 10.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	props->props_sk.push_back( tmp );

	props->depth_point = 1500.0;

	props->props_oil.visc = 0.62807;
	props->props_oil.b_bore = 1.09754;
	props->props_oil.dens_stc = 800.026;
	props->props_oil.beta = 1.24703e-09;

	return props;
}*/

/*oil_rz::Properties* getProps()
{
	oil_rz::Properties* props = new oil_rz::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_z = 30;

	props->timePeriods.push_back(10.0 * 365.0 * 86400.0);
	
	props->leftBoundIsRate = true;
	props->rightBoundIsPres = true;
	props->rates.push_back(50.0);
	//props->pwf.push_back(100.0 * 1.E+5);
	//props->pwf.push_back(231.5 * 1.E+5);

	props->ht = 1000.0;
	props->ht_min = 100.0;
	props->ht_max  = 5000000.0;
	
	props->alpha = 7200.0;

	props->perfIntervals.push_back( make_pair(1, 30) );

	props->r_w = 0.1;
	props->r_e = 1128.4;
	//props->r_e = 1128.4/10.0 + props->r_w;
	//props->r_e = 19.8;

	oil_rz::Skeleton_Props tmp;
	tmp.cellsNum_z = 30;
	tmp.m = 0.1;
	//tmp.p_init = tmp.p_out = 248.91 * 1.0e+5;
	tmp.p_init = tmp.p_out = 275.8 * 1.0e+5;
	tmp.h1 = 0.0;
	tmp.h2 = 10.0;
	tmp.height = 10.0;
	tmp.perm_r = 20.0;
	tmp.perm_z = 20.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 0.0;// 4.35113e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	props->props_sk.push_back( tmp );

	/*tmp.cellsNum_z = 10;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = 220.0 * 1.0e+5;
	tmp.h1 = 1.0;
	tmp.h2 = 2.0;
	tmp.height = 1.0;
	tmp.perm_r = 20.0;
	tmp.perm_z = 20.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 0.0;// 4.35113e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	props->props_sk.push_back(tmp);

	props->depth_point = 0.0;

	props->props_oil.visc = 1.0;
	props->props_oil.b_bore = 1.0;
	props->props_oil.dens_stc = 800.026;
	props->props_oil.beta = 0.0;//5.8e-10;

	return props;
}*/

/*gasOil_rz_NIT::Properties* getProps()
{
	gasOil_rz_NIT::Properties* props = new gasOil_rz_NIT::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_z = 1;

	props->timePeriods.push_back(100.0 * 86400.0);
	//props->timePeriods.push_back(40.0 * 86400.0);
	//props->timePeriods.push_back(40.0 * 86400.0);
	//props->timePeriods.push_back(1000.0 * 3600.0);
	
	props->leftBoundIsRate = true;
	props->rightBoundIsPres = true;

	props->rates.push_back(100.0);
	//props->rates.push_back(0.0);
	//props->rates.push_back(0.0);
	//props->pwf.push_back(180.0 * 100000.0);
	//props->pwf.push_back(200.0 * 100000.0);

	props->ht = 0.01;
	props->ht_min = 0.01;
	props->ht_max  = 1000000.0;
	
	props->alpha = 7200.0;

	props->perfIntervals.push_back( make_pair(1, 1) );

	props->r_w = 0.05;
	props->r_e = 1000.0;

	gasOil_rz_NIT::Skeleton_Props tmp;
	tmp.cellsNum_z = 1;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = tmp.p_bub = 300.0 * 100000.0;
	tmp.s_init = 0.999;
	tmp.t_init = 0.0;
	tmp.h1 = 0.0;
	tmp.h2 = 10.0;
	tmp.height = 10.0;
	tmp.perm_r = 100.0;
	tmp.perm_z = 100.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 6.0 * 1.0e-10;
	
	tmp.skins.push_back(5.0);
	//tmp.skins.push_back(0.0);
	//tmp.skins.push_back(0.0);

	tmp.radiuses_eff.push_back( 8.0 * props->r_w );
	//tmp.radiuses_eff.push_back(1.0 * props->r_w);
	//tmp.radiuses_eff.push_back( 1.0 * props->r_w );

	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda_r = tmp.lambda_z = 5.0;
	props->props_sk.push_back( tmp );

	props->depth_point = 0.0;

	// Thermal properties
	props->props_oil.visc = 1.0;
	props->props_oil.b_bore = 1.0;
	props->props_oil.dens_stc = 736.0;
	props->props_oil.beta = 1.0 * 1.e-9;
	props->props_oil.jt = 4.0 * 1.e-7;
	props->props_oil.ad = 2.1 * 1.e-7;
	props->props_oil.c = 1880.0;
	props->props_oil.lambda = 0.16;

	props->props_gas.visc = 0.03;
	props->props_gas.dens_stc = 0.8;
	props->props_gas.jt = -1.6 * 1.e-6;
	props->props_gas.ad = 3.6 * 1.e-6;
	props->props_gas.c = 3200.0;
	props->props_gas.lambda = 0.06;

	props->L = -50.0*1.e3;
	
	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/new/koil.txt");
	setDataFromFile(props->kr_gas, "props/new/kgas0.txt");
	
	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/new/Boil200.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/new/Rs200.txt");

	return props;
}*/

/*gas1D::Properties* getProps()
{
	gas1D::Properties* props = new gas1D::Properties();

	props->cellsNum_r = 100;

	props->timePeriods.push_back(10.0 * 365.0 * 86400.0);
	
	props->leftBoundIsRate = false;
	props->rightBoundIsPres = true;
	//props->rates.push_back(5000.0);
	props->pwf.push_back(100.0 * 100000.0);

	props->ht = 20000.0;
	props->ht_min = 10000.0;
	props->ht_max  = 10000000.0;
	
	props->alpha = 7200.0;

	props->perfIntervals.push_back( make_pair(0, 0) );

	props->r_w = 0.1;
	props->r_e = 1000.0;

	gas1D::Skeleton_Props tmp;
	tmp.cellsNum_z = 1;
	tmp.m = 0.2;
	tmp.p_init = tmp.p_out = 150.0 * 1.0e+5;
	tmp.h1 = 1500.0;
	tmp.h2 = 1510.0;
	tmp.height = 10.0;
	tmp.perm_r = 10.0 / 0.986923;
	tmp.perm_z = 0.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 0.0;//3.0e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	props->props_sk.push_back( tmp );

	props->depth_point = 1500.0;

	//props->props_gas.visc = 0.01;

	// Defining relative permeabilities
	setDataFromFile(props->z_factor, "props/z_real.txt");
	setDataFromFile(props->visc_gas, "props/gas_visc_real.txt");

	return props;
}*/

/*gasOil_3d_NIT::Properties* getProps()
{
	gasOil_3d_NIT::Properties* props = new gasOil_3d_NIT::Properties();
	
	props->cellsNum_r = 30;
	props->cellsNum_phi = 30;
	props->cellsNum_z = 35;

	props->timePeriods.push_back(1000.0 * 3600.0);
	
	props->leftBoundIsRate = true;
	props->rightBoundIsPres = true;
	props->rates.push_back(1.0);

	props->ht = 10.0;
	props->ht_min = 10.0;
	props->ht_max  = 5000000.0;
	
	props->alpha = 7200.0;

	props->perfIntervals.push_back( make_pair(16, 16) );
	props->perfIntervals.push_back( make_pair(17+6*(props->cellsNum_r+2)*(props->cellsNum_z+2), 17+6*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(18+12*(props->cellsNum_r+2)*(props->cellsNum_z+2), 18+12*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(19+18*(props->cellsNum_r+2)*(props->cellsNum_z+2), 19+18*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(20+24*(props->cellsNum_r+2)*(props->cellsNum_z+2), 20+24*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );

	/*props->perfIntervals.push_back( make_pair(6+10*(props->cellsNum_r+2)*(props->cellsNum_z+2), 6+10*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(8+15*(props->cellsNum_r+2)*(props->cellsNum_z+2), 8+15*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(10+20*(props->cellsNum_r+2)*(props->cellsNum_z+2), 10+20*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(12+25*(props->cellsNum_r+2)*(props->cellsNum_z+2), 12+25*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(14+30*(props->cellsNum_r+2)*(props->cellsNum_z+2), 14+30*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(16+35*(props->cellsNum_r+2)*(props->cellsNum_z+2), 16+35*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(18+40*(props->cellsNum_r+2)*(props->cellsNum_z+2), 18+40*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(20+45*(props->cellsNum_r+2)*(props->cellsNum_z+2), 20+45*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(22+50*(props->cellsNum_r+2)*(props->cellsNum_z+2), 22+50*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(24+55*(props->cellsNum_r+2)*(props->cellsNum_z+2), 24+55*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(26+60*(props->cellsNum_r+2)*(props->cellsNum_z+2), 26+60*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(28+65*(props->cellsNum_r+2)*(props->cellsNum_z+2), 28+65*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(30+70*(props->cellsNum_r+2)*(props->cellsNum_z+2), 30+70*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(32+75*(props->cellsNum_r+2)*(props->cellsNum_z+2), 32+75*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(34+80*(props->cellsNum_r+2)*(props->cellsNum_z+2), 34+80*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(36+85*(props->cellsNum_r+2)*(props->cellsNum_z+2), 36+85*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(38+90*(props->cellsNum_r+2)*(props->cellsNum_z+2), 38+90*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );
	props->perfIntervals.push_back( make_pair(36+95*(props->cellsNum_r+2)*(props->cellsNum_z+2), 40+95*(props->cellsNum_r+2)*(props->cellsNum_z+2)) );

	props->r_w = 0.1;
	props->r_e = 500.0;

	gasOil_3d_NIT::Skeleton_Props tmp;
	tmp.cellsNum_z = 5;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = tmp.p_bub = 250.0 * 1.0e+5;
	tmp.s_init = 0.999;
	tmp.t_init = 320.0;
	tmp.h1 = 1500.0;
	tmp.h2 = 1500.25;
	tmp.height = 0.25;
	tmp.perm_r = 100.0;
	tmp.perm_z = 5.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda_r = tmp.lambda_z = 5.0;
	props->props_sk.push_back( tmp );

	tmp.cellsNum_z = 25;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = tmp.p_bub = 250.0 * 1.0e+5;
	tmp.s_init = 0.999;
	tmp.t_init = 320.0;
	tmp.h1 = 1500.25;
	tmp.h2 = 1500.75;
	tmp.height = 0.5;
	tmp.perm_r = 100.0;
	tmp.perm_z = 5.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda_r = tmp.lambda_z = 5.0;
	props->props_sk.push_back( tmp );

	tmp.cellsNum_z = 5;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = tmp.p_bub = 250.0 * 1.0e+5;
	tmp.s_init = 0.999;
	tmp.t_init = 320.0;
	tmp.h1 = 1500.75;
	tmp.h2 = 1501.0;
	tmp.height = 0.25;
	tmp.perm_r = 100.0;
	tmp.perm_z = 5.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);
	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda_r = tmp.lambda_z = 5.0;
	props->props_sk.push_back( tmp );

	props->depth_point = 1500.0;

	props->props_oil.visc = 1.64;
	props->props_oil.b_bore = 1.245;
	props->props_oil.dens_stc = 736.0;
	props->props_oil.beta = 1.0 * 1.e-9;
	props->props_oil.jt = 4.0 * 1.e-7;
	props->props_oil.ad = 2.1 * 1.e-7;
	props->props_oil.c = 2200.0;
	props->props_oil.lambda = 0.16;

	props->props_gas.visc = 0.02833;
	props->props_gas.dens_stc = 0.72275;

	props->props_gas.visc = 0.02833;
	props->props_gas.dens_stc = 0.8;
	props->props_gas.jt = -1.73 * 1.e-6;
	props->props_gas.ad = 3.76 * 1.e-6;
	props->props_gas.c = 3400.0;
	props->props_gas.lambda = 0.06;

	props->L = -50.0*1.e3;

	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/koil.txt");
	setDataFromFile(props->kr_gas, "props/kgas.txt");
	
	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/Boil.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/Rs.txt");

	return props;
}*/

/*gasOil_perf::Properties* getProps()
{
	gasOil_perf::Properties* props = new gasOil_perf::Properties();

	props->cellsNum_r = 50;
<<<<<<< Updated upstream
	props->cellsNum_phi = 6;
=======
	props->cellsNum_phi = 8;
>>>>>>> Stashed changes
	props->cellsNum_z = 5;

	props->timePeriods.push_back(0.1 * 86400.0);
	props->timePeriods.push_back(0.2 * 86400.0);
	//props->timePeriods.push_back(15.0 * 86400.0);
	//props->timePeriods.push_back(30.0 * 86400.0);

	props->leftBoundIsRate = false;
	props->rightBoundIsPres = true;
	//props->rates.push_back(20.0);
	props->pwf.push_back(100.0 * 1.E+5);
	props->pwf.push_back(200.0 * 1.E+5);
	//props->pwf.push_back(100.0 * 1.E+5);
	//props->pwf.push_back(200.0 * 1.E+5);

<<<<<<< Updated upstream
	props->ht = 10.0;
	props->ht_min = 10.0;
=======
	props->ht = 500.0;
	props->ht_min = 100.0;
>>>>>>> Stashed changes
	props->ht_max = 100000.0;

	props->alpha = 7200.0;

<<<<<<< Updated upstream
	props->perfTunnels.push_back( make_pair(3, 0) );
	//props->perfTunnels.push_back(make_pair(3 + 1 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(3 + 2 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(3 + 3 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(3 + 4 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(3 + 5 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
=======
	props->perfTunnels.push_back( make_pair(3, 10) );
	props->perfTunnels.push_back(make_pair(3 + 2 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 10));
	props->perfTunnels.push_back(make_pair(3 + 4 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 10));
	props->perfTunnels.push_back(make_pair(3 + 6 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 10));
>>>>>>> Stashed changes

	props->r_w = 0.05;
	props->r_e = 1000.0;

	gasOil_perf::Skeleton_Props tmp;
	tmp.cellsNum_z = 5;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = tmp.p_bub = 200.0 * 1.0e+5;
	tmp.s_init = 1.0;
	tmp.h1 = 0.0;
	tmp.h2 = 0.1;
	tmp.height = 0.1;
	tmp.perm_r = 100.0;
	tmp.perm_z = 5.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;

	tmp.skins.push_back(0.0);
	tmp.skins.push_back(0.0);
	//tmp.skins.push_back(0.0);
	//tmp.skins.push_back(0.0);
	
	tmp.radiuses_eff.push_back(props->r_w);
	tmp.radiuses_eff.push_back(props->r_w);
	//tmp.radiuses_eff.push_back(props->r_w);
	//tmp.radiuses_eff.push_back(props->r_w);

	props->props_sk.push_back(tmp);

	props->depth_point = 0.0;

	props->props_oil.visc = 1.64;
	props->props_oil.b_bore = 1.0;
	props->props_oil.dens_stc = 736.0;
	props->props_oil.beta = 1.0 * 1.e-9;

	props->props_gas.visc = 0.02833;
	props->props_gas.dens_stc = 0.8;

	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/koil.txt");
	setDataFromFile(props->kr_gas, "props/kgas.txt");

	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/Boil.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/Rs.txt");

	return props;
}*/

/*gasOil_3d::Properties* getProps()
{
	gasOil_3d::Properties* props = new gasOil_3d::Properties();

	props->cellsNum_r = 50;
	props->cellsNum_phi = 40;
	props->cellsNum_z = 5;

	props->timePeriods.push_back(0.1 * 86400.0);
	props->timePeriods.push_back(0.2 * 86400.0);
	//props->timePeriods.push_back(15.0 * 86400.0);
	//props->timePeriods.push_back(30.0 * 86400.0);

	props->leftBoundIsRate = false;
	props->rightBoundIsPres = true;
	//props->rates.push_back(20.0);
	props->pwf.push_back(180.0 * 1.E+5);
	props->pwf.push_back(200.0 * 1.E+5);
	//props->pwf.push_back(100.0 * 1.E+5);
	//props->pwf.push_back(200.0 * 1.E+5);

	props->ht = 100.0;
	props->ht_min = 1.0;
	props->ht_max = 100000.0;

	props->alpha = 7200.0;

	props->perfIntervals.push_back(make_pair(3, 3));
	props->perfIntervals.push_back(make_pair(3 + 10 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 3 + 10 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2)));
	props->perfIntervals.push_back(make_pair(3 + 20 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 3 + 20 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2)));
	props->perfIntervals.push_back(make_pair(3 + 30 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 3 + 30 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2)));

	props->r_w = 0.05;
	props->r_e = 1000.0;

	gasOil_3d::Skeleton_Props tmp;
	tmp.cellsNum_z = 5;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = tmp.p_bub = 200.0 * 1.0e+5;
	tmp.s_init = 0.999;
	tmp.h1 = 0.0;
	tmp.h2 = 0.1;
	tmp.height = 0.1;
	tmp.perm_r = 100.0;
	tmp.perm_z = 5.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 4.35113e-10;

	tmp.skins.push_back(0.0);
	tmp.skins.push_back(0.0);
	//tmp.skins.push_back(0.0);
	//tmp.skins.push_back(0.0);

	tmp.radiuses_eff.push_back(props->r_w);
	tmp.radiuses_eff.push_back(props->r_w);
	//tmp.radiuses_eff.push_back(props->r_w);
	//tmp.radiuses_eff.push_back(props->r_w);

	props->props_sk.push_back(tmp);

	props->depth_point = 0.0;

	props->props_oil.visc = 1.64;
	props->props_oil.b_bore = 1.0;
	props->props_oil.dens_stc = 736.0;
	props->props_oil.beta = 1.0 * 1.e-9;

	props->props_gas.visc = 0.02833;
	props->props_gas.dens_stc = 0.8;

	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/koil.txt");
	setDataFromFile(props->kr_gas, "props/kgas.txt");

	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/Boil.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/Rs.txt");

	return props;
}*/

/*gasOil_perf_nit::Properties* getProps()
{
	gasOil_perf_nit::Properties* props = new gasOil_perf_nit::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_phi = 3;
	props->cellsNum_z = 1;

	props->timePeriods.push_back(20.0 * 86400.0);
	props->timePeriods.push_back(40.0 * 86400.0);
	//props->timePeriods.push_back(15.0 * 86400.0);
	//props->timePeriods.push_back(30.0 * 86400.0);

	props->leftBoundIsRate = true;
	props->rightBoundIsPres = true;
	props->rates.push_back(100.0);
	props->rates.push_back(0.0);
	//props->pwf.push_back(180.0 * 1.E+5);
	//props->pwf.push_back(140.0 * 1.E+5);
	//props->pwf.push_back(100.0 * 1.E+5);
	//props->pwf.push_back(200.0 * 1.E+5);

	props->ht = 1.0;
	props->ht_min = 1.0;
	props->ht_max = 500000.0;

	props->alpha = 7200.0;

	//props->perfTunnels.push_back(make_pair(3, 0));
	//props->perfTunnels.push_back(make_pair(3 + 10 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(3 + 20 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(3 + 30 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));

	props->perfTunnels.push_back(make_pair(1, 0));
	props->perfTunnels.push_back(make_pair(1 + 1 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(1 + 2 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	/*props->perfTunnels.push_back(make_pair(3 + 3 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(3 + 4 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(3 + 5 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(3 + 6 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(3 + 7 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(3 + 8 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(3 + 9 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(3 + 10 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(3 + 11 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	
	props->r_w = 0.05;
	props->r_e = 1000.0;

	gasOil_perf_nit::Skeleton_Props tmp;
	tmp.cellsNum_z = 1;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = tmp.p_bub = 200.0 * 100000.0;
	tmp.s_init = 0.999;
	tmp.t_init = -0.025;
	tmp.h1 = 0.0;
	tmp.h2 = 10.0;
	tmp.height = 10.0;
	tmp.perm_r = 100.0;
	tmp.perm_z = 100.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 6.0 * 1.0e-10;

	tmp.skins.push_back(0.0);
	tmp.skins.push_back(0.0);

	tmp.radiuses_eff.push_back(1.0 * props->r_w);
	tmp.radiuses_eff.push_back(1.0 * props->r_w);

	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda_r = tmp.lambda_z = 0.0;// 5.0;
	props->props_sk.push_back(tmp);

	props->depth_point = 0.0;

	// Thermal properties
	props->props_oil.visc = 1.0;
	props->props_oil.b_bore = 1.0;
	props->props_oil.dens_stc = 736.0;
	props->props_oil.beta = 1.0 * 1.e-9;
	props->props_oil.jt = 0.0;// 4.0 * 1.e-7;
	props->props_oil.ad = 0.0;// 2.1 * 1.e-7;
	props->props_oil.c = 1880.0;
	props->props_oil.lambda = 0.0;// 0.16;

	props->props_gas.visc = 0.03;
	props->props_gas.dens_stc = 0.8;
	props->props_gas.jt = 0.0;// -1.6 * 1.e-6;
	props->props_gas.ad = 0.0;// 3.6 * 1.e-6;
	props->props_gas.c = 3200.0;
	props->props_gas.lambda = 0.0;// 0.06;

	props->L = -50.0*1.e3;

	// Defining relative permeabilities
	setDataFromFile(props->kr_oil, "props/koil.txt");
	setDataFromFile(props->kr_gas, "props/kgas.txt");

	// Defining volume factors
	//props->byDefault.B_oil = true;
	setDataFromFile(props->B_oil, "props/new/Boil100.txt");
	//props->byDefault.B_gas = false;
	setDataFromFile(props->B_gas, "props/Bgas.txt");

	//props->byDefault.Rs = true;
	setDataFromFile(props->Rs, "props/new/Rs100.txt");

	return props;
}*/

/*oil_perf_nit::Properties* getProps()
{
	oil_perf_nit::Properties* props = new oil_perf_nit::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_phi = 3;
	props->cellsNum_z = 1;

	props->timePeriods.push_back(100.0 * 86400.0);
	//props->timePeriods.push_back(10.0 * 86400.0);
	//props->timePeriods.push_back(15.0 * 86400.0);
	//props->timePeriods.push_back(30.0 * 86400.0);

	props->leftBoundIsRate = false;
	props->rightBoundIsPres = true;
	//props->rates.push_back(20.0);
	props->pwf.push_back(180.0 * 1.E+5);
	//props->pwf.push_back(140.0 * 1.E+5);
	//props->pwf.push_back(100.0 * 1.E+5);
	//props->pwf.push_back(200.0 * 1.E+5);

	props->ht = 1.0;
	props->ht_min = 1.0;
	props->ht_max = 500000.0;

	props->alpha = 7200.0;
	
	//props->perfTunnels.push_back(make_pair(1, 0));
	//props->perfTunnels.push_back(make_pair(1 + 1 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(2 + 2 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(1 + 3 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(1 + 1 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(4 + 5 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(1 + 6 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(1 + 7 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(6 + 8 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(1 + 2 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(1 + 10 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(8 + 11 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(1 + 12 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	//props->perfTunnels.push_back(make_pair(1 + 13 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));
	props->perfTunnels.push_back(make_pair(1 + 0 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 0));

	//props->perfTunnels.push_back(make_pair(5 + 1 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 2));
	//props->perfTunnels.push_back(make_pair(6 + 11 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 2));
	//props->perfTunnels.push_back(make_pair(7 + 21 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 2));

	//props->perfTunnels.push_back(make_pair(8 + 1 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 2));
	//props->perfTunnels.push_back(make_pair(9 + 11 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 2));
	//props->perfTunnels.push_back(make_pair(10 + 21 * (props->cellsNum_r + 2)*(props->cellsNum_z + 2), 2));

	props->r_w = 0.05;
	props->r_e = 1000.0;

	oil_perf_nit::Skeleton_Props tmp;
	tmp.cellsNum_z = 1;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = 200.0 * 1.0e+5;
	tmp.t_init = 0.0;
	tmp.h1 = 0.0;
	tmp.h2 = 0.1;
	tmp.height = 0.1;
	tmp.perm_r = 500.0;
	tmp.perm_z = 500.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 0.0;// 4.35113e-10;

	tmp.skins.push_back(5.0);
	//tmp.skins.push_back(5.0);
	//tmp.skins.push_back(5.0);
	//tmp.skins.push_back(5.0);

	tmp.radiuses_eff.push_back( 10.0 * props->r_w );
	//tmp.radiuses_eff.push_back( 0.15 );
	//tmp.radiuses_eff.push_back( 0.15 );
	//tmp.radiuses_eff.push_back( 0.15 );

	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda_r = tmp.lambda_z = 0.0;// 5.0;

	props->props_sk.push_back(tmp);

	props->depth_point = 0.0;

	props->props_oil.visc = 1.0;
	props->props_oil.b_bore = 1.0;
	props->props_oil.dens_stc = 736.0;
	props->props_oil.beta = 0.0;// 1.0 * 1.e-9;
	props->props_oil.jt = 4.0 * 1.e-7;
	props->props_oil.ad = 0.0;// 2.0 * 1.e-7;
	props->props_oil.c = 1880.0;
	props->props_oil.lambda = 0.16;

	return props;
}*/

/*oil1D_NIT::Properties* getProps()
{
	oil1D_NIT::Properties* props = new oil1D_NIT::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_z = 1;

	props->timePeriods.push_back(86400.0 * 20.0);
	
	props->leftBoundIsRate = false;
	props->rightBoundIsPres = true;

	props->pwf.push_back(180.0 * 1.E+5);

	props->ht = 100.0;
	props->ht_min = 100.0;
	props->ht_max  = 100000.0;
	
	props->alpha = 7200.0;

	props->perfIntervals.push_back( make_pair(0, 0) );

	props->r_w = 0.05;
	props->r_e = 1000.0;

	oil1D_NIT::Skeleton_Props tmp;
	tmp.cellsNum_z = 1;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = 200.0 * 100000.0;
	tmp.t_init = 320.0;
	tmp.h1 = 0.0;
	tmp.h2 = 0.1;
	tmp.height = 0.1;
	tmp.perm_r = 500.0;
	tmp.perm_z = 500.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 0.0 * 1.0e-10;
	
	tmp.skins.push_back(0.0);
	tmp.radiuses_eff.push_back(props->r_w);

	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda = 0.0;
	props->props_sk.push_back( tmp );

	props->depth_point = 0.0;

	// Thermal properties
	props->props_oil.visc = 1.0;
	props->props_oil.b_bore = 1.0;
	props->props_oil.dens_stc = 736.0;
	props->props_oil.beta = 0.0;// 1.0 * 1.e-9;
	props->props_oil.jt = 4.0 * 1.e-7;
	props->props_oil.ad = 0.0;// 2.16 * 1.e-7;
	props->props_oil.c = 1880.0;
	props->props_oil.lambda = 0.0;

	return props;
}*/


/*oil_rz_nit::Properties* getProps()
{
	oil_rz_nit::Properties* props = new oil_rz_nit::Properties();

	props->cellsNum_r = 100;
	props->cellsNum_z = 50;

	props->timePeriods.push_back(20.0 * 86400.0);
	//props->timePeriods.push_back(10.0 * 86400.0);
	//props->timePeriods.push_back(15.0 * 86400.0);
	//props->timePeriods.push_back(30.0 * 86400.0);

	props->leftBoundIsRate = false;
	props->rightBoundIsPres = true;
	props->pwf.push_back(180.0 * 1.E+5);
	//props->pwf.push_back(140.0 * 1.E+5);
	//props->pwf.push_back(100.0 * 1.E+5);
	//props->pwf.push_back(200.0 * 1.E+5);

	props->ht = 100.0;
	props->ht_min = 10.0;
	props->ht_max = 500000.0;

	props->alpha = 7200.0;

	props->perfIntervals.push_back(make_pair(10, 40));

	props->r_w = 0.05;
	props->r_e = 1000.0;

	oil_rz_nit::Skeleton_Props tmp;
	tmp.cellsNum_z = 50;
	tmp.m = 0.1;
	tmp.p_init = tmp.p_out = 200.0 * 1.0e+5;
	tmp.t_init = 0.0;
	tmp.h1 = 0.0;
	tmp.h2 = 10.0;
	tmp.height = 0.1;
	tmp.perm_r = 500.0;
	tmp.perm_z = 500.0;
	tmp.dens_stc = 2000.0;
	tmp.beta = 0.0;// 4.35113e-10;

	tmp.skins.push_back(0.0);
	//tmp.skins.push_back(5.0);
	//tmp.skins.push_back(5.0);
	//tmp.skins.push_back(5.0);

	tmp.radiuses_eff.push_back( props->r_w );
	//tmp.radiuses_eff.push_back( 0.15 );
	//tmp.radiuses_eff.push_back( 0.15 );
	//tmp.radiuses_eff.push_back( 0.15 );

	tmp.c = 1800.0;
	tmp.kappa_eff = 0.0;
	tmp.lambda_r = tmp.lambda_z = 0.0;// 6.0;

	props->props_sk.push_back(tmp);

	props->depth_point = 0.0;

	props->props_oil.visc = 1.0;
	props->props_oil.b_bore = 1.0;
	props->props_oil.dens_stc = 736.0;
	props->props_oil.beta = 0.0;//1.0 * 1.e-9;
	props->props_oil.jt = 4.0 * 1.e-7;
	props->props_oil.ad = 0.0;// 2.0 * 1.e-7;
	props->props_oil.c = 1880.0;
	props->props_oil.lambda = 0.0;// 0.16;

	return props;
}*/

int main(int argc, char** argv)
{
	gasOil_rz_NIT::Properties* props = getProps();
	Scene<gasOil_rz_NIT::GasOil_RZ_NIT, gasOil_rz_NIT::GasOil2DNITSolver, gasOil_rz_NIT::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();
	
	/*gasOil_3d::Properties* props = getProps();
	Scene<gasOil_3d::GasOil_3D, gasOil_3d::Par3DSolver, gasOil_3d::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();*/

	/*gasOil_perf_nit::Properties* props = getProps();
	Scene<gasOil_perf_nit::GasOil_Perf_NIT, gasOil_perf_nit::ParPerfNITSolver, gasOil_perf_nit::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();*/

	/*gasOil_perf::Properties* props = getProps();
	Scene<gasOil_perf::GasOil_Perf, gasOil_perf::ParPerfSolver, gasOil_perf::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();*/

	/*oil_perf_nit::Properties* props = getProps();
	Scene<oil_perf_nit::Oil_Perf_NIT, oil_perf_nit::OilPerfNITSolver, oil_perf_nit::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();*/
	
	/*oil_rz_nit::Properties* props = getProps();
	Scene<oil_rz_nit::Oil_RZ_NIT, oil_rz_nit::OilRZNITSolver, oil_rz_nit::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();*/
	
	/*oil_rz::Properties* props = getProps();
	Scene<oil_rz::Oil_RZ, oil_rz::OilRZSolver, oil_rz::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();*/

	/*oil1D_NIT::Properties* props = getProps();
	Scene<oil1D_NIT::Oil1D_NIT, oil1D_NIT::Oil1DNITSolver, oil1D_NIT::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();*/

	/*gasOil_rz::Properties* props = getProps();
	Scene<gasOil_rz::GasOil_RZ, gasOil_rz::GasOil2DSolver, gasOil_rz::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();*/

	/*gas1D::Properties* props = getProps();
	Scene<gas1D::Gas1D, gas1D::Gas1DSol, gas1D::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();*/

	/*gas1D::Properties* props = getProps();
	double p_c = props->props_sk[0].p_out / 100000.0;
	double p_bhp [] = {80.0, 90.0, 100.0, 110.0, 120.0};
	double rate [] = {0.0, 0.0, 0.0, 0.0, 0.0}; 

	for(int i = 0; i < 5; i++)
	{
		props->pwf.clear();
		props->pwf.push_back( p_bhp[i] * 100000.0 );
		Scene<gas1D::Gas1D, gas1D::Gas1DSol, gas1D::Properties> scene;	
		scene.load(*props, i);

		gas1D::Gas1D* model = scene.getModel();

		scene.setSnapshotterType("VTK");
		scene.start();

		rate[i] = model->getRate() * model->Q_dim * 86400.0;
	}

	valarray<double> P_bhp (p_bhp, 5);
	valarray<double> Rate (rate, 5);

	double a = (Rate * Rate).sum();
	double b = (Rate * Rate * Rate).sum();
	double c = b;
	double d = (Rate * Rate * Rate * Rate).sum();

	double A = (d * (Rate * (p_c * p_c - P_bhp * P_bhp)).sum() - b * (Rate * Rate * (p_c * p_c - P_bhp * P_bhp)).sum() ) / (a * d - b * c);
	double B = (-c * (Rate * (p_c * p_c - P_bhp * P_bhp)).sum() + a * (Rate * Rate * (p_c * p_c - P_bhp * P_bhp)).sum() ) / (a * d - b * c);
*/

	/*oil_rz::Properties* props = getProps();
	Scene<oil_rz::Oil_RZ, oil_rz::OilRZSolver, oil_rz::Properties> scene;
	scene.load(*props);
	scene.setSnapshotterType("VTK");
	scene.start();*/

	/*testing::InitGoogleTest(&argc, argv);
	int res = RUN_ALL_TESTS();
	while (true) {};*/

	return 0;
}