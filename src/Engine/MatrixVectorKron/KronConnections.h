/*
Copyright (c) 2012, UT-Battelle, LLC
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
// END LICENSE BLOCK
/** \ingroup DMRG */
/*@{*/

/*! \file KronConnections.h
 *
 *
 */

#ifndef KRON_CONNECTIONS_H
#define KRON_CONNECTIONS_H

#include "Matrix.h"
#include "Concurrency.h"
#include "InitKron.h"

namespace Dmrg {

template<typename ModelType, typename ModelHelperType_>
class KronConnections {
	typedef InitKron<ModelType,ModelHelperType_> InitKronType;

	typedef typename InitKronType::SparseMatrixType SparseMatrixType;
	typedef typename SparseMatrixType::value_type ComplexOrRealType;

	typedef typename InitKronType::ArrayOfMatStructType ArrayOfMatStructType;
	typedef typename InitKronType::GenIjPatchType GenIjPatchType;
	typedef PsimagLite::Concurrency ConcurrencyType;
	typedef typename ArrayOfMatStructType::MatrixDenseOrSparseType MatrixDenseOrSparseType;
	typedef PsimagLite::Vector<SizeType>::Type VectorSizeType;

public:

	typedef PsimagLite::Matrix<ComplexOrRealType> MatrixType;
	typedef typename MatrixDenseOrSparseType::VectorType VectorType;
	typedef typename PsimagLite::Vector<VectorType>::Type VectorVectorType;
	typedef typename InitKronType::RealType RealType;
	typedef typename GenIjPatchType::BasisType BasisType;

	KronConnections(const InitKronType& initKron,
	                VectorType& x,
	                const VectorType& y,
	                SizeType)
	    : initKron_(initKron),
	      x_(x),
	      y_(y),
	      offsetForPatches_(initKron_.patch(GenIjPatchType::LEFT).size())
	{
		computeOffsets();
	}

	SizeType tasks() const
	{
		assert(initKron_.patch(GenIjPatchType::LEFT).size() ==
		       initKron_.patch(GenIjPatchType::RIGHT).size());
		return initKron_.patch(GenIjPatchType::LEFT).size();
	}

	void doTask(SizeType outPatch, SizeType)
	{
		SizeType nC = initKron_.connections();
		SizeType total = tasks();
		SizeType offsetX = offsetForPatches_[outPatch];
		assert(offsetX < x_.size());
		for (SizeType inPatch=0;inPatch<total;++inPatch) {
			SizeType offsetY = offsetForPatches_[inPatch];
			assert(offsetY < y_.size());
			for (SizeType ic=0;ic<nC;++ic) {
				const ArrayOfMatStructType& xiStruct = initKron_.xc(ic);
				const ArrayOfMatStructType& yiStruct = initKron_.yc(ic);

				const MatrixDenseOrSparseType& Amat =  xiStruct(outPatch,inPatch);
				const MatrixDenseOrSparseType& Bmat =  yiStruct(outPatch,inPatch);
				checks(Amat, Bmat, outPatch, inPatch);
				kronMult(x_, offsetX, y_, offsetY, 'n', 'n', Amat, Bmat);
			}
		}

	}

	void sync()
	{}

private:

	void computeOffsets()
	{
		assert(initKron_.patch(GenIjPatchType::LEFT).size() ==
		       initKron_.patch(GenIjPatchType::RIGHT).size());

		SizeType npatch = initKron_.patch(GenIjPatchType::LEFT).size();
		SizeType sum = 0;
		const BasisType& left = initKron_.lrs().left();
		const BasisType& right = initKron_.lrs().right();

		for( SizeType ipatch=0; ipatch < npatch; ipatch++) {
			offsetForPatches_[ipatch] = sum;

			SizeType igroup = initKron_.patch(GenIjPatchType::LEFT)[ipatch];
			SizeType jgroup = initKron_.patch(GenIjPatchType::RIGHT)[ipatch];

			assert(left.partition(igroup+1) >= left.partition(igroup));
			SizeType sizeLeft =  left.partition(igroup+1) - left.partition(igroup);

			assert(right.partition(jgroup+1) >= right.partition(jgroup));
			SizeType sizeRight = right.partition(jgroup+1) - right.partition(jgroup);

			sum +=  sizeLeft*sizeRight;
		}
	}

	// In production mode this function should be empty
	void checks(const MatrixDenseOrSparseType& Amat,
	            const MatrixDenseOrSparseType& Bmat,
	            SizeType ipatch,
	            SizeType jpatch) const
	{
#ifndef NDEBUG
		SizeType lSizeI = lSizeFunction(ipatch);
		SizeType lSizeJ = lSizeFunction(jpatch);
		SizeType rSizeI = rSizeFunction(ipatch);
		SizeType rSizeJ = rSizeFunction(jpatch);
		assert(Amat.rows() == lSizeI);
		assert(Amat.cols() == lSizeJ);
		assert(Bmat.rows() == rSizeI);
		assert(Bmat.cols() == rSizeJ);
#endif
	}

	SizeType lSizeFunction(SizeType ipatch) const
	{
		SizeType igroup = initKron_.patch(GenIjPatchType::LEFT)[ipatch];
		return initKron_.lrs().left().partition(igroup+1) -
		        initKron_.lrs().left().partition(igroup);
	}

	SizeType rSizeFunction(SizeType ipatch) const
	{
		SizeType jgroup = initKron_.patch(GenIjPatchType::RIGHT)[ipatch];
		return initKron_.lrs().right().partition(jgroup+1) -
		        initKron_.lrs().right().partition(jgroup);
	}

	const InitKronType& initKron_;
	VectorType& x_;
	const VectorType& y_;
	VectorSizeType offsetForPatches_;
}; //class KronConnections

} // namespace PsimagLite

/*@}*/

#endif // KRON_CONNECTIONS_H
