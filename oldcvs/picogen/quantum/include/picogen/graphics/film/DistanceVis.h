/***************************************************************************
 *            DistanceVis.h
 *
 *  2008-08-18 08:01:00 2008
 *  Copyright  2008  Sebastian Mach
 *  seb@greenhybrid.net
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DISTANCEVIS_H__INCLUDED
#define DISTANCEVIS_H__INCLUDED

namespace picogen {
    namespace graphics {
        namespace film {

            class DistanceVis : public abstract::IFilm {
                private:

                    unsigned int width, height;
                    ::picogen::misc::templates::surface<
                        ::picogen::graphics::color::AverageColor
                    > surface;

                public:
                    DistanceVis(unsigned int width, unsigned int height);
                    ~DistanceVis();
                    virtual void addSample (param_in (::picogen::graphics::structs::sample, sample));
                    virtual ::picogen::graphics::color::Color operator () (unsigned int x, unsigned int y) const;
                    virtual unsigned int getWidth() const;
                    virtual unsigned int getHeight() const;
            };
        } // namespace film {
    } // namespace graphics {
} // namespace picogen {
#endif // DISTANCEVIS_H__INCLUDED
