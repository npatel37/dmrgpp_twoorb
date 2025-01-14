/*
Copyright (c) 2009-2012, UT-Battelle, LLC
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

/** \ingroup DMRG */
/*@{*/

/*! \file WaveFunctionTransfFactory.h
 *
 *  This class implements the wave function transformation factory,
 *  see PRL 77, 3633 (1996)
 *
 */

#ifndef WFT_FACTORY_H
#define WFT_FACTORY_H
#include "Utils.h"
#include "ProgressIndicator.h"
#include "WaveFunctionTransfLocal.h"
#include "WaveFunctionTransfPatched.h"
#include "WaveFunctionTransfSu2.h"
#include "DmrgWaveStruct.h"
#include "IoSimple.h"
#include "Random48.h"
#include "DiskStack.h"

namespace Dmrg {
template<typename LeftRightSuperType,typename VectorWithOffsetType_>
class WaveFunctionTransfFactory {

	typedef PsimagLite::IoSimple IoType;

public:

	enum {DO_NOT_RESET_COUNTER,RESET_COUNTER};

	typedef PsimagLite::Vector<SizeType>::Type VectorSizeType;
	typedef PsimagLite::Vector<PsimagLite::String>::Type VectorStringType;
	typedef typename LeftRightSuperType::BasisWithOperatorsType BasisWithOperatorsType;
	typedef typename BasisWithOperatorsType::BlockDiagonalMatrixType BlockDiagonalMatrixType;
	typedef typename BasisWithOperatorsType::SparseMatrixType SparseMatrixType;
	typedef typename BasisWithOperatorsType::BasisType BasisType;
	typedef typename SparseMatrixType::value_type SparseElementType;
	typedef typename PsimagLite::Vector<SparseElementType>::Type VectorType;
	typedef typename BasisWithOperatorsType::RealType RealType;
	typedef typename BasisType::FactorsType FactorsType;
	typedef DmrgWaveStruct<LeftRightSuperType> DmrgWaveStructType;
	typedef VectorWithOffsetType_ VectorWithOffsetType;
	typedef WaveFunctionTransfBase<DmrgWaveStructType,VectorWithOffsetType>
	WaveFunctionTransfBaseType;
	typedef WaveFunctionTransfLocal<DmrgWaveStructType,VectorWithOffsetType>
	WaveFunctionTransfLocalType;
	typedef WaveFunctionTransfPatched<DmrgWaveStructType,VectorWithOffsetType>
	WaveFunctionTransfPatchedType;
	typedef WaveFunctionTransfSu2<DmrgWaveStructType,VectorWithOffsetType>
	WaveFunctionTransfSu2Type;

	template<typename SomeParametersType>
	WaveFunctionTransfFactory(SomeParametersType& params)
	    : isEnabled_(!(params.options.find("nowft")!=PsimagLite::String::npos)),
	      stage_(ProgramGlobals::INFINITE),
	      counter_(0),
	      firstCall_(true),
	      progress_("WaveFunctionTransf"),
	      filenameIn_(params.checkpoint.filename),
	      filenameOut_(params.filename),
	      WFT_STRING(ProgramGlobals::WFT_STRING),
	      wftImpl_(0),
	      rng_(3433117),
	      twoSiteDmrg_(params.options.find("twositedmrg")!=PsimagLite::String::npos),
	      noLoad_(false),
	      save_(params.options.find("noSaveWft") == PsimagLite::String::npos)
	{
		if (!isEnabled_) return;

		bool b = (params.options.find("checkpoint")!=PsimagLite::String::npos ||
		        params.options.find("restart")!=PsimagLite::String::npos);

		if (b) {
			if (params.options.find("noloadwft")!=PsimagLite::String::npos)
				noLoad_=true;
			else
				load();
		} else {
			if (params.options.find("noloadwft")!=PsimagLite::String::npos) {
				PsimagLite::String str("Error: noloadwft needs restart or checkpoint\n");
				throw PsimagLite::RuntimeError(str.c_str());
			}
		}

		bool inBlocks = (params.options.find("wftInPatches")!=PsimagLite::String::npos);

		if (BasisType::useSu2Symmetry()) {
			if (inBlocks)
				err("wftInPatches not allowed when SU(2) is in use\n");

			wftImpl_=new WaveFunctionTransfSu2Type(stage_,
			                                       firstCall_,
			                                       counter_,
			                                       dmrgWaveStruct_,
			                                       twoSiteDmrg_);
		} else {
			if (inBlocks)
				wftImpl_ = new WaveFunctionTransfPatchedType(stage_,
				                                             firstCall_,
				                                             counter_,
				                                             dmrgWaveStruct_,
				                                             twoSiteDmrg_);
			else
				wftImpl_=new WaveFunctionTransfLocalType(stage_,
				                                         firstCall_,
				                                         counter_,
				                                         dmrgWaveStruct_,
				                                         twoSiteDmrg_);
		}
	}

