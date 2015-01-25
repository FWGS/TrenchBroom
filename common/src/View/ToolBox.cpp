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

#include "ToolBox.h"
#include "SetBool.h"
#include "View/Tool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ToolBoxHelper::~ToolBoxHelper() {}
        
        Ray3 ToolBoxHelper::pickRay(const int x, const int y) const {
            return doGetPickRay(x, y);
        }
        
        Hits ToolBoxHelper::pick(const Ray3& pickRay) const {
            return doPick(pickRay);
        }
        
        void ToolBoxHelper::showPopupMenu() {
            doShowPopupMenu();
        }
        
        void ToolBoxHelper::doShowPopupMenu() {}
        
        ToolBox::ToolBox(wxWindow* window, ToolBoxHelper* helper) :
        m_window(window),
        m_helper(helper),
        m_toolChain(NULL),
        m_dragReceiver(NULL),
        m_modalReceiver(NULL),
        m_dropReceiver(NULL),
        m_savedDropReceiver(NULL),
        m_ignoreNextDrag(false),
        m_clickToActivate(true),
        m_ignoreNextClick(false),
        m_lastActivation(wxDateTime::Now()),
        m_enabled(true),
        m_ignoreMotionEvents(false) {
            assert(m_window != NULL);
            assert(m_helper != NULL);
            bindEvents();
        }
        
        ToolBox::~ToolBox() {
            unbindEvents();
        }
        
        void ToolBox::setClickToActivate(const bool clickToActivate) {
            m_clickToActivate = clickToActivate;
            if (!m_clickToActivate)
                m_ignoreNextClick = false;
        }
        
        const Ray3& ToolBox::pickRay() const {
            return m_inputState.pickRay();
        }
        
        const Hits& ToolBox::hits() const {
            return m_inputState.hits();
        }
        
        void ToolBox::updateHits() {
            const Ray3 pickRay = m_helper->pickRay(m_inputState.mouseX(),  m_inputState.mouseY());
            m_inputState.setPickRay(pickRay);
            
            Hits hits = m_helper->pick(m_inputState.pickRay());
            m_toolChain->pick(m_inputState, hits);
            m_inputState.setHits(hits);
        }
        
        void ToolBox::updateLastActivation() {
            m_lastActivation = wxDateTime::Now();
        }
        
        bool ToolBox::dragEnter(const wxCoord x, const wxCoord y, const String& text) {
            assert(m_dropReceiver == NULL);
            
            if (!m_enabled)
                return false;
            
            deactivateAllTools();
            mouseMoved(wxPoint(x, y));
            updateHits();
            m_dropReceiver = m_toolChain->dragEnter(m_inputState, text);
            m_window->Refresh();
            
            return m_dropReceiver != NULL;
        }
        
        bool ToolBox::dragMove(const wxCoord x, const wxCoord y, const String& text) {
            if (m_dropReceiver == NULL)
                return false;
            
            if (!m_enabled)
                return false;
            
            mouseMoved(wxPoint(x, y));
            updateHits();
            m_dropReceiver->dragMove(m_inputState);
            m_window->Refresh();
            
            return true;
        }
        
        void ToolBox::dragLeave() {
            if (m_dropReceiver == NULL)
                return;
            if (!m_enabled)
                return;
            
            // This is a workaround for a bug in wxWidgets 3.0.0 on GTK2, where a drag leave event
            // is sent right before the drop event. So we save the drag receiver in an instance variable
            // and if dragDrop() is called, it can use that variable to find out who the drop receiver is.
            m_savedDropReceiver = m_dropReceiver;
            
            m_dropReceiver->dragLeave(m_inputState);
            m_dropReceiver = NULL;
            m_window->Refresh();
        }
        
        bool ToolBox::dragDrop(const wxCoord x, const wxCoord y, const String& text) {
            if (m_dropReceiver == NULL && m_savedDropReceiver == NULL)
                return false;
            
            if (!m_enabled)
                return false;
            
            if (m_dropReceiver == NULL) {
                m_dropReceiver = m_savedDropReceiver;
                m_dropReceiver->activate(m_inputState); // GTK2 fix: has been deactivated by dragLeave()
                m_dropReceiver->dragEnter(m_inputState, text);
            }
            
            updateHits();
            const bool success = m_dropReceiver->dragDrop(m_inputState);
            m_dropReceiver = NULL;
            m_savedDropReceiver = NULL;
            m_window->Refresh();
            
            return success;
        }
        
        void ToolBox::OnKey(wxKeyEvent& event) {
            if (!m_enabled)
                return;
            
            if (updateModifierKeys()) {
                updateHits();
                m_toolChain->modifierKeyChange(m_inputState);
            }
            event.Skip();
            m_window->Refresh();
        }
        
        void ToolBox::OnMouseButton(wxMouseEvent& event) {
            if (!m_enabled)
                return;
            
            const MouseButtonState button = mouseButton(event);
            if (m_ignoreNextClick && button == MouseButtons::MBLeft) {
                if (event.ButtonUp())
                    m_ignoreNextClick = false;
                event.Skip();
                return;
            }
            
            m_window->SetFocus();
            if (event.ButtonUp())
                m_ignoreNextClick = false;

            updateModifierKeys();
            if (event.ButtonDown()) {
                captureMouse();
                m_clickPos = event.GetPosition();
                m_inputState.mouseDown(button);
                m_toolChain->mouseDown(m_inputState);
            } else {
                if (m_dragReceiver != NULL) {
                    m_dragReceiver->endMouseDrag(m_inputState);
                    m_dragReceiver = NULL;
                    
                    m_inputState.mouseUp(button);
                    releaseMouse();
                } else if (!m_ignoreNextDrag) {
                    const bool handled = m_toolChain->mouseUp(m_inputState);
                    
                    m_inputState.mouseUp(button);
                    releaseMouse();
                    
                    if (button == MouseButtons::MBRight && !handled) {
                        // We miss mouse events when a popup menu is already open, so we must make sure that the input
                        // state is up to date.
                        
                        mouseMoved(event.GetPosition());
                        updateHits();
                        
                        showPopupMenu();
                    }
                } else {
                    m_inputState.mouseUp(button);
                    releaseMouse();
                }
            }
            
            updateHits();
            m_ignoreNextDrag = false;
            
            m_window->Refresh();
        }
        
        void ToolBox::OnMouseDoubleClick(wxMouseEvent& event) {
            if (!m_enabled)
                return;
            
            const MouseButtonState button = mouseButton(event);
            updateModifierKeys();
            
            m_clickPos = event.GetPosition();
            m_inputState.mouseDown(button);
            m_toolChain->mouseDoubleClick(m_inputState);
            m_inputState.mouseUp(button);
            
            updateHits();
            
            m_window->Refresh();
        }
        
        void ToolBox::OnMouseMotion(wxMouseEvent& event) {
            if (!m_enabled || m_ignoreMotionEvents)
                return;
            
            updateModifierKeys();
            if (m_dragReceiver != NULL) {
                mouseMoved(event.GetPosition());
                updateHits();
                if (!m_dragReceiver->mouseDrag(m_inputState)) {
                    m_dragReceiver->endMouseDrag(m_inputState);
                    m_dragReceiver = NULL;
                    m_ignoreNextDrag = true;
                }
            } else if (!m_ignoreNextDrag) {
                if (m_inputState.mouseButtons() != MouseButtons::MBNone) {
                    if (std::abs(event.GetPosition().x - m_clickPos.x) > 1 ||
                        std::abs(event.GetPosition().y - m_clickPos.y) > 1) {
                        m_dragReceiver = m_toolChain->startMouseDrag(m_inputState);
                        if (m_dragReceiver == NULL)
                            m_ignoreNextDrag = true;
                        mouseMoved(event.GetPosition());
                        updateHits();
                        if (m_dragReceiver != NULL)
                            m_dragReceiver->mouseDrag(m_inputState);
                    }
                } else {
                    mouseMoved(event.GetPosition());
                    updateHits();
                    m_toolChain->mouseMove(m_inputState);
                }
            }
            
            m_window->Refresh();
        }
        
        void ToolBox::OnMouseWheel(wxMouseEvent& event) {
            if (!m_enabled)
                return;
            
            updateModifierKeys();
            const float delta = static_cast<float>(event.GetWheelRotation()) / event.GetWheelDelta() * event.GetLinesPerAction();
            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                m_inputState.scroll(delta, 0.0f);
            else if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                m_inputState.scroll(0.0f, delta);
            m_toolChain->scroll(m_inputState);
            
            updateHits();
            m_window->Refresh();
        }
        
        void ToolBox::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            if (!m_enabled)
                return;
            
            cancelDrag();
            
            m_window->Refresh();
            event.Skip();
        }
        
        void ToolBox::OnSetFocus(wxFocusEvent& event) {
            if (updateModifierKeys())
                m_toolChain->modifierKeyChange(m_inputState);
            m_window->Refresh();
            m_window->SetCursor(wxCursor(wxCURSOR_ARROW));
            
            // if this focus event happens as a result of a window activation, the don't ignore the next click
            if ((wxDateTime::Now() - m_lastActivation).IsShorterThan(wxTimeSpan(0, 0, 0, 100)))
                m_ignoreNextClick = false;
            
            event.Skip();
        }
        
        void ToolBox::OnKillFocus(wxFocusEvent& event) {
            cancelDrag();
            releaseMouse();
            if (clearModifierKeys())
                m_toolChain->modifierKeyChange(m_inputState);
            if (m_clickToActivate) {
                m_ignoreNextClick = true;
                m_window->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            m_window->Refresh();
            event.Skip();
        }
        
        void ToolBox::addTool(Tool* tool) {
            if (m_toolChain == NULL)
                m_toolChain = tool;
            else
                m_toolChain->appendTool(tool);
        }
        
        void ToolBox::deactivateWhen(Tool* master, Tool* slave) {
            assert(master != NULL);
            assert(slave != NULL);
            assert(master != slave);
            m_deactivateWhen[master].push_back(slave);
        }

        bool ToolBox::anyToolActive() const {
            return m_modalReceiver != NULL;
        }
        
        bool ToolBox::toolActive(const Tool* tool) const {
            return m_modalReceiver == tool;
        }
        
        void ToolBox::toggleTool(Tool* tool) {
            if (tool == NULL) {
                if (m_modalReceiver != NULL) {
                    deactivateTool(m_modalReceiver);
                    m_modalReceiver = NULL;
                }
            } else {
                if (m_modalReceiver == tool) {
                    deactivateTool(m_modalReceiver);
                    m_modalReceiver = NULL;
                } else {
                    if (m_modalReceiver != NULL) {
                        deactivateTool(m_modalReceiver);
                        m_modalReceiver = NULL;
                    }
                    if (activateTool(tool, m_inputState))
                        m_modalReceiver = tool;
                }
            }
            m_window->Refresh();
        }
        
        void ToolBox::deactivateAllTools() {
            toggleTool(NULL);
        }
        
        bool ToolBox::enabled() const {
            return m_enabled;
        }
        
        void ToolBox::enable() {
            m_enabled = true;
        }
        
        void ToolBox::disable() {
            cancelDrag();
            m_enabled = false;
        }
        
        bool ToolBox::cancel() {
            if (m_dragReceiver != NULL) {
                m_dragReceiver->cancelMouseDrag(m_inputState);
                m_dragReceiver = NULL;
                return true;
            }
            
            if (m_toolChain->cancel(m_inputState))
                return true;
            
            if (anyToolActive()) {
                deactivateAllTools();
                return true;
            }
            
            return false;
        }
        
        void ToolBox::setRenderOptions(Renderer::RenderContext& renderContext) {
            if (m_toolChain != NULL)
                m_toolChain->setRenderOptions(m_inputState, renderContext);
        }
        
        void ToolBox::renderTools(Renderer::RenderContext& renderContext) {
            /* if (m_modalReceiver != NULL)
                m_modalReceiver->renderOnly(m_inputState, renderContext);
            else */
            if (m_toolChain != NULL)
                m_toolChain->renderChain(m_inputState, renderContext);
        }
        
        bool ToolBox::activateTool(Tool* tool, const InputState& inputState) {
            if (!tool->activate(inputState))
                return false;
            
            ToolMap::iterator mapIt = m_deactivateWhen.find(tool);
            if (mapIt != m_deactivateWhen.end()) {
                const ToolList& slaves = mapIt->second;
                ToolList::const_iterator listIt, listEnd;
                for (listIt = slaves.begin(), listEnd = slaves.end(); listIt != listEnd; ++listIt) {
                    Tool* slave = *listIt;
                    
                    slave->deactivate(m_inputState);
                    toolDeactivatedNotifier(slave);
                }
            }
        
            toolActivatedNotifier(tool);
            return true;
        }
        
        void ToolBox::deactivateTool(Tool* tool) {
            tool->deactivate(m_inputState);
            toolDeactivatedNotifier(tool);

            ToolMap::iterator mapIt = m_deactivateWhen.find(tool);
            if (mapIt != m_deactivateWhen.end()) {
                const ToolList& slaves = mapIt->second;
                ToolList::const_iterator listIt, listEnd;
                for (listIt = slaves.begin(), listEnd = slaves.end(); listIt != listEnd; ++listIt) {
                    Tool* slave = *listIt;
                    
                    slave->activate(m_inputState);
                    toolActivatedNotifier(slave);
                }
            }
        }

        void ToolBox::captureMouse() {
            if (!m_window->HasCapture() && m_dragReceiver == NULL)
                m_window->CaptureMouse();
        }
        
        void ToolBox::releaseMouse() {
            if (m_window->HasCapture() && m_dragReceiver == NULL)
                m_window->ReleaseMouse();
        }

        
        void ToolBox::cancelDrag() {
            if (m_dragReceiver != NULL) {
                m_toolChain->cancelMouseDrag(m_inputState);
                m_inputState.clearMouseButtons();
                m_dragReceiver = NULL;
            }
        }
        
        ModifierKeyState ToolBox::modifierKeys() {
            const wxMouseState mouseState = wxGetMouseState();
            
            ModifierKeyState state = ModifierKeys::MKNone;
            if (mouseState.CmdDown())
                state |= ModifierKeys::MKCtrlCmd;
            if (mouseState.ShiftDown())
                state |= ModifierKeys::MKShift;
            if (mouseState.AltDown())
                state |= ModifierKeys::MKAlt;
            return state;
        }
        
        bool ToolBox::updateModifierKeys() {
            const ModifierKeyState keys = modifierKeys();
            if (keys != m_inputState.modifierKeys()) {
                m_inputState.setModifierKeys(keys);
                return true;
            }
            return false;
        }
        
        bool ToolBox::clearModifierKeys() {
            if (m_inputState.modifierKeys() != ModifierKeys::MKNone) {
                m_inputState.setModifierKeys(ModifierKeys::MKNone);
                return true;
            }
            return false;
        }
        
        MouseButtonState ToolBox::mouseButton(wxMouseEvent& event) {
            switch (event.GetButton()) {
                case wxMOUSE_BTN_LEFT:
                    return MouseButtons::MBLeft;
                case wxMOUSE_BTN_MIDDLE:
                    return MouseButtons::MBMiddle;
                case wxMOUSE_BTN_RIGHT:
                    return MouseButtons::MBRight;
                default:
                    return MouseButtons::MBNone;
            }
        }
        
        void ToolBox::mouseMoved(const wxPoint& position) {
            const wxPoint delta = position - m_lastMousePos;
            m_inputState.mouseMove(position.x, position.y, delta.x, delta.y);
            m_lastMousePos = position;
        }
        
        void ToolBox::showPopupMenu() {
            m_helper->showPopupMenu();
        }
        
        void ToolBox::bindEvents() {
            m_window->Bind(wxEVT_KEY_DOWN, &ToolBox::OnKey, this);
            m_window->Bind(wxEVT_KEY_UP, &ToolBox::OnKey, this);
            m_window->Bind(wxEVT_LEFT_DOWN, &ToolBox::OnMouseButton, this);
            m_window->Bind(wxEVT_LEFT_UP, &ToolBox::OnMouseButton, this);
            m_window->Bind(wxEVT_LEFT_DCLICK, &ToolBox::OnMouseDoubleClick, this);
            m_window->Bind(wxEVT_RIGHT_DOWN, &ToolBox::OnMouseButton, this);
            m_window->Bind(wxEVT_RIGHT_UP, &ToolBox::OnMouseButton, this);
            m_window->Bind(wxEVT_RIGHT_DCLICK, &ToolBox::OnMouseDoubleClick, this);
            m_window->Bind(wxEVT_MIDDLE_DOWN, &ToolBox::OnMouseButton, this);
            m_window->Bind(wxEVT_MIDDLE_UP, &ToolBox::OnMouseButton, this);
            m_window->Bind(wxEVT_MIDDLE_DCLICK, &ToolBox::OnMouseDoubleClick, this);
            m_window->Bind(wxEVT_AUX1_DOWN, &ToolBox::OnMouseButton, this);
            m_window->Bind(wxEVT_AUX1_UP, &ToolBox::OnMouseButton, this);
            m_window->Bind(wxEVT_AUX1_DCLICK, &ToolBox::OnMouseDoubleClick, this);
            m_window->Bind(wxEVT_AUX2_DOWN, &ToolBox::OnMouseButton, this);
            m_window->Bind(wxEVT_AUX2_UP, &ToolBox::OnMouseButton, this);
            m_window->Bind(wxEVT_AUX2_DCLICK, &ToolBox::OnMouseDoubleClick, this);
            m_window->Bind(wxEVT_MOTION, &ToolBox::OnMouseMotion, this);
            m_window->Bind(wxEVT_MOUSEWHEEL, &ToolBox::OnMouseWheel, this);
            m_window->Bind(wxEVT_MOUSE_CAPTURE_LOST, &ToolBox::OnMouseCaptureLost, this);
            m_window->Bind(wxEVT_SET_FOCUS, &ToolBox::OnSetFocus, this);
            m_window->Bind(wxEVT_KILL_FOCUS, &ToolBox::OnKillFocus, this);
        }
        
        void ToolBox::unbindEvents() {
            m_window->Unbind(wxEVT_KEY_DOWN, &ToolBox::OnKey, this);
            m_window->Unbind(wxEVT_KEY_UP, &ToolBox::OnKey, this);
            m_window->Unbind(wxEVT_LEFT_DOWN, &ToolBox::OnMouseButton, this);
            m_window->Unbind(wxEVT_LEFT_UP, &ToolBox::OnMouseButton, this);
            m_window->Unbind(wxEVT_LEFT_DCLICK, &ToolBox::OnMouseDoubleClick, this);
            m_window->Unbind(wxEVT_RIGHT_DOWN, &ToolBox::OnMouseButton, this);
            m_window->Unbind(wxEVT_RIGHT_UP, &ToolBox::OnMouseButton, this);
            m_window->Unbind(wxEVT_RIGHT_DCLICK, &ToolBox::OnMouseDoubleClick, this);
            m_window->Unbind(wxEVT_MIDDLE_DOWN, &ToolBox::OnMouseButton, this);
            m_window->Unbind(wxEVT_MIDDLE_UP, &ToolBox::OnMouseButton, this);
            m_window->Unbind(wxEVT_MIDDLE_DCLICK, &ToolBox::OnMouseDoubleClick, this);
            m_window->Unbind(wxEVT_AUX1_DOWN, &ToolBox::OnMouseButton, this);
            m_window->Unbind(wxEVT_AUX1_UP, &ToolBox::OnMouseButton, this);
            m_window->Unbind(wxEVT_AUX1_DCLICK, &ToolBox::OnMouseDoubleClick, this);
            m_window->Unbind(wxEVT_AUX2_DOWN, &ToolBox::OnMouseButton, this);
            m_window->Unbind(wxEVT_AUX2_UP, &ToolBox::OnMouseButton, this);
            m_window->Unbind(wxEVT_AUX2_DCLICK, &ToolBox::OnMouseDoubleClick, this);
            m_window->Unbind(wxEVT_MOTION, &ToolBox::OnMouseMotion, this);
            m_window->Unbind(wxEVT_MOUSEWHEEL, &ToolBox::OnMouseWheel, this);
            m_window->Unbind(wxEVT_MOUSE_CAPTURE_LOST, &ToolBox::OnMouseCaptureLost, this);
            m_window->Unbind(wxEVT_SET_FOCUS, &ToolBox::OnSetFocus, this);
            m_window->Unbind(wxEVT_KILL_FOCUS, &ToolBox::OnKillFocus, this);
        }
    }
}
