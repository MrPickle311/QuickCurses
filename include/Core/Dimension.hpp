#pragma once
#include <exception>
#include <system_error>
#include <atomic>
#include <vector>

//TODO: make DimensionChecker a template class
template<int32_t Border> 
class DimensionChecker
{
public:
    void checkDimensions(int32_t dimension)
    {
        if(dimension < Border )
            throw std::logic_error(std::string{"Diemnsions cannot be < "} + std::to_string(Border));
    }
};

template<typename DimensionType,int32_t Border>
class Dimension
{
private:
    DimensionChecker<Border> checker_;
    std::atomic_int32_t value_;
public:
    explicit Dimension(int32_t value):
        value_{value}
    {
        checker_.checkDimensions(value_);
    }
    explicit Dimension(Dimension const& other):
        value_{(int32_t)other.value_}
    {}
    int32_t getValue() const
    {
        return value_;
    }
    void replace(int32_t new_value)
    {
        checker_.checkDimensions(new_value);
        value_ = new_value;
    }
    void operator+= (Dimension const& other)
    {
        value_ += other.value_;
    }
    void operator-= (Dimension const& other)
    {
        value_ -= other.value_;
    }
    friend Dimension operator+ (DimensionType const& left,DimensionType const& right)
    {
        return Dimension{ left.getValue() + right.getValue() };
    }
    friend Dimension operator- (DimensionType const& left,DimensionType const& right)
    {
        return Dimension{ left.getValue() - right.getValue() };
    }
    friend bool operator< (DimensionType const& left,DimensionType const& right)
    {
        return left.value_ < right.value_;
    }
        friend bool operator> (DimensionType const& left,DimensionType const& right)
    {
        return  ! ( left < right );
    }
    friend bool operator<= (DimensionType const& left,DimensionType const& right)
    {
        return left.value_ <= right.value_;
    }
    friend bool operator>= (DimensionType const& left,DimensionType const& right)
    {
        return ! ( left <= right );
    }
    friend bool operator== (DimensionType const& left,DimensionType const& right)
    {
        return left.value_ == right.value_;
    }
    friend bool operator!= (DimensionType const& left,DimensionType const& right)
    {
        return ! ( left == right );
    }
};

class Width:
    public Dimension<Width,1>
{
public:
    using Dimension<Width,1>::Dimension;
};

class Height:
    public Dimension<Height,1>
{
public:
    using Dimension<Height,1>::Dimension;
};

class XCoordinate:
    public Dimension<XCoordinate,0>
{
public:
    using Dimension<XCoordinate,0>::Dimension;
};

class YCoordinate:
    public Dimension<YCoordinate,0>
{
public:
    using Dimension<YCoordinate,0>::Dimension;
};

template<typename HorizontalDimension,typename VerctivalDimension>
class SurfaceDimension
{
private:
    HorizontalDimension horizontal_;
    VerctivalDimension  verctical_;
public:
    explicit SurfaceDimension(HorizontalDimension horizontal,
                              VerctivalDimension  verctical):
                        horizontal_{horizontal},
                        verctical_{verctical}
    {}
    explicit SurfaceDimension(SurfaceDimension<HorizontalDimension,VerctivalDimension> const& other):
                        horizontal_{other.getHorizontalDimension()},
                        verctical_{other.getVercticalDimension()}
    {}
    int32_t getHorizontalDimension() const
    {
        return horizontal_.getValue();
    }
    int32_t getVercticalDimension() const
    {
        return verctical_.getValue();
    }
    void replaceHorizontalDimension(int32_t new_value) 
    {
        horizontal_.replace(new_value);
    }
    void replaceVercticalDimension(int32_t new_value) 
    {
        verctical_.replace(new_value);
    }
};

class Point:
    public SurfaceDimension<XCoordinate,YCoordinate>
{
public:
    using SurfaceDimension<XCoordinate,YCoordinate>::SurfaceDimension;
};

class Size:
    public SurfaceDimension<Width,Height>
{
public:
    using SurfaceDimension<Width,Height>::SurfaceDimension;
};

template<typename DimensionType>
class DimensionGenerator
{
protected:
    DimensionType start_value_; 
    DimensionType end_value_; 
    DimensionType step_value_;
    std::vector<DimensionType> sequence_;
public:
//TODO: Would be better if constructor could take a YCoordinate,XCoordinate...
    DimensionGenerator(int32_t start_value,
                       int32_t end_value,
                       int32_t step_value):
                start_value_{start_value},
                end_value_{end_value},
                step_value_{step_value}
    {}
    DimensionGenerator(DimensionType start_value,
                       DimensionType end_value,
                       DimensionType step_value):
                start_value_{start_value.getValue()},
                end_value_{end_value.getValue()},
                step_value_{step_value.getValue()}
    {}
    void setStartValue(DimensionType new_start_value)
    {
        start_value_ = new_start_value;
    }
    void setEndValue(DimensionType new_end_value)
    {
        end_value_ = new_end_value;
    }
    void setStepValue(DimensionType new_step_value)
    {
        step_value_ = new_step_value;
    }
    DimensionType getStartValue() const
    {
        return start_value_;
    }
    DimensionType getEndValue() const
    {
        return end_value_;
    }
    DimensionType getStepValue() const
    {
        return step_value_;
    }
    void generateSequence()
    {
        sequence_.clear();
        for(DimensionType temp {start_value_}; temp <= end_value_ ; temp += step_value_)
            sequence_.emplace_back(temp);
    }
    std::vector<DimensionType> getSequence() const
    {
        return sequence_;
    }
};

class WidthGenerator:
    public DimensionGenerator<Width>
{
public:
    using DimensionGenerator<Width>::DimensionGenerator;
};

class HeightGenerator:
    public DimensionGenerator<Height>
{
public:
    using DimensionGenerator<Height>::DimensionGenerator;
};

class XCoordinateGenerator:
    public DimensionGenerator<XCoordinate>
{
public:
    using DimensionGenerator<XCoordinate>::DimensionGenerator;
};

class YCoordinateGenerator:
    public DimensionGenerator<YCoordinate>
{
public:
    using DimensionGenerator<YCoordinate>::DimensionGenerator;
};

//TODO: Make a following modes
//or maybe tempalte method ?
enum class SurfaceDimensionGeneratorMode{PointByPoint,Grid};

template<typename HorizontalDimension,typename VerctivalDimension>
class SurfaceDimensionGenerator:
    protected DimensionGenerator<HorizontalDimension>,
    protected DimensionGenerator<VerctivalDimension>
{
    using SfDim = SurfaceDimension<HorizontalDimension,VerctivalDimension>;
    using HGen = DimensionGenerator<HorizontalDimension>;
    using VGen = DimensionGenerator<VerctivalDimension>;
public:
//TODO: Would be better if constructor could take a Point,Size...
    SurfaceDimensionGenerator(int32_t horizontal_start_value,
                              int32_t horizontal_end_value,
                              int32_t horizontal_step_value,
                              int32_t verctical_start_value,
                              int32_t verctical_end_value,
                              int32_t verctical_step_value):
        DimensionGenerator<HorizontalDimension>{horizontal_start_value,
                                                horizontal_end_value,
                                                horizontal_step_value},
        DimensionGenerator<VerctivalDimension>{verctical_start_value,
                                                verctical_end_value,
                                                verctical_step_value}
    {}
    SurfaceDimensionGenerator(SurfaceDimension<HorizontalDimension,VerctivalDimension> start_value,
                              SurfaceDimension<HorizontalDimension,VerctivalDimension> end_value,
                              SurfaceDimension<HorizontalDimension,VerctivalDimension> step_value):
        DimensionGenerator<HorizontalDimension>{start_value.getHorizontalDimension(),
                                                end_value.getHorizontalDimension(),
                                                step_value.getHorizontalDimension()},
        DimensionGenerator<VerctivalDimension>{start_value.getVercticalDimension(),
                                                end_value.getVercticalDimension(),
                                                step_value.getVercticalDimension()}
    {}
    void  setStartValue(SurfaceDimension<HorizontalDimension,VerctivalDimension> start_value)
    {
        HGen::start_value = start_value.getHorizontalDimension();
        VGen::start_value = start_value.getVercticalDimension();
    } 
    void  setEndValue(SurfaceDimension<HorizontalDimension,VerctivalDimension> end_value)
    {
        HGen::end_value = end_value.getHorizontalDimension();
        VGen::end_value = end_value.getVercticalDimension();
    }
    void  setStepValue(SurfaceDimension<HorizontalDimension,VerctivalDimension> step_value) 
    {
        HGen::step_value = step_value.getHorizontalDimension();
        VGen::step_value = step_value.getVercticalDimension();
    }
    
    SurfaceDimension<HorizontalDimension,VerctivalDimension> getStartValue() const
    {
        return SfDim{HGen::getStartValue(),VGen::getStartValue()};
    }
    SurfaceDimension<HorizontalDimension,VerctivalDimension> getEndValue() const
    {
        return SfDim{HGen::getEndValue(),VGen::getEndValue()};
    }
    SurfaceDimension<HorizontalDimension,VerctivalDimension> getStepValue() const
    {
        return SfDim{HGen::getStepValue(),VGen::getStepValue()};
    }
    
    void generateSequence()
    {
        HGen::generateSequence();
        VGen::generateSequence();
    }
    std::vector<SurfaceDimension<HorizontalDimension,VerctivalDimension>> 
    getSequence() const
    {
        std::vector<SfDim> sequence_;
        sequence_.reserve(HGen::sequence_.size() * VGen::sequence_.size());
        
        for(auto&& h_dim : HGen::sequence_)
            for(auto&& v_dim : VGen::sequence_)
                sequence_.push_back(SfDim{h_dim,v_dim});
        return sequence_;
    } 
};

class PointGenerator:
    public SurfaceDimensionGenerator<XCoordinate,YCoordinate>
{
public:
    using SurfaceDimensionGenerator<XCoordinate,YCoordinate>::SurfaceDimensionGenerator;
};

class SizeGenerator:
    public SurfaceDimensionGenerator<Width,Height>
{
public:
    using SurfaceDimensionGenerator<Width,Height>::SurfaceDimensionGenerator;
};

//TODO: Make an optimization -> generators hold generated vectors 
//SurfaceDimensionGenerator has access to upper(horizontal and vertical) vectors