	~WaveFunctionTransfFactory()
	{
		if (!isEnabled_) return;
		save(filenameOut_);
		delete wftImpl_;
	}

	void setStage(ProgramGlobals::DirectionEnum stage)
	{
		if (stage == stage_) return;
		stage_=stage;
		counter_=0;
	}

	void triggerOn(const LeftRightSuperType& lrs)
	{
		bool allow=false;
		switch (stage_) {
		case ProgramGlobals::INFINITE:
			allow=false;
			break;
		case ProgramGlobals::EXPAND_SYSTEM:
			allow=true;

		case ProgramGlobals::EXPAND_ENVIRON:
			allow=true;
		}
		// FIXME: Must check the below change when using SU(2)!!
		//if (m<0) allow = false; // isEnabled_=false;

		if (noLoad_) allow = false;

		if (!isEnabled_ || !allow) return;
		beforeWft(lrs);
		PsimagLite::OstringStream msg;
		msg<<"Window open, ready to transform vectors";
		progress_.printline(msg,std::cout);
	}

	// FIXME: change name to transformVector
	template<typename SomeVectorType,typename SomeVectorType2>
	void setInitialVector(SomeVectorType& dest,
	                      const SomeVectorType2& src,
	                      const LeftRightSuperType& lrs,
	                      const VectorSizeType& nk) const
	{
		bool allow=false;
		switch (stage_) {
		case ProgramGlobals::INFINITE:
			allow=false;
			break;
		case ProgramGlobals::EXPAND_SYSTEM:
			allow=true;

		case ProgramGlobals::EXPAND_ENVIRON:
			allow=true;
		}

		// FIXME: Must check the below change when using SU(2)!!
		//if (m<0) allow = false; // isEnabled_=false;

		if (noLoad_) allow = false;

		if (isEnabled_ && allow) {
#ifndef NDEBUG
			RealType eps = 1e-12;
			RealType x = norm(src);
			bool b = (x<eps);
			if (b) std::cerr<<"norm="<<x<<"\n";
			assert(!b);
#endif
			createVector(dest,src,lrs,nk);
		} else {
			createRandomVector(dest);
		}
	}

	void triggerOff(const LeftRightSuperType& lrs)
	{
		bool allow=false;
		switch (stage_) {
		case ProgramGlobals::INFINITE:
			allow=false;
			break;
		case ProgramGlobals::EXPAND_SYSTEM:
			allow=true;

		case ProgramGlobals::EXPAND_ENVIRON:
			allow=true;
		}

		// FIXME: Must check the below change when using SU(2)!!
		//if (m<0) allow = false; // isEnabled_=false;

		if (noLoad_) allow = false;

		if (!isEnabled_ || !allow) return;
		afterWft(lrs);
		PsimagLite::OstringStream msg;
		msg<<"Window closed, no more transformations, please";
		progress_.printline(msg,std::cout);
	}

	template<typename SomeVectorType>
	void createRandomVector(SomeVectorType& y) const
	{
		for (SizeType jj=0;jj<y.sectors();jj++) {
			SizeType j = y.sector(jj);
			createRandomVector(y,j);
		}

		if (!isEnabled_) return; // don't make noise unless enabled
		PsimagLite::OstringStream msg;
		msg<<"Yes, I'm awake, but there's nothing heavy to do now";
		progress_.printline(msg,std::cout);
	}

	template<typename SomeVectorType>
	void createRandomVector(SomeVectorType& y,SizeType i0) const
	{
		SizeType total = y.effectiveSize(i0);
		typename SomeVectorType::value_type tmp;
		RealType atmp=0;
		for (SizeType i=0;i<total;i++) {
			myRandomT(tmp);
			y.fastAccess(i0,i)=tmp;
			atmp += PsimagLite::real(tmp*PsimagLite::conj(tmp));
		}

		assert(fabs(atmp)>1e-10);
		atmp = 1.0 / sqrt (atmp);
		for (SizeType i=0;i<total;i++) y.fastAccess(i0,i) *= atmp;

	}

	void push(const BlockDiagonalMatrixType& transform,
	          ProgramGlobals::DirectionEnum direction,
	          const LeftRightSuperType& lrs)
	{
		if (!isEnabled_) return;

		switch (stage_) {
		case ProgramGlobals::INFINITE:
			if (direction == ProgramGlobals::EXPAND_SYSTEM) {
				wsStack_.push(transform);
				dmrgWaveStruct_.ws=transform;
			} else {
				weStack_.push(transform);
				dmrgWaveStruct_.we=transform;
			}
			break;
		case ProgramGlobals::EXPAND_ENVIRON:
			if (direction != ProgramGlobals::EXPAND_ENVIRON)
				throw std::logic_error("EXPAND_ENVIRON but option==0\n");
			dmrgWaveStruct_.we=transform;
			dmrgWaveStruct_.ws=transform;
			weStack_.push(transform);
			break;
		case ProgramGlobals::EXPAND_SYSTEM:
			if (direction != ProgramGlobals::EXPAND_SYSTEM)
				throw std::logic_error("EXPAND_SYSTEM but option==1\n");
			dmrgWaveStruct_.ws=transform;
			dmrgWaveStruct_.we=transform;
			wsStack_.push(transform);
			break;
		}

		dmrgWaveStruct_.lrs=lrs;
		PsimagLite::OstringStream msg;
		msg<<"OK, pushing option="<<direction<<" and stage="<<stage_;
		progress_.printline(msg,std::cout);

		if (noLoad_) {
			SizeType center = computeCenter(lrs,direction);
			updateNoLoad(lrs,center);
		}
	}

	const BlockDiagonalMatrixType& transform(SizeType what) const
	{
		return (what==ProgramGlobals::SYSTEM) ? dmrgWaveStruct_.ws : dmrgWaveStruct_.we;
	}

	const BlockDiagonalMatrixType& stackTransform(SizeType what) const
	{
		if (what==ProgramGlobals::SYSTEM) {
			if (wsStack_.size()==0) return dmrgWaveStruct_.ws;
			return wsStack_.top();
		} else {
			if (weStack_.size()==0) return dmrgWaveStruct_.we;
			return weStack_.top();
		}
	}

	const LeftRightSuperType& lrs() const { return dmrgWaveStruct_.lrs; }

	bool isEnabled() const { return isEnabled_; }

	bool twoSiteDmrg() const { return twoSiteDmrg_; }

	void save(PsimagLite::String fileOut) const
	{
		if (!isEnabled_) return;

		typename IoType::Out io(utils::pathPrepend(WFT_STRING, fileOut));
		if (!save_) return;
		PsimagLite::String s="isEnabled="+ttos(isEnabled_);
		io.printline(s);
		s="stage="+ttos(stage_);
		io.printline(s);
		s="counter="+ttos(counter_);
		io.printline(s);
		io.printline("dmrgWaveStruct");

		dmrgWaveStruct_.save(io);
		io.print("wsStack\n", wsStack_);
		io.print("weStack\n", weStack_);
	}

	void appendFileList(VectorStringType& files, PsimagLite::String rootName) const
	{
		files.push_back(utils::pathPrepend(WFT_STRING,rootName));
	}

private:

	void load()
	{
		if (!isEnabled_)
			throw PsimagLite::RuntimeError("WFT::load(...) called but wft is disabled\n");

		typename IoType::In io(utils::pathPrepend(WFT_STRING,filenameIn_));
		io.readline(isEnabled_,"isEnabled=");
		io.readline(stage_,"stage=");
		io.readline(counter_,"counter=");
		firstCall_=false;
		io.advance("dmrgWaveStruct");
		dmrgWaveStruct_.load(io);
		io.read(wsStack_,"wsStack");
		io.read(weStack_,"weStack");
	}

	void myRandomT(std::complex<RealType> &value) const
	{
		value = std::complex<RealType>(rng_() - 0.5, rng_() - 0.5);
	}

	void myRandomT(RealType &value) const
	{
		value = rng_() - 0.5;
	}

	void beforeWft(const LeftRightSuperType&)
	{
		if (stage_ == ProgramGlobals::EXPAND_ENVIRON) {
			if (wsStack_.size()>=1) {
				dmrgWaveStruct_.ws=wsStack_.top();
				wsStack_.pop();
				if (twoSiteDmrg_ && wsStack_.size()>0)
					dmrgWaveStruct_.ws=wsStack_.top();
			} else {
				throw PsimagLite::RuntimeError("System Stack is empty\n");
			}
		}

		if (stage_ == ProgramGlobals::EXPAND_SYSTEM) {
			if (weStack_.size()>=1) {
				dmrgWaveStruct_.we=weStack_.top();
				weStack_.pop();
				if (twoSiteDmrg_ && weStack_.size()>0)
					dmrgWaveStruct_.we=weStack_.top();
			} else {
				throw PsimagLite::RuntimeError("Environ Stack is empty\n");
			}
		}
		if (counter_ == 0 && stage_ == ProgramGlobals::EXPAND_SYSTEM) {
			if (weStack_.size()>=1) {
				dmrgWaveStruct_.we=weStack_.top();
			}
		}

		if (counter_ == 0 && stage_ == ProgramGlobals::EXPAND_ENVIRON) {
			if (wsStack_.size()>=1) {
				dmrgWaveStruct_.ws=wsStack_.top();
			}
		}
	}

	void createVector(VectorWithOffsetType& psiDest,
	                  const VectorWithOffsetType& psiSrc,
	                  const LeftRightSuperType& lrs,
	                  const VectorSizeType& nk) const
	{
		wftImpl_->transformVector(psiDest,psiSrc,lrs,nk);

		RealType norm1 = Dmrg::norm(psiSrc);
		RealType norm2 = Dmrg::norm(psiDest);
		PsimagLite::OstringStream msg;
		msg<<"Transformation completed ";
		if (fabs(norm1-norm2)>1e-5) {
			msg<<"WARNING: orig. norm= "<<norm1<<" resulting norm= "<<norm2;
		}

		if (norm2 < 1e-5)
			err("WFT Factory: norm2 = " + ttos(norm2) + " < 1e-5\n");

		progress_.printline(msg,std::cout);
	}

	void afterWft(const LeftRightSuperType& lrs)
	{
		dmrgWaveStruct_.lrs=lrs;
		firstCall_=false;
		counter_++;
	}

	void printDmrgWave() const
	{
		PsimagLite::IoSimple::Out io(std::cerr);
		dmrgWaveStruct_.save(io);
		std::cerr<<"wsStack="<<wsStack_.size()<<"\n";
		std::cerr<<"weStack="<<weStack_.size()<<"\n";
		std::cerr<<"counter="<<counter_<<"\n";
	}

	SizeType computeCenter(const LeftRightSuperType& lrs,
	                       ProgramGlobals::DirectionEnum direction) const
	{
		if (direction == ProgramGlobals::EXPAND_SYSTEM) {
			SizeType total = lrs.left().block().size();
			assert(total>0);
			total--;
			return lrs.left().block()[total];
		}

		return lrs.right().block()[0];
	}

	void updateNoLoad(const LeftRightSuperType& lrs,SizeType center)
	{
		sitesSeen_.push_back(center);
		SizeType numberOfSites = lrs.super().block().size();
		if (checkSites(numberOfSites)) {
			noLoad_=false;
			PsimagLite::OstringStream msg;
			msg<<" now available";
			progress_.printline(msg,std::cout);
		}
	}

	bool checkSites(SizeType numberOfSites) const
	{
		assert(numberOfSites>0);
		for (SizeType i=1;i<numberOfSites-1;i++) {
			bool seen = (std::find(sitesSeen_.begin(),
			                       sitesSeen_.end(),
			                       i) != sitesSeen_.end());
			if (!seen) return false;
		}

		return true;
	}

	bool isEnabled_;
	ProgramGlobals::DirectionEnum stage_;
	SizeType counter_;
	bool firstCall_;
	PsimagLite::ProgressIndicator progress_;
	PsimagLite::String filenameIn_;
	PsimagLite::String filenameOut_;
	const PsimagLite::String WFT_STRING;
	DmrgWaveStructType dmrgWaveStruct_;
	typename PsimagLite::Stack<BlockDiagonalMatrixType>::Type wsStack_,weStack_;
	WaveFunctionTransfBaseType* wftImpl_;
	PsimagLite::Random48<RealType> rng_;
	bool twoSiteDmrg_;
	bool noLoad_;
	bool save_;
	VectorSizeType sitesSeen_;
}; // class WaveFunctionTransformation
} // namespace Dmrg

/*@}*/
#endif

