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

#ifndef DISKSTACK_HEADER_H
#define DISKSTACK_HEADER_H

// All these includes are in PsimagLite
#include "Stack.h"
#include "IoSimple.h"
#include "ProgressIndicator.h"

// A disk stack, similar to std::stack but stores in disk not in memory
namespace Dmrg {
template<typename DataType>
class DiskStack {

	typedef typename PsimagLite::IoSimple::In IoInType;
	typedef typename PsimagLite::IoSimple::Out IoOutType;

public:

	DiskStack(const PsimagLite::String &file1,
	          const PsimagLite::String &file2,
	          bool hasLoad,
	          bool isObserveCode)
	    : fileIn_(file1),
	      fileOut_(file2),
	      isObserveCode_(isObserveCode),
	      total_(0),
	      progress_("DiskStack")
	{
		unlink(fileOut_.c_str());
		if (!hasLoad) return;

		try {
			ioIn_.open(fileIn_);
		} catch (std::exception& e) {
			std::cerr<<"Problem opening reading file "<<fileIn_<<"\n";
			throw PsimagLite::RuntimeError("DiskStack::load(...)\n");
		}

		int x = 0;
		ioIn_.readline(x,"#STACKMETARANK=",IoInType::LAST_INSTANCE);

		ioIn_.advance("#STACKMETASTACK");
		ioIn_>>stack_;
		ioIn_.close();
		PsimagLite::OstringStream msg;
		msg<<"Attempt to read from file " + fileIn_ + " succeeded";
		progress_.printline(msg,std::cout);

	}

	~DiskStack()
	{
		int x = 0;
		ioOut_.open(fileOut_,std::ios_base::app);
		ioOut_.printline("#STACKMETARANK="+ttos(x));
		ioOut_.printline("#STACKMETATOTAL="+ttos(total_));
		ioOut_<<"#STACKMETASTACK\n";
		ioOut_<<stack_;
		ioOut_.close();
	}

	static bool persistent() { return true; }

	bool inDisk() const { return true; }

	void push(DataType const &d)
	{
		ioOut_.open(fileOut_,std::ios_base::app);
		d.save(ioOut_,DataType::SAVE_ALL);
		ioOut_.close();

		stack_.push(total_);
		total_++;
	}

	void pop()
	{
		stack_.pop();
	}

	DataType top()
	{
		ioIn_.open(fileIn_);
		DataType dt(ioIn_,"",stack_.top(),isObserveCode_);
		ioIn_.close();
		return dt;
	}

	SizeType size() const { return stack_.size(); }

	friend void copyDiskToDisk(DiskStack& dest, const DiskStack& src)
	{
		dest.isObserveCode_ = src.isObserveCode_;
		dest.total_ = src.total_;
		dest.stack_ = src.stack_;
		// copy src.fileIn_ --> dest.fileIn_
		myCopy(src.fileIn_, dest.fileIn_);
		// copy src.fileOut_ --> dest.fileOut_
		myCopy(src.fileOut_, dest.fileOut_);
	}

	friend std::ostream& operator<<(std::ostream& os,
	                                const DiskStack& ds)
	{
		os<<"DISKSTACK: filein: "<<ds.fileIn_<<" fileout="<<ds.fileOut_<<"\n";
		os<<"total="<<ds.total_<<"\n";
		os<<ds.stack_;
		return os;
	}

private:

	static void myCopy(PsimagLite::String src, PsimagLite::String dest)
	{
		std::ifstream  src2(src.c_str(), std::ios::binary);
		std::ofstream  dst2(dest.c_str(), std::ios::binary);

		dst2 << src2.rdbuf();
	}

	PsimagLite::String fileIn_;
	PsimagLite::String fileOut_;
	bool isObserveCode_;
	int total_;
	PsimagLite::ProgressIndicator progress_;
	IoInType ioIn_;
	IoOutType ioOut_;
	PsimagLite::Stack<int>::Type stack_;
}; // class DiskStack

} // namespace DMrg

#endif

