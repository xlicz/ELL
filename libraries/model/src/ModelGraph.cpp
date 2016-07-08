////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     Model.cpp (model)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ModelGraph.h"
#include "Port.h"

// stl
#include <unordered_map>
#include <iostream>

/// <summary> model namespace </summary>
namespace model
{
    Node* Model::GetNode(Node::NodeId id)
    {
        auto it = _nodeMap.find(id);
        if (it == _nodeMap.end())
        {
            return nullptr; // weak_ptr equivalent of nullptr
        }
        else
        {
            return it->second.get();
        }
    }

    Model::NodeIterator Model::GetNodeIterator() const
    {
        return Model::NodeIterator(this, {});
    };

    Model::NodeIterator Model::GetNodeIterator(const Node* outputNode) const
    {
        return Model::NodeIterator(this, {outputNode});
    }

    Model::NodeIterator Model::GetNodeIterator(const std::vector<const Node*>& outputNodes) const
    {
        return Model::NodeIterator(this, outputNodes);
    }

    //
    // NodeIterator
    //

    // Real implementation function for `Visit()`
    Model::NodeIterator::NodeIterator(const Model* model, const std::vector<const Node*>& outputNodes) : _model(model)
    {
        if (_model->Size() == 0)
        {
            _currentNode = nullptr;
            return;
        }

        // start with output nodes in the stack
        _stack = outputNodes;
        _sentinelNode = nullptr;

        if (_stack.size() == 0) // Visit full graph
        {
            // helper function to find a terminal node
            auto IsLeaf = [](const Node* node) { return node->GetDependentNodes().size() == 0; };

            // start with some arbitrary node
            const Node* anOutputNode = _model->_nodeMap.begin()->second.get(); // !!! need private access

            // follow dependency chain until we get an output node
            while (!IsLeaf(anOutputNode))
            {
                anOutputNode = anOutputNode->GetDependentNodes()[0];
            }
            _stack.push_back(anOutputNode);
            _sentinelNode = anOutputNode;
        }
    }

    void Model::NodeIterator::Next()
    {
        while (_stack.size() > 0)
        {
            const Node* node = _stack.back();

            // check if we've already visited this node
            if (_visitedNodes.find(node) != _visitedNodes.end())
            {
                _stack.pop_back();
                continue;
            }

            // we can visit this node only if all its inputs have been visited already
            bool canVisit = true;
            for (auto input : node->_inputs)
            {
                for (const auto& inputNode : input->GetInputNodes())
                {
                    canVisit = canVisit && _visitedNodes.find(inputNode) != _visitedNodes.end();
                }
            }

            if (canVisit)
            {
                _stack.pop_back();
                _visitedNodes.insert(node);

                // In "visit whole graph" mode, we want to defer visiting the chosen output node until the end
                // In "visit active graph" mode, this test should never fail, and we'll always visit the node
                if (node != _sentinelNode)
                {
                    // TODO:
                    _currentNode = node;
                    //                    visitor(*node);
                }

                if (_sentinelNode != nullptr) // sentinelNode is non-null only if we're in visit-whole-graph mode
                {
                    // now add all our children (Note: this part is the only difference between visit-all and visit-active-graph
                    for (const auto& child : ModelImpl::Reverse(node->_dependentNodes)) // Visiting the children in reverse order more closely retains the order the features were originally created
                    {
                        // note: this is kind of inefficient --- we're going to push multiple copies of child on the stack. But we'll check if we've visited it already when we pop it off.
                        // TODO: optimize this if it's a problem
                        _stack.push_back(child);
                    }
                }
            }
            else // visit node's inputs
            {
                for (auto input : ModelImpl::Reverse(node->_inputs)) // Visiting the inputs in reverse order more closely retains the order the features were originally created
                {
                    for (const auto& inputNode : input->GetInputNodes())
                    {
                        _stack.push_back(inputNode);
                    }
                }
            }
        }

        if (_sentinelNode != nullptr)
        {
            //            visitor(*sentinelNode);
            _currentNode = _sentinelNode;
        }
    }
}
