/*
 Copyright (C) 2010-2013 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MovementRestriction__
#define __TrenchBroom__MovementRestriction__

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class MovementRestriction {
        private:
            typedef enum {
                ARNone,
                ARLeftRight,
                ARForwardBack
            } Restriction;
            
            Restriction m_horizontalRestriction;
            Vec3 m_xAxis;
            Vec3 m_yAxis;
            bool m_verticalRestriction;
        public:
            MovementRestriction();
            
            void toggleHorizontalRestriction(const Renderer::Camera& camera);
            void setVerticalRestriction(const bool verticalRestriction);
            
            bool isRestricted(const Math::Axis::Type axis) const;
            Vec3 apply(const Vec3& v) const;
        };
    }
}

#endif /* defined(__TrenchBroom__MovementRestriction__) */