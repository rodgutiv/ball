// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#ifndef CANONICALIZER_H
#define CANONICALIZER_H

#include "base.h"

#include <BALL/KERNEL/atomContainer.h>

using namespace BALL;
using namespace std;

/// C l a s s   C a n o n i c a l i z e r
/// ############################################################################
class Canonicalizer
{
public:
	Canonicalizer();
	~Canonicalizer();
	
	void canonicalize(AtomContainer &molecule);
	
private:
	// TODO: create OBGraphSym and OBmol as private member
};

/// C l a s s   M a t c h e r
/// ############################################################################
class Matcher
{
public:
	Matcher( CoordinateMap& coord_map );
	~Matcher();
	
	void matchFragment(AtomContainer &fragment);
	
	static const String getUCK(AtomContainer & mol);
	
private:
	CoordinateMap& _coord_lib;
};

#endif // CANONICALIZER_H