////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     Example.tcc (data)
//  Authors:  Ofer Dekel
//
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace emll
{
namespace data
{
    template <typename DataVectorType, typename MetadataType>
    Example<DataVectorType, MetadataType>::Example(const std::shared_ptr<const DataVectorType>& dataVector, const MetadataType& metadata)
        : _dataVector(dataVector), _metadata(metadata)
    {
    }

    template<typename DataVectorType, typename MetadataType>
    template<typename TargetExampleType, utilities::IsSame<typename TargetExampleType::DataVectorType, DataVectorType> Concept>
    TargetExampleType Example<DataVectorType, MetadataType>::ToExample() const
    {
        // shallow copy of data vector
        return TargetExampleType(_dataVector, TargetExampleType::MetadataType(_metadata));
    }

    template<typename DataVectorType, typename MetadataType>
    template<typename TargetExampleType, utilities::IsDifferent<typename TargetExampleType::DataVectorType, DataVectorType> Concept>
    TargetExampleType Example<DataVectorType, MetadataType>::ToExample() const
    {
        // deep copy of data vector
        return TargetExampleType(std::make_shared<TargetExampleType::DataVectorType>(_dataVector->Duplicate<TargetExampleType::DataVectorType>()), TargetExampleType::MetadataType(_metadata));
    }

    template <typename DataVectorType, typename MetadataType>
    void Example<DataVectorType, MetadataType>::Print(std::ostream& os) const
    {
        _metadata.Print(os);
        os << "\t";
        _dataVector->Print(os);
    }

    template <typename DataVectorType, typename MetadataType>
    std::ostream& operator<<(std::ostream& ostream, const Example<DataVectorType, MetadataType>& example)
    {
        example.Print(ostream);
        return ostream;
    }
}
}
