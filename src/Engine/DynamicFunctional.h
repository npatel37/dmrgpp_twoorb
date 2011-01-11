
/*
Copyright (c) 2009, UT-Battelle, LLC
All rights reserved

[DMRG++, Version 2.0.0]
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

#ifndef DYN_FUNCTIONAL_H
#define DYN_FUNCTIONAL_H

#include "ProgressIndicator.h"

namespace Dmrg {
	
	template<
		typename RealType,typename SparseMatrixType,typename VectorWithOffsetType>
	class DynamicFunctional  {
	public:
		
		typedef RealType FieldType; // see documentation
		
		
		DynamicFunctional(
				const SparseMatrixType& H,
				const VectorWithOffsetType& aVector,
				RealType omega,
				RealType E0,
				RealType eta)
		:	
		 	H_(H),
		 	aVector_(aVector),
		 	omega_(omega),
		 	E0_(E0),
		 	eta_(eta),
		 	progress_("DynamicFunctional",0)
		 	
		{}
		
		

		void packComplexToReal(std::vector<RealType>& svReal,const std::vector<std::complex<RealType> >& sv)
		{
			svReal.resize(sv.size()*2);
			size_t j = 0;
			for (size_t i=0;i<sv.size();i++) {
				svReal[j++] = real(sv[i]);
				svReal[j++] = imag(sv[i]);
			}
		}
		

		void packRealToComplex(std::vector<std::complex<RealType> >& sv,const std::vector<RealType>& svReal)
		{
			sv.resize(svReal.size()/2);
			size_t j = 0;
			for (size_t i=0;i<sv.size();i++) {
				sv[i] = std::complex<RealType>(svReal[j],svReal[j+1]);
				j += 2;
			}
		}
		

		template<typename SomeVectorType>
		RealType operator()(const SomeVectorType &v) const
		{
			throw std::runtime_error("Neeeds implementation (sorry)\n");
		}
		

		size_t size() const {throw std::runtime_error("Neeeds implementation (sorry)\n");; }
		
		
	private:
		
		const SparseMatrixType& H_;
		const VectorWithOffsetType& aVector_;
		RealType omega_;
		RealType E0_;
		RealType eta_;
		ProgressIndicator progress_;
		
	}; // class DynamicFunctional
	
} // namespace
#endif // DYN_FUNCTIONAL_H
