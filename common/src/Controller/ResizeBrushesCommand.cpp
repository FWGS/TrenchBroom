/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "ResizeBrushesCommand.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/ModelUtils.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType ResizeBrushesCommand::Type = Command::freeType();

        ResizeBrushesCommand::Ptr ResizeBrushesCommand::resizeBrushes(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec3& delta, const bool lockTextures) {
            return Ptr(new ResizeBrushesCommand(document, faces, collectBrushes(faces), delta, lockTextures));
        }
        
        const Model::BrushList& ResizeBrushesCommand::brushes() const {
            return m_brushes;
        }

        ResizeBrushesCommand::ResizeBrushesCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Model::BrushList& brushes, const Vec3& delta, const bool lockTextures) :
        DocumentCommand(Type, makeName(brushes), true, document),
        m_faces(faces),
        m_brushes(brushes),
        m_delta(delta),
        m_lockTextures(lockTextures) {}
        
        String ResizeBrushesCommand::makeName(const Model::BrushList& brushes) {
            return brushes.size() == 1 ? "Resize Brush" : "Resize Brushes";
        }
        
        Model::BrushList ResizeBrushesCommand::collectBrushes(const Model::BrushFaceList& faces) {
            Model::BrushSet brushSet;
            Model::BrushList brushList;
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                Model::Brush* brush = face->parent();
                assert(brush != NULL);
                
                if (brushSet.insert(brush).second)
                    brushList.push_back(brush);
            }
            
            return brushList;
        }

        bool ResizeBrushesCommand::doPerformDo() {
            return moveBoundary(m_delta);
        }
        
        bool ResizeBrushesCommand::doPerformUndo() {
            return moveBoundary(-m_delta);
        }

        bool ResizeBrushesCommand::doIsRepeatable(View::MapDocumentSPtr document) const {
            return false;
        }

        bool ResizeBrushesCommand::doCollateWith(Command::Ptr command) {
            Ptr other = Command::cast<ResizeBrushesCommand>(command);
            if (other->m_lockTextures != m_lockTextures)
                return false;
            
            m_delta += other->m_delta;
            return true;
        }

        bool ResizeBrushesCommand::moveBoundary(const Vec3& delta) {
            View::MapDocumentSPtr document = lockDocument();
            const BBox3& worldBounds = document->worldBounds();
            
            Model::ObjectSet entitySet;
            Model::ObjectList entities;
            Model::ObjectList brushes;
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                Model::Brush* brush = face->parent();
                if (!brush->canMoveBoundary(worldBounds, *face, delta))
                    return false;
                brushes.push_back(brush);
                
                Model::Entity* entity = brush->parent();
                if (entitySet.insert(entity).second)
                    entities.push_back(entity);
            }

            document->objectsWillChangeNotifier(entities);
            document->objectsWillChangeNotifier(brushes);
            
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                Model::Brush* brush = face->parent();
                brush->moveBoundary(worldBounds, *face, delta, m_lockTextures);
            }

            document->objectsDidChangeNotifier(brushes);
            document->objectsDidChangeNotifier(entities);
            return true;
        }
    }
}
