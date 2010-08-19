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

#ifndef DIFFERENTIALGEOMETRY_H_INCLUDED_20090301
#define DIFFERENTIALGEOMETRY_H_INCLUDED_20090301

#include "../geometry.hh"
#include "../sealed.hh"

namespace redshift {

        SEALED(DifferentialGeometry);

        class DifferentialGeometry : MAKE_SEALED(DifferentialGeometry) {
        public:
                DifferentialGeometry();

                DifferentialGeometry (
                        Point  const &center,
                        Normal const &geometricNormal,
                        Vector const &dpdu, Vector const &dpdv,
                        Vector const &dndu, Vector const &dndv
                );
                DifferentialGeometry (
                        Point  const &center,
                        Normal const &geometricNormal,
                        Vector const &dpdu, Vector const &dpdv,
                        Vector const &dndu, Vector const &dndv,
                        real_t u, real_t v, unsigned char texId
                );

                Point  getCenter() const ;

                Normal getShadingNormal() const ;
                Normal getGeometricNormal() const ;

                real_t u() const { return u_; }
                real_t v() const { return v_; }
                unsigned char textureId() const { return textureId_; }

                // D'oh, those names are braindead.
                Vector get_dpdu() const { return dpdu; }
                Vector get_dpdv() const { return dpdv; }
                Vector get_dndu() const { return dpdu; }
                Vector get_dndv() const { return dpdv; }

                void applyTransform (Transform const &t);
        private:

                Point  center;
                Normal geometricNormal;
                Normal shadingNormal;

                Vector dpdu, dpdv; // partial derivative of position, PBRT: p.83
                Vector dndu, dndv; // partial derivative of normal, PBRT: p.83

                real_t u_, v_;
                unsigned char textureId_;
        };
}

#endif // DIFFERENTIALGEOMETRY_H_INCLUDED_20090301
