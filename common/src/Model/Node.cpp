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

#include "Node.h"

#include "CollectionUtils.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Model {
        Node::Node() :
        m_parent(NULL) {}
        
        Node::~Node() {
            VectorUtils::clearAndDelete(m_children);
        }
        
        Node* Node::parent() const {
            return m_parent;
        }
        
        bool Node::hasChildren() const {
            return !m_children.empty();
        }

        const NodeList& Node::children() const {
            return m_children;
        }

        void Node::addChild(Node* child) {
            assert(child != NULL);
            assert(VectorUtils::contains(m_children, child));
            assert(child->parent() == NULL);
            assert(canAddChild(child));
            attachChild(child);
            m_children.push_back(child);
        }
        
        void Node::removeChild(Node* child) {
            m_children.erase(doRemoveChild(child), m_children.end());
        }

        bool Node::canAddChild(Node* child) const {
            return doCanAddChild(child);
        }

        bool Node::canRemoveChild(Node* child) const {
            return doCanRemoveChild(child);
        }

        NodeList::iterator Node::doRemoveChild(Node* child) {
            assert(child != NULL);
            assert(child->parent() == this);

            NodeList::iterator it = std::remove(m_children.begin(), m_children.end(), child);
            assert(it != m_children.end());
            detachChild(*it);
            return it;
        }

        void Node::attachChild(Node* child) {
            child->setParent(this);
        }
        
        void Node::detachChild(Node* child) {
            child->setParent(NULL);
        }

        void Node::setParent(Node* parent) {
            assert(parent != NULL);
            assert(m_parent == NULL);
            
            if (parent == m_parent)
                return;
            m_parent = parent;
            ancestorDidChange();
        }
        
        void Node::ancestorDidChange() {
            doAncestorDidChange();
            NodeList::const_iterator it, end;
            for (it = m_children.begin(), end = m_children.end(); it != end; ++it) {
                Node* child = *it;
                child->ancestorDidChange();
            }
        }

        void Node::findAttributablesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableList& result) const {
            return doFindAttributablesWithAttribute(name, value, result);
        }
        
        void Node::findAttributablesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableList& result) const {
            return doFindAttributablesWithNumberedAttribute(prefix, value, result);
        }

        void Node::addToIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            doAddToIndex(attributable, name, value);
        }
        
        void Node::removeFromIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            doRemoveFromIndex(attributable, name, value);
        }

        void Node::doAddToIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            if (m_parent != NULL)
                m_parent->addToIndex(attributable, name, value);
        }
        
        void Node::doRemoveFromIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            if (m_parent != NULL)
                m_parent->removeFromIndex(attributable, name, value);
        }

        void Node::doFindAttributablesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableList& result) const {
            if (m_parent != NULL)
                m_parent->findAttributablesWithAttribute(name, value, result);
        }
        
        void Node::doFindAttributablesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableList& result) const {
            if (m_parent != NULL)
                m_parent->findAttributablesWithNumberedAttribute(prefix, value, result);
        }
 }
}
