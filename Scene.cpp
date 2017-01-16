#include "Scene.h"
#include "paralution.hpp"

#include "model/GasOil_RZ/GasOil_RZ.h"
#include "model/GasOil_RZ/GasOil2DSolver.h"

#include "model/VPP2d/VPP2d.hpp"
#include "model/VPP2d/VPPSolver.hpp"

#include "model/Bingham1d/Bingham1d.hpp"
#include "model/Bingham1d/BingSolver.hpp"

#include "model/GasOil_Elliptic/GasOil_Elliptic.hpp"
#include "model/GasOil_Elliptic/GasOilEllipticSolver.hpp"

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
Scene<gasOil_elliptic::GasOil_Elliptic, gasOil_elliptic::GasOilEllipticSolver, gasOil_elliptic::Properties>::~Scene()
{
	paralution::stop_paralution();

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
void Scene<gasOil_elliptic::GasOil_Elliptic, gasOil_elliptic::GasOilEllipticSolver, gasOil_elliptic::Properties>::load(gasOil_elliptic::Properties& props)
{
	model->load(props);
	paralution::init_paralution();
	method = new gasOil_elliptic::GasOilEllipticSolver(model);
}
template <class modelType, class methodType, typename propsType>
void Scene<modelType, methodType, propsType>::load(propsType& props, int i) {}
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

template class Scene<gasOil_rz::GasOil_RZ, gasOil_rz::GasOil2DSolver, gasOil_rz::Properties>;
template class Scene<vpp2d::VPP2d, vpp2d::VPPSolver, vpp2d::Properties>;
template class Scene<bing1d::Bingham1d, bing1d::Bing1dSolver, bing1d::Properties>;
template class Scene<gasOil_elliptic::GasOil_Elliptic, gasOil_elliptic::GasOilEllipticSolver, gasOil_elliptic::Properties>;