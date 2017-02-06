#include "stdafx.h"
#include "Vehicle.hpp"


Vehicle::Vehicle(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics) : Entity(renderer, physics)
{

}

Vehicle::~Vehicle()
{
}
