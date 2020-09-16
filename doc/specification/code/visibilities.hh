//forward delc. unit types
class CorrelatedFluxUnit;
class MegaHertzUnit;
class SecondsUnit;
class Unitless;

typedef VectorContainer<double, SecondsUnit> TimeAxis;
typedef VectorContainer<double, MegaHertzUnit> FrequencyAxis;
typedef VectorContainer< std::string, Unitless> PolarizationAxis;
typedef VectorContainer< std::string, Unitless> BaselineAxis;

class Visibilties: public TensorContainer< std::complex<double>, CorrelatedFluxUnit, 4, TimeAxis, FrequencyAxis, PolarizationAxis, BaselineAxis > {
//...impl..
};
