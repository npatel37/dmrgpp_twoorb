/*
Copyright (c) 2009-2015, UT-Battelle, LLC
All rights reserved

[DMRG++, Version 3.0]
[by G.A., Oak Ridge National Laboratory]

UT Battelle Open Source Software License 11242008

OPEN SOURCE LICENSE

Subject to the conditions of this License, each
contributor to this software hereby grants, free of
charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), a
perpetual, worldwide, non-exclusive, no-charge,
royalty-free, irrevocable copyright license to use, copy,
modify, merge, publish, distribute, and/or sublicense
copies of the Software.

1. Redistributions of Software must retain the above
copyright and license notices, this list of conditions,
and the following disclaimer.  Changes or modifications
to, or derivative works of, the Software should be noted
with comments and the contributor and organization's
name.

2. Neither the names of UT-Battelle, LLC or the
Department of Energy nor the names of the Software
contributors may be used to endorse or promote products
derived from this software without specific prior written
permission of UT-Battelle.

3. The software and the end-user documentation included
with the redistribution, with or without modification,
must include the following acknowledgment:

"This product includes software produced by UT-Battelle,
LLC under Contract No. DE-AC05-00OR22725  with the
Department of Energy."

*********************************************************
DISCLAIMER

THE SOFTWARE IS SUPPLIED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER, CONTRIBUTORS, UNITED STATES GOVERNMENT,
OR THE UNITED STATES DEPARTMENT OF ENERGY BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

NEITHER THE UNITED STATES GOVERNMENT, NOR THE UNITED
STATES DEPARTMENT OF ENERGY, NOR THE COPYRIGHT OWNER, NOR
ANY OF THEIR EMPLOYEES, REPRESENTS THAT THE USE OF ANY
INFORMATION, DATA, APPARATUS, PRODUCT, OR PROCESS
DISCLOSED WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS.

*********************************************************

*/
/** \ingroup DMRG */
/*@{*/

/*! \file TargetParamsTimeStep.h
 *
 *  This is a structure to represent the parameters of the TimeStep Evolution
 *  algorithm. Don't add functions to this class because
 *  this class's data is all public
 */
#ifndef TARGET_PARAMS_TIMESTEP_H
#define TARGET_PARAMS_TIMESTEP_H

#include "TargetParamsTimeVectors.h"
#include "TargetParamsCommon.h"

namespace Dmrg {
// Coordinates reading of TargetSTructure from input file
template<typename ModelType>
class TargetParamsTimeStep : public TargetParamsTimeVectors<ModelType> {

public:

	typedef TargetParamsTimeVectors<ModelType> TimeVectorParamsType;
	typedef TargetParamsCommon<ModelType> TargetParamsCommonType;
	typedef typename ModelType::RealType RealType;

	template<typename IoInputter>
	TargetParamsTimeStep(IoInputter& io,const ModelType& model)
	    : TimeVectorParamsType(io,model),
	      maxTime_(0),useQns_(false)
	{
		try {
			io.readline(maxTime_,"TSPMaxTime=");
		} catch (std::exception&) {}

		try {
			int x = 0;
			io.readline(x,"TSPUseQns=");
			useQns_ = (x > 0);
		} catch (std::exception&) {}
	}

	virtual RealType maxTime() const
	{
		return maxTime_;
	}

	virtual bool useQns() const
	{
		return useQns_;
	}

private:

	RealType maxTime_;
	bool useQns_;

}; // class TargetParamsTimeStep

template<typename ModelType>
inline std::ostream&
operator<<(std::ostream& os,const TargetParamsTimeStep<ModelType>& t)
{
	os<<"#TargetParams.type=TimeStep\n";

	const typename TargetParamsTimeStep<ModelType>::TimeVectorParamsType& tp1 = t;
	os<<tp1;

	const typename TargetParamsTimeStep<ModelType>::TargetParamsCommonType& tp = t;
	os<<tp;

	if (t.maxTime() > 0)
		os<<"TSPMaxTime="<<t.maxTime()<<"\n";

	os<<"TSPUseQns= "<<t.useQns()<<"\n";

	return os;
}
} // namespace Dmrg

/*@}*/
#endif // TARGET_PARAMS_TIMESTEP_H

