#ifndef __AbstractTransformAxis_h__
#define __AbstractTransformAxis_h__

// AbstractTransformAxis.h
// Author: Peter Loan, Frank C. Anderson
/*
 * Copyright (c)  2006-2007, Stanford University. All rights reserved. 
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

// INCLUDE
#include <iostream>
#include <string>
#include <math.h>
#include <OpenSim/Simulation/osimSimulationDLL.h>
#include <OpenSim/Common/PropertyObjPtr.h>
#include <OpenSim/Common/PropertyStr.h>
#include <OpenSim/Common/Storage.h>
#include <OpenSim/Common/Function.h>
#include <OpenSim/Common/Constant.h>

namespace OpenSim {

class AbstractDynamicsEngine;
class AbstractJoint;
class AbstractCoordinate;

//=============================================================================
//=============================================================================
/**
 * A base class that specifies the interface for a SIMM dof (a "potential"
 * degree of freedom in a joint).
 *
 * @author Peter Loan
 * @version 1.0
 */
class OSIMSIMULATION_API AbstractTransformAxis : public Object  
{

//=============================================================================
// DATA
//=============================================================================
public:
	enum MotionType
	{
		Translational,
		Rotational
	};

protected:

	/** Transform function of the generalized coordinate used to represent
	the amount of transformation along a specified axis. */
	PropertyObjPtr<Function> _functionProp;
	Function *&_function;

	/** Name of the coordinate that serves as the indepenent variable of the
	tranform function. */
	PropertyStr _coordinateNameProp;
	std::string& _coordinateName;

	// Pointer to the coordinate used to compute the transformation.
	const AbstractCoordinate *_coordinate;

	// Pointer to the joint to which the coordinate belongs.
	AbstractJoint *_joint;

//=============================================================================
// METHODS
//=============================================================================
public:
	//--------------------------------------------------------------------------
	// CONSTRUCTION
	//--------------------------------------------------------------------------
	AbstractTransformAxis();
	AbstractTransformAxis(const AbstractTransformAxis &aDof);
	virtual ~AbstractTransformAxis();
	virtual Object* copy() const = 0;

#ifndef SWIG
	AbstractTransformAxis& operator=(const AbstractTransformAxis &aDof);
#endif
   void copyData(const AbstractTransformAxis &aDof);
	virtual void setup(AbstractDynamicsEngine* aEngine, AbstractJoint* aJoint);

	// GET & SET
	virtual void setCoordinateName(const std::string &aName);
	virtual std::string getCoordinateName() const { return _coordinateName; }
	virtual MotionType getMotionType() const = 0;
	virtual void setAxis(const SimTK::Vec3& axis) = 0;
	virtual void getAxis(SimTK::Vec3& axis) const = 0;
	virtual void getAxis(double rAxis[]) const = 0;
	virtual double getValue() = 0;
	virtual const AbstractCoordinate* getCoordinate() const { return _coordinate; }
	virtual Function* getFunction() const;
	virtual void setFunction(Function* aFunction);
	AbstractJoint* getJoint() { return _joint; }

	OPENSIM_DECLARE_DERIVED(AbstractTransformAxis, Object);

private:
	void setNull();
	void setupProperties();

//=============================================================================
};	// END of class AbstractTransformAxis
//=============================================================================
//=============================================================================

} // end of namespace OpenSim

#endif // __AbstractTransformAxis_h__

