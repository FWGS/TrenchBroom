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

#include "WorldBoundsIssueGenerator.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class WorldBoundsIssueGenerator::WorldBoundsIssue : public Issue {
        public:
            friend class WorldBoundsIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            WorldBoundsIssue(Node* node) :
            Issue(node) {}
            
            IssueType doGetType() const {
                return Type;
            }
            
            const String doGetDescription() const {
                return "Object is out of world bounds";
            }
        };
        
        class WorldBoundsIssueGenerator::WorldBoundsIssueQuickFix : public IssueQuickFix {
        public:
            WorldBoundsIssueQuickFix() :
            IssueQuickFix("Delete objects") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const {
                facade->deleteObjects();
            }
        };

        const IssueType WorldBoundsIssueGenerator::WorldBoundsIssue::Type = Issue::freeType();
        
        WorldBoundsIssueGenerator::WorldBoundsIssueGenerator(const BBox3& bounds) :
        IssueGenerator(WorldBoundsIssue::Type, "Objects out of world bounds"),
        m_bounds(bounds) {
            addQuickFix(new WorldBoundsIssueQuickFix());
        }
        
        void WorldBoundsIssueGenerator::doGenerate(Entity* entity, IssueList& issues) const {
            if (!m_bounds.contains(entity->bounds()))
                issues.push_back(new WorldBoundsIssue(entity));
        }
        
        void WorldBoundsIssueGenerator::doGenerate(Brush* brush, IssueList& issues) const {
            if (!m_bounds.contains(brush->bounds()))
                issues.push_back(new WorldBoundsIssue(brush));
        }
    }
}
