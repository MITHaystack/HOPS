#include <string>
#include <array>

namespace hops 
{

class MHO_Unit 
{
    public:
        MHO_Unit():fStringRep(""){};
        MHO_Unit(const std::string& unit);
        virtual ~MHO_Unit();
        
        //setter and getter for string representation
        void SetUnitString(const std::string unit);
        std::string GetUnitString()

        // operator overloads for multiplication and division
        MHO_Unit operator*(const MHO_Unit& other) const;
        MHO_Unit operator/(const MHO_Unit& other) const;
        
        // operator overloads for compound assignment
        SIUnit& operator*=(const SIUnit& other);
        SIUnit& operator/=(const SIUnit& other);
        
        //raise the unit to an integer power 
        void RaiseToPower(int power);
        
        //invert the unit:
        void Invert();

        // equality and assignment operations
        bool operator==(const MHO_Unit& other) const;
        MHO_Unit& operator=(const MHO_Unit& other);

    private:
        
        std::string fStringRep;
        
        virtual void Parse(const std::string& repl); //takes a string and determines the appropriate unit exponents, and sets them in fExp
        //should be able to handle statements like "m/s" "s^{-1}" or "kg*m^2/s^2" as well as some compound SI units like joule "J" or tesla "T", etc.

        //could be: 
        //[ length, time, mass, ampere, temperature, luminosity, quantity (mole) ]
        //or maybe we would rather have:
        //[ length, time, mass, ampere, temperature, luminosity, radians ]
        
        std::array<int, 7> fExp; 

};

}