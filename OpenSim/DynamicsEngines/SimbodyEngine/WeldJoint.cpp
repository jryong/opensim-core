// WeldJoint.cpp
// Author: Ajay Seth
/*
 * Copyright (c)  2007, Stanford University. All rights reserved. 
* Use of the OpenSim software in source form is permitted provided that the following
* conditions are met:
* 	1. The software is used only for non-commercial research and education. It may not
*     be used in relation to any commercial activity.
* 	2. The software is not distributed or redistributed.  Software distribution is allowed 
*     only through https://simtk.org/home/opensim.
* 	3. Use of the OpenSim software or derivatives must be acknowledged in all publications,
*      presentations, or documents describing work in which OpenSim or derivatives are used.
* 	4. Credits to developers may not be removed from executables
*     created from modifications of the source.
* 	5. Modifications of source code must retain the above copyright notice, this list of
*     conditions and the following disclaimer. 
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
*  SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
*  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR BUSINESS INTERRUPTION) OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
*  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//=============================================================================
// INCLUDES
//=============================================================================
#include <iostream>
#include <math.h>
#include <OpenSim/Common/rdMath.h>
#include <OpenSim/Common/Mtx.h>
#include <OpenSim/Common/Function.h>
#include <OpenSim/Common/Constant.h>
#include "WeldJoint.h"
#include "TransformAxis.h"
#include "SimbodyEngine.h"
#include <OpenSim/Common/SimmMacros.h>
#include <OpenSim/Simulation/Model/BodySet.h>

//=============================================================================
// STATICS
//=============================================================================
using namespace std;
using namespace SimTK;
using namespace OpenSim;

//=============================================================================
// CONSTRUCTOR(S) AND DESTRUCTOR
//=============================================================================
//_____________________________________________________________________________
/**
 * Destructor.
 */
WeldJoint::~WeldJoint()
{
}
//_____________________________________________________________________________
/**
 * Default constructor.
 */
WeldJoint::WeldJoint() :
	Joint()
{
	setNull();
	setupProperties();
	updateSimbody();
}
//_____________________________________________________________________________
/**
 * Copy constructor.
 *
 * @param aJoint WeldJoint to be copied.
 */
WeldJoint::WeldJoint(const WeldJoint &aJoint) :
   Joint(aJoint)
{
	setNull();
	setupProperties();
	copyData(aJoint);
}

//=============================================================================
// CONSTRUCTION
//=============================================================================
//_____________________________________________________________________________
/**
 * Copy this body and return a pointer to the copy.
 * The copy constructor for this class is used.
 *
 * @return Pointer to a copy of this OpenSim::Body.
 */
Object* WeldJoint::copy() const
{
	WeldJoint *joint = new WeldJoint(*this);
	return(joint);
}
//_____________________________________________________________________________
/**
 * Copy data members from one WeldJoint to another.
 *
 * @param aJoint WeldJoint to be copied.
 */
void WeldJoint::copyData(const WeldJoint &aJoint)
{
	Joint::copyData(aJoint);
}

//_____________________________________________________________________________
/**
 * Set the data members of this WeldJoint to their null values.
 */
void WeldJoint::setNull()
{
	setType("WeldJoint");
	_parentBody = NULL;
	_body = NULL;
	_dynamicsEngine = NULL;
}

//_____________________________________________________________________________
/**
 * Connect properties to local pointers.
 */
void WeldJoint::setupProperties()
{
}

//_____________________________________________________________________________
/**
 * Update the underlying SDFast parameters, such as the inboard to joint and
 * body to joint vectors.
 *
 * @return True if the new inboard to joint was set; false otherwise.
 */
void WeldJoint::updateSimbody()
{
	setLocationInParent(_locationInParent);
	setLocation(_location);
}

//_____________________________________________________________________________
/**
 * Perform some set up functions that happen after the
 * object has been deserialized or copied.
 *
 * @param aEngine dynamics engine containing this WeldJoint.
 */
void WeldJoint::setup(AbstractDynamicsEngine* aEngine)
{
	string errorMessage;

	// Base class
	AbstractJoint::setup(aEngine);
	//_engine = dynamic_cast<SimbodyEngine*>(aEngine);

	// Look up the parent and child bodies by name in the
	// dynamics engine and store pointers to them.
	_parentBody = dynamic_cast<OpenSim::Body*>(aEngine->getBodySet()->get(_parentName));
	if (!_parentBody) {
		errorMessage += "Invalid parent body (" + _parentName + ") specified in joint " + getName();
		throw (Exception(errorMessage.c_str()));
	}
}

//=============================================================================
// OPERATORS
//=============================================================================
//_____________________________________________________________________________
/**
 * Assignment operator.
 *
 * @return Reference to this object.
 */
WeldJoint& WeldJoint::operator=(const WeldJoint &aJoint)
{
	AbstractJoint::operator=(aJoint);
	copyData(aJoint);
	return(*this);
}


//=============================================================================
// GET AND SET
//=============================================================================
//_____________________________________________________________________________
/**
 * Get the WeldJoint's forward transform.
 *
 * @return Reference to the forward transform.
 */
const OpenSim::Transform& WeldJoint::getForwardTransform()
{

	return _forwardTransform;
}

//_____________________________________________________________________________
/**
 * Get the WeldJoint's inverse transform.
 *
 * @return Reference to the inverse transform.
 */
const OpenSim::Transform& WeldJoint::getInverseTransform()
{

	return _inverseTransform;
}


//=============================================================================
// SCALING
//=============================================================================
//_____________________________________________________________________________
/**
 * Scale a joint based on XYZ scale factors for the bodies.
 *
 * @param aScaleSet Set of XYZ scale factors for the bodies.
 * @todo Need to scale transforms appropriately, given an arbitrary axis.
 */
void WeldJoint::scale(const ScaleSet& aScaleSet)
{
	// Joint knows how to scale locations of the joint in parent and on the body
	Joint::scale(aScaleSet);
}

//=============================================================================
// Simbody Model building.
//=============================================================================
//_____________________________________________________________________________
void WeldJoint::connectBody()
{
	// Joint checks that parent body being connected to is valid.
	Joint::connectBody();

	// CHILD TRANSFORM
	Rotation rotation(BodyRotationSequence, _orientation[0],XAxis, _orientation[1],YAxis, _orientation[2],ZAxis);
	SimTK::Transform childTransform(rotation,_location);

	// PARENT TRANSFORM
	Rotation parentRotation(BodyRotationSequence,_orientationInParent[0],XAxis,_orientationInParent[1],YAxis,_orientationInParent[2],ZAxis);
	SimTK::Transform parentTransform(parentRotation, _locationInParent);

	// COORDINATE SET
	// Should make sure it is empty regardless if coordinates were assigned.
	_coordinateSet.setSize(0); 

	// CREATE MOBILIZED BODY
	MobilizedBody::Weld
		simtkBody(getMultibodySystem(getEngine())->updMatterSubsystem().updMobilizedBody(getMobilizedBodyIndex(_parentBody)),
			parentTransform,SimTK::Body::Rigid(_body->getMassProperties()),
			childTransform);

	setMobilizedBodyIndex(_body, simtkBody.getMobilizedBodyIndex());

	associateCoordinatesAndSpeeds();
}