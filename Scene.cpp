#include "Scene.h"

#include "model/Oil1D/Oil1D.h"
#include "model/Oil1D/Oil1DSolver.h"

#include "model/Gas1D/Gas1D.h"
#include "model/Gas1D/Gas1D_simple.h"
#include "model/Gas1D/Gas1DSolver.h"

#include "model/Oil1D_NIT/Oil1D_NIT.h"
#include "model/Oil1D_NIT/Oil1DNITSolver.h"

#include "model/Oil_RZ/Oil_RZ.h"
#include "model/Oil_RZ/OilRZSolver.h"

#include "model/Oil_RZ_NIT/Oil_RZ_NIT.h"
#include "model/Oil_RZ_NIT/OilRZNITSolver.h"

#include "model/GasOil_RZ/GasOil_RZ.h"
#include "model/GasOil_RZ/GasOil2DSolver.h"

#include "model/GasOil_RZ_NIT/GasOil_RZ_NIT.h"
#include "model/GasOil_RZ_NIT/GasOil2DNITSolver.h"

#include "model/3D/GasOil_3D/GasOil_3D.h"
#include "model/3D/GasOil_3D/GasOil3DSolver.h"
#include "model/3D/GasOil_3D/Par3DSolver.h"

#include "model/3D/GasOil_3D_NIT/GasOil_3D_NIT.h"
#include "model/3D/GasOil_3D_NIT/GasOil3DNITSolver.h"

#include "model/3D/Perforation/GasOil_Perf.h"
#include "model/3D/Perforation/ParPerfSolver.h"

#include "model/3D/Perforation/Oil_Perf_NIT.h"
#include "model/3D/Perforation/OilPerfNITSolver.h"

#include "model/3D/Perforation/GasOil_Perf_NIT.h"
#include "model/3D/Perforation/ParPerfNITSolver.h"

#include "tests/gas1D-test.h"

#include "paralution.hpp"

using namespace std;
using namespace paralution;

template <class modelType, class methodType, typename propsType>
Scene<modelType, methodType, propsType>::Scene()
{
	model = new modelType();
}

template <class modelType, class methodType, typename propsType>
Scene<modelType, methodType, propsType>::~Scene()
{
	delete model;
	delete method;
}

template <>
Scene<gasOil_3d::GasOil_3D, gasOil_3d::Par3DSolver, gasOil_3d::Properties>::~Scene()
{
	stop_paralution();

	delete model;
	delete method;
}

template <class modelType, class methodType, typename propsType>
void Scene<modelType, methodType, propsType>::load(propsType& props)
{
	model->load(props);
	method = new methodType(model);
}

template <>
void Scene<gasOil_3d::GasOil_3D, gasOil_3d::Par3DSolver, gasOil_3d::Properties>::load(gasOil_3d::Properties& props)
{
	model->load(props);
	init_paralution();
	//set_omp_threads_paralution(1);
	//info_paralution();
	method = new gasOil_3d::Par3DSolver(model);
}

template <>
void Scene<gasOil_perf::GasOil_Perf, gasOil_perf::ParPerfSolver, gasOil_perf::Properties>::load(gasOil_perf::Properties& props)
{
	model->load(props);
	init_paralution();
	//set_omp_threads_paralution(1);
	//info_paralution();
	method = new gasOil_perf::ParPerfSolver(model);
}

template <>
void Scene<oil_perf_nit::Oil_Perf_NIT, oil_perf_nit::OilPerfNITSolver, oil_perf_nit::Properties>::load(oil_perf_nit::Properties& props)
{
	model->load(props);
	init_paralution();
	//set_omp_threads_paralution(1);
	//info_paralution();
	method = new oil_perf_nit::OilPerfNITSolver(model);
}

template <>
void Scene<gasOil_perf_nit::GasOil_Perf_NIT, gasOil_perf_nit::ParPerfNITSolver, gasOil_perf_nit::Properties>::load(gasOil_perf_nit::Properties& props)
{
	model->load(props);
	init_paralution();
	//set_omp_threads_paralution(1);
	//info_paralution();
	method = new gasOil_perf_nit::ParPerfNITSolver(model);
}

template <class modelType, class methodType, typename propsType>
void Scene<modelType, methodType, propsType>::load(propsType& props, int i)
{
}

template <>
void Scene<gas1D::Gas1D, gas1D::Gas1DSol, gas1D::Properties>::load(gas1D::Properties& props, int i)
{
	model->load(props);
	method = new gas1D::Gas1DSolver<gas1D::Gas1D>(model, i);
}

template <class modelType, class methodType, typename propsType>
void Scene<modelType, methodType, propsType>::setSnapshotterType(std::string type)
{
	model->setSnapshotter(type, model);
}

template <class modelType, class methodType, typename propsType>
void Scene<modelType, methodType, propsType>::start()
{
	method->start();
}

template <class modelType, class methodType, typename propsType>
modelType* Scene<modelType, methodType, propsType>::getModel() const
{
	return model;
}

template class Scene<oil1D::Oil1D, oil1D::Oil1DSolver, oil1D::Properties>;
template class Scene<gas1D::Gas1D, gas1D::Gas1DSol, gas1D::Properties>;
template class Scene<gas1D::Gas1D_simple, gas1D::Gas1DSolSimp, gas1D::Properties>;
template class Scene<oil1D_NIT::Oil1D_NIT, oil1D_NIT::Oil1DNITSolver, oil1D_NIT::Properties>;
template class Scene<oil_rz::Oil_RZ, oil_rz::OilRZSolver, oil_rz::Properties>;
template class Scene<oil_rz_nit::Oil_RZ_NIT, oil_rz_nit::OilRZNITSolver, oil_rz_nit::Properties>;
template class Scene<gasOil_rz::GasOil_RZ, gasOil_rz::GasOil2DSolver, gasOil_rz::Properties>;
template class Scene<gasOil_rz_NIT::GasOil_RZ_NIT, gasOil_rz_NIT::GasOil2DNITSolver, gasOil_rz_NIT::Properties>;
template class Scene<gasOil_3d::GasOil_3D, gasOil_3d::GasOil3DSolver, gasOil_3d::Properties>;
template class Scene<gasOil_3d::GasOil_3D, gasOil_3d::Par3DSolver, gasOil_3d::Properties>;
template class Scene<gasOil_3d_NIT::GasOil_3D_NIT, gasOil_3d_NIT::GasOil3DNITSolver, gasOil_3d_NIT::Properties>;
template class Scene<gasOil_perf::GasOil_Perf, gasOil_perf::ParPerfSolver, gasOil_perf::Properties>;
template class Scene<oil_perf_nit::Oil_Perf_NIT, oil_perf_nit::OilPerfNITSolver, oil_perf_nit::Properties>;
template class Scene<gasOil_perf_nit::GasOil_Perf_NIT, gasOil_perf_nit::ParPerfNITSolver, gasOil_perf_nit::Properties>;

template class Scene<Gas1D_Wrapped, gas1D::Gas1DSol, gas1D::Properties>;