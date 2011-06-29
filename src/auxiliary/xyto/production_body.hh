//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2010  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef PRODUCTION_BODY_HH_INCLUDED_20100726
#define PRODUCTION_BODY_HH_INCLUDED_20100726

#include "pattern.hh"

namespace xyto { 

class ProductionBody {
public:
        ProductionBody();
        ProductionBody(ProductionBody const &rhs);
        ProductionBody& operator= (ProductionBody const &rhs);

        Pattern pattern() const;
        double probability() const;
        void setPattern(Pattern const & p);
        void setProbability(double);
private:
        Pattern pattern_;
        double probability_;
};

}

#endif // PRODUCTION_BODY_HH_INCLUDED_20100726
