#include "stdafx.h"
#include "Vehicle.hpp"


Vehicle::Vehicle(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics) : Entity(renderer, physics)
{

}

Vehicle::~Vehicle()
{
}
