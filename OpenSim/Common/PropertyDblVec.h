#ifndef _PropertyDblVec_h_
#define _PropertyDblVec_h_
// PropertyDblVec.h
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
* Copyright (c) 2010, Stanford University. All rights reserved. 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions
* are met: 
*  - Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer. 
*  - Redistributions in binary form must reproduce the above copyright 
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the distribution. 
*  - Neither the name of the Stanford University nor the names of its 
*    contributors may be used to endorse or promote products derived 
*    from this software without specific prior written permission. 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
* POSSIBILITY OF SUCH DAMAGE. 
*/

#ifdef WIN32
#pragma warning( disable : 4251 )
#endif

#include "osimCommonDLL.h"
#include <string>
#include "Property.h"
#include "SimTKcommon.h"

//=============================================================================
//=============================================================================
/**
 * Class PropertyDblVec_ extends class Property.  It consists of a small
 * vector of doubles (i.e., SimTK::Vec<M>) and the methods for accessing
 * and modifying this Vec<M>.
 *
 * @version 1.0
 * @author Ayman Habib, Ajay Seth
 */
namespace OpenSim { 

template<int M> class PropertyDblVec_ : public Property
{

//=============================================================================
// DATA
//=============================================================================
private:
	/** Vector of doubles. */
	SimTK::Vec<M> _vec;

	char _typeAsString[7];

//=============================================================================
// METHODS
//=============================================================================
	//--------------------------------------------------------------------------
	// CONSTRUCTION
	//--------------------------------------------------------------------------
public:
	/** Default constructor */
	PropertyDblVec_():Property(Property::DblVec, "DblVec_PropertyName") 
		{ sprintf(_typeAsString, "DblVec%i", M); 
		  setAllowableArraySize(M);
		};
	/** Construct from name and value */
	PropertyDblVec_(const std::string &aName, const SimTK::Vec<M>& aVec):Property(Property::DblVec, aName), _vec(aVec) 
		{ sprintf(_typeAsString, "DblVec%i", M); 
		  setAllowableArraySize(M);
		};
	/** Construct from name and value as an Array<double> */
	PropertyDblVec_(const std::string &aName, const Array<double> &anArray):Property(Property::DblVec, aName)
		{ sprintf(_typeAsString, "DblVec%i", M);  setValue(anArray);
		  setAllowableArraySize(M);
		};
	/** Copy constructor */
	PropertyDblVec_(const PropertyDblVec_ &aProperty):Property(aProperty)
		{ _vec = aProperty._vec; };
	/* Return a copy of this property */
	virtual Property* copy() const {
		Property *property = new PropertyDblVec_<M>(*this);
		return(property);
	};
	virtual ~PropertyDblVec_() { };

	//--------------------------------------------------------------------------
	// OPERATORS
	//--------------------------------------------------------------------------
public:
	/** Assign this property to another */
	PropertyDblVec_& operator=(const PropertyDblVec_ &aProperty){
		Property::operator =(aProperty);
		_vec = aProperty._vec;
		return(*this);
	};

	//--------------------------------------------------------------------------
	// GET AND SET
	//--------------------------------------------------------------------------
public:
	/** Get the type of this property as a string. */
	virtual const char* getTypeAsString() const{
		return(_typeAsString);
	};

	// VALUE
	/** set value of property from an equivalently sized Vec */
	virtual void setValue(const SimTK::Vec<M> &aVec) { assert(aVec.size()==M); _vec=aVec; };
	/** set value of this property from an array of doubles of equal or greater length */
	virtual void setValue(const Array<double> &anArray){
		assert(anArray.getSize() >= M);
		for(int i=0;i<M; i++)
			_vec[i] = anArray[i];
	};
	/** get writable reference to the value */
	virtual SimTK::Vec<M>& getValueDblVec() {return _vec; };
	/** get const (read-only) reference to the value */
	virtual const SimTK::Vec<M>& getValueDblVec() const {return _vec; };
	/** set value from double array */
	virtual void setValue(int aSize, const double aArray[]){ // to be used by the serialization code
		assert(aSize == M);
		setValue(SimTK::Vec<M>::getAs(aArray));
	};	
	/** Get a constant String represeting the value of this property. */
	virtual const std::string &toString(){
		std::string str = "(";
		char dbl[256];
			for(int i=0; i < _vec.size(); i++){
				sprintf(dbl, "%g", _vec[i]);
				str += (i>0?" ":"") + std::string(dbl);
			}
		str += ")";
		_valueString = str;
		return (_valueString);
	};
	// SIZE
	virtual int getArraySize() const { return M; }
//=============================================================================
};	// END of class PropertyDblVec_

}; //OpenSim namespace
//=============================================================================
//=============================================================================

#endif //__PropertyDblVec__h__