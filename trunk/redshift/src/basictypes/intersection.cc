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

#include "../../include/basictypes/intersection.hh"

namespace redshift {



Intersection::Intersection ()
: primitive()
, differentialGeometry()
{
}



Intersection::Intersection (
        shared_ptr<redshift::primitive::Primitive const> prim,
        DifferentialGeometry const &geom
)
: primitive(prim)
, differentialGeometry(geom)
{
}



Intersection::Intersection (Intersection const &i)
: primitive(i.primitive)
, differentialGeometry(i.differentialGeometry) {
}



Intersection & Intersection::operator = (Intersection const &i) {
        primitive = i.primitive;
        differentialGeometry = i.differentialGeometry;
        return *this;
}



real_t Intersection::getDistance() const {
        return differentialGeometry.getDistance();
}



Normal Intersection::getGeometricNormal() const {
        return differentialGeometry.getGeometricNormal();
}



Normal Intersection::getShadingNormal() const {
        return differentialGeometry.getShadingNormal();
}



DifferentialGeometry Intersection::getDifferentialGeometry() const {
        return differentialGeometry;
}



shared_ptr<primitive::Primitive const> Intersection::getPrimitive () const {
        return primitive;
}



}
