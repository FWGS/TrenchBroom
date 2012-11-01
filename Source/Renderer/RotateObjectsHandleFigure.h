/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__RotateObjectsHandleFigure__
#define __TrenchBroom__RotateObjectsHandleFigure__

#include "Renderer/Figure.h"

namespace TrenchBroom {
    namespace Controller {
        class RotateObjectsHandle;
    }
    
    namespace Renderer {
        class RotateObjectsHandleFigure : public Figure {
        protected:
            Controller::RotateObjectsHandle& m_handle;
            float m_axisLength;
        public:
            RotateObjectsHandleFigure(Controller::RotateObjectsHandle& handle, float axisLength);

            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__RotateObjectsHandleFigure__) */
