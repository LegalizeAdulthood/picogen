//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2009  Sebastian Mach (*1983)
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

#include "../../include/primitives/closedsphere.hh"

namespace redshift { namespace primitive {



ClosedSphere::ClosedSphere (Point const & center, real_t radius)
: sphereData (center, radius)
{
}



ClosedSphere::~ClosedSphere () {
}



bool ClosedSphere::doesIntersect (RayDifferential const &ray) const {
        return sphereData.doesIntersect (ray);
}



bool ClosedSphere::doesIntersect (Ray const &ray) const {
        return sphereData.doesIntersect (ray);
}



optional<Intersection>
 ClosedSphere::intersect(RayDifferential const &ray) const {

        const optional<DifferentialGeometry> i(sphereData.intersect(ray));
        if (i) {
                return Intersection (shared_from_this(), *i);
        } else {
                return false;
        }
}



} }