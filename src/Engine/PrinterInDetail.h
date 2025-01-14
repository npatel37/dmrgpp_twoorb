#ifndef PRINTERINDETAIL_H
#define PRINTERINDETAIL_H
#include <iostream>

namespace Dmrg {

template<typename LeftRightSuperType>
class PrinterInDetail {

	typedef typename LeftRightSuperType::BasisWithOperatorsType BasisWithOperatorsType;
	typedef typename BasisWithOperatorsType::SymmetryElectronsSzType SymmetryElectronsSzType;

public:

	PrinterInDetail(const LeftRightSuperType& lrs, SizeType mode, bool extended)
	    : lrs_(lrs), mode_(mode), extended_(extended)
	{}

	void print(std::ostream& os, PsimagLite::String msg) const
	{
		if (!extended_) return;
		printOneSide(os, "left", lrs_.left());
		printOneSide(os, "right", lrs_.right());
	}

private:

	void printOneSide(std::ostream& os,
	                  PsimagLite::String msg,
	                  const BasisWithOperatorsType& basis) const
	{
		SizeType sites = basis.block().size();
		os<<"#Side="<<msg<<"\n";
		os<<"#SitesOnThisSide ";
		for (SizeType i = 0; i < sites; ++i) {
			os<<basis.block()[i]<<" ";
		}

		os<<"\n";

		SizeType n = basis.partition();
		os<<"#Partitions "<<n<<"\n";
		for (SizeType i = 0; i < n - 1; ++i) {
			SizeType s = basis.partition(i + 1, BasisWithOperatorsType::AFTER_TRANSFORM) -
			        basis.partition(i, BasisWithOperatorsType::AFTER_TRANSFORM);
			SizeType j = basis.qn(basis.partition(i));
			PsimagLite::String q = SymmetryElectronsSzType::qnPrint(j,mode_ + 1);
			os<<q<<" "<<s<<"\n";
		}

		assert(sites > 0);
		SizeType site = basis.block()[sites - 1];
		SizeType end = basis.operatorsPerSite(0);
		SizeType siteC = site;
		if (msg == "right") {
			assert(site >= basis.block()[0]);
			siteC = site - basis.block()[0];
		}

		os<<"#Operators at site "<<site<<" ("<<siteC<<")\n";
		for (SizeType sigma = 0; sigma < end; ++sigma) {
			typename BasisWithOperatorsType::PairType p = basis.getOperatorIndices(siteC, sigma);
			os<<sigma<<" non-zeroes="<<basis.getOperatorByIndex(p.first).data.nonZero();
			os<<" rows="<<basis.getOperatorByIndex(p.first).data.rows()<<"\n";
		}
	}

	const LeftRightSuperType& lrs_;
	SizeType mode_;
	bool extended_;
}; // class PrinterInDetail
}
#endif // PRINTERINDETAIL_H
