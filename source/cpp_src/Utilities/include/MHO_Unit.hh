#include <array>
#include <string>

namespace hops
{

/*!
 *@file MHO_Unit
 *@class MHO_Unit.hh
 *@date Wed Feb 8 15:28:42 2023 -0500
 *@brief a unit class
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_Unit
{
    public:
        MHO_Unit(): fStringRep("");
        MHO_Unit(const std::string& unit);
        virtual ~MHO_Unit();

        //setter and getter for string representation
        void SetUnitString(const std::string unit) { Parse(unit); };

        std::string GetUnitString() const { return ConstructString(); }

        // operator overloads for multiplication and division
        MHO_Unit operator*(const MHO_Unit& other) const;
        MHO_Unit operator/(const MHO_Unit& other) const;

        // operator overloads for compound assignment
        MHO_Unit& operator*=(const MHO_Unit& other);
        MHO_Unit& operator/=(const MHO_Unit& other);

        //raise the unit to an integer power
        void RaiseToPower(int power);

        //invert the unit:
        void Invert();

        // equality operator
        bool operator==(const MHO_Unit& other) const;

        //assignment operator
        MHO_Unit& operator=(const MHO_Unit& other);

    private:
        std::string fStringRep;

        //the base SI units specified are:
        //[ length, time, mass, ampere, temperature, luminosity, quantity (mole) ]

        //but we don't deal with mole's very much so I think we would would probably rather use:
        //[ length, time, mass, ampere, temperature, luminosity, radians ]
        std::array< int, 7 > fExp;

        virtual void
        Parse(const std::string& repl); //takes a string and determines the appropriate unit exponents, and sets them in fExp
        //should be able to handle statements like "m/s" "s^{-1}" or "kg*m^2/s^2" as well as some compound SI units like joule "J" or tesla "T", etc.

        virutal std::string ConstructString() const; //constructs a human readable string from the base unit exponents

        //at some point we may want to add the ability to support common pre-factors (e.g. k for kilo, M for Mega, u for micro, etc.)
};

} // namespace hops
