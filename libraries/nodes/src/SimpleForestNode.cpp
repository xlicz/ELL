////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     SimpleForestNode.cpp (nodes)
//  Authors:  Ofer Dekel
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SimpleForestNode.h"

// stl
#include <vector>
#include <memory>

namespace nodes
{
    SimpleForestNode::SimpleForestNode(const model::OutputPortElements<double>& input, const predictors::SimpleForestPredictor& forest) : Node({ &_input }, { &_prediction, &_treeOutputs, &_edgeIndicatorVector }), _input(this, input, inputPortName), _prediction(this, outputPortName, 1), _treeOutputs(this, treeOutputsPortName, forest.NumTrees()), _edgeIndicatorVector(this, edgeIndicatorVectorPortName, forest.NumEdges()), _forest(forest)
    {}

    std::string SimpleForestNode::GetRuntimeTypeName() const
    {
        return "SimpleForestNode";
    }

    void SimpleForestNode::Copy(model::ModelTransformer& transformer) const
    {
        auto newOutputPortElements = transformer.TransformOutputPortElements(_input.GetOutputPortElements());
        auto newNode = transformer.AddNode<SimpleForestNode>(newOutputPortElements, _forest);
        transformer.MapOutputPort(prediction, newNode->prediction);
        transformer.MapOutputPort(treeOutputs, newNode->treeOutputs);
        transformer.MapOutputPort(edgeIndicatorVector, newNode->edgeIndicatorVector);
    }

    void SimpleForestNode::Refine(model::ModelTransformer & transformer) const
    {
    }

    void SimpleForestNode::Compute() const
    {
        // forest output
        _prediction.SetOutput({ _forest.Predict(_input) });

        // individual tree outputs
        std::vector<double> treeOutputs(_forest.NumTrees());
        for(size_t i=0; i<_forest.NumTrees(); ++i)
        {
            treeOutputs[i] = _forest.Predict(_input, _forest.GetRootIndex(i));
        }
        _treeOutputs.SetOutput(std::move(treeOutputs));

        // path indicator
        auto edgeIndicator = _forest.GetEdgeIndicatorVector(_input);
        _edgeIndicatorVector.SetOutput(std::move(edgeIndicator));
    }

    SimpleForestPredictorOutputs BuildSubModel(const predictors::SimpleForestPredictor& predictor, model::Model& model, const model::OutputPortElements<double>& outputPortElements)
    {
        auto newNode = model.AddNode<SimpleForestNode>(outputPortElements, predictor);
        return { newNode->prediction };
    }
}