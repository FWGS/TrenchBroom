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

#ifndef __TrenchBroom__MoveObjectsTool__
#define __TrenchBroom__MoveObjectsTool__

#include "Controller/DragTool.h"
#include "Controller/MoveObjectsHandle.h"
#include "Renderer/MoveObjectsHandleFigure.h"
#include "Utility/Math.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace View {
        class DocumentViewHolder;
    }
    
    namespace Controller {
        class MoveObjectsTool : public DragTool {
        protected:
            typedef enum {
                RNone,
                RXAxis,
                RYAxis,
                RZAxis
            } RestrictToAxis;
            
            MoveObjectsHandle m_handle = MoveObjectsHandle(64.0f, 32.0f);
            Vec3f m_totalDelta;
            RestrictToAxis m_restrictToAxis;
            Renderer::MoveObjectsHandleFigure* m_handleFigure;
            
            void updateHits(InputEvent& event);
            bool handleMouseMoved(InputEvent& event);

            bool handleBeginPlaneDrag(InputEvent& event, Plane& dragPlane, Vec3f& initialDragPoint);
            bool handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint);
            void handleEndPlaneDrag(InputEvent& event);

            void handleChangeEditState(const Model::EditStateChangeSet& changeSet);
        public:
            MoveObjectsTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveObjectsTool__) */
