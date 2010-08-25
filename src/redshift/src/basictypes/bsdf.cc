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

#include "../../include/basictypes/bsdf.hh"

namespace redshift {


Bsdf::Bsdf (DifferentialGeometry const &shadingDG_
        // PBRT adds a refraction index parameter, see 10.1, p. 462
)
: shadingDG(shadingDG_)
, geometricNormal (shadingDG.getGeometricNormal())
, shadingNormal (shadingDG.getShadingNormal())
, s (normalize (shadingDG.get_dpdu()))
, t (cross (vector_cast<Vector>(shadingNormal), s))
//,s (normalize (shadingDG.get_dpdu()))
//,t (normalize (shadingDG.get_dpdv()))
{
}



Vector Bsdf::worldToLocal (Vector const &v) const {
        return Vector (
                dot (v, s),
                dot (v, vector_cast<Vector>(shadingNormal)),
                dot (v, t)
        );
}



Vector Bsdf::localToWorld (Vector const &v) const {
        return Vector (
                s.x*v.x + shadingNormal.x*v.y + t.x*v.z,
                s.y*v.x + shadingNormal.y*v.y + t.y*v.z,
                s.z*v.x + shadingNormal.z*v.y + t.z*v.z
        );
}



BsdfSample Bsdf::sample_f (
        const Vector &in_, BsdfType type, Random &rand
) const {
        const int nc = numComponents (type);
        if (nc == 0) {
                const char *msg = "bsdf.cc:Bsdf::sample_f() called for "
                                  "a combination of Reflection/Specular"
                                  " that is not present in this Bsdf.\n";
                std::cerr << msg;
                throw std::runtime_error(msg);
        }
        if (nc != 1) {
                throw std::runtime_error (
                        "BSDFs with mith multiple components "
                        "of same kind are currently not supported. "
                        "(" __FILE__ ")\n");
        }
        /*Color col;
        typedef std::vector<shared_ptr<Bxdf> >::iterator It;
        for (It it = bxdfs.begin(); it!=bxdfs.end(); ++it) {
                if ((**it).is (r,s))
                        col = col + (**it).f (out, in);
        }
        return col;*/

        const Vector in = worldToLocal (in_);

        typedef std::vector<shared_ptr<Bxdf> >::const_iterator It;
        for (It it = bxdfs.begin(); it!=bxdfs.end(); ++it) {
                if ((**it).is (type)) {
                        const BsdfSample ret = (**it).sample_f (in, rand);
                        return BsdfSample(
                                ret.color(),
                                localToWorld (ret.incident()),
                                ret.pdf()
                        );
                }
        }
        throw std::runtime_error("Impossible! See bsdf.cc:Bsdf::sample_f()");
}



Color Bsdf::f (
        const Vector &out_, const Vector &in_,
        BsdfType s,
        Random &rand
) const {
        const Vector out = worldToLocal (out_);
        const Vector in  = worldToLocal (in_);

        const real_t dotIn  = dot (vector_cast<Normal>(in_), geometricNormal);
        const real_t dotOut = dot (vector_cast<Normal>(out_), geometricNormal);
        if (dotIn * dotOut > 0) {
                // Both in same hemisphere -> BRDF only
                //r = Bsdf::Reflection (r & ~Bsdf::reflection);
                s.cancel (BsdfType::transmission);
        } else {
                // Different hemisphere -> BTDF only
                //r = Bsdf::Reflection (r & ~Bsdf::transmission);
                s.cancel (BsdfType::reflection);
        }

        Color col;
        typedef std::vector<shared_ptr<Bxdf> >::const_iterator It;
        for (It it = bxdfs.begin(); it!=bxdfs.end(); ++it) {
                if ((**it).is (s))
                        col = col + (**it).f (out, in, rand);
        }
        return col;
}



bool Bsdf::hasComponent (BsdfType s) const {
        typedef std::vector<shared_ptr<Bxdf> >::const_iterator It;
        for (It it = bxdfs.begin(); it!=bxdfs.end(); ++it) {
                if ((**it).is (s))
                        return true;
        }
        return false;
}



bool Bsdf::hasComponent (BsdfType::Specular s) const {
        typedef std::vector<shared_ptr<Bxdf> >::const_iterator It;
        for (It it = bxdfs.begin(); it!=bxdfs.end(); ++it)
                if ((**it).is (s))
                        return true;
        return false;
}



int Bsdf::numComponents (BsdfType s) const {
        typedef std::vector<shared_ptr<Bxdf> >::const_iterator It;
        int ret = 0;
        for (It it = bxdfs.begin(); it!=bxdfs.end(); ++it)
                if ((**it).is (s))
                        ++ret;
        return ret;
}



}
