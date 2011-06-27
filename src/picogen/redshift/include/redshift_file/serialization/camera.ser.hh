//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~// Copyright (C) 2010  Sebastian Mach (*1983)// * mail: phresnel/at/gmail/dot/com// * http://phresnel.org// * http://picogen.org//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~// This program is free software: you can redistribute it and/or modify// it under the terms of the GNU General Public License as published by// the Free Software Foundation, either version 3 of the License, or// (at your option) any later version.//// This program is distributed in the hope that it will be useful,// but WITHOUT ANY WARRANTY; without even the implied warranty of// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the// GNU General Public License for more details.//// You should have received a copy of the GNU General Public License// along with this program.  If not, see <http://www.gnu.org/licenses/>.//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#ifndef CAMERA_SER_HH_20101013#define CAMERA_SER_HH_20101013#include "../camera.hh"#include "transform.ser.hh"namespace picogen { namespace redshift_file {        template<typename Arch>        void Camera::serialize (Arch &arch) {                using actuarius::pack;                arch & pack ("transform", transforms);                arch & pack ("type", Typenames, type);                switch (type) {                case pinhole:                        arch & pack ("front", pinholeParams.front);                        break;                case cylindrical:                        arch & pack ("front", cylindricalParams.front);                        break;                case cubemap_front:                case cubemap_back:                case cubemap_left:                case cubemap_right:                case cubemap_bottom:                case cubemap_top:                        break;                };        }} }#endif // CAMERA_SER_HH_20101013