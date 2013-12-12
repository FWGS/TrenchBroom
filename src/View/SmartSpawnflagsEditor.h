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

#ifndef __TrenchBroom__SmartSpawnflagsEditor__
#define __TrenchBroom__SmartSpawnflagsEditor__

#include "Model/ModelTypes.h"
#include "View/SmartPropertyEditor.h"
#include "View/ViewTypes.h"

#include <wx/gdicmn.h>

class wxCheckBox;
class wxCommandEvent;
class wxScrolledWindow;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class SmartSpawnflagsEditor : public SmartPropertyEditor {
        private:
            typedef enum {
                Unset,
                On,
                Off,
                Mixed
            } FlagValue;
            
            wxScrolledWindow* m_scrolledWindow;
            wxCheckBox* m_flags[24];
            wxPoint m_lastScrollPos;
        public:
            SmartSpawnflagsEditor(View::MapDocumentPtr document, View::ControllerPtr controller);
            
            void OnCheckBoxClicked(wxCommandEvent& event);
        private:
            wxWindow* doCreateVisual(wxWindow* parent);
            void doUpdateVisual(const Model::PropertyKey& key, const Model::EntityList& entities);
        };
    }
}

#endif /* defined(__TrenchBroom__SmartSpawnflagsEditor__) */