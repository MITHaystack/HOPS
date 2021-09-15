#ifndef MHO_ExtensibleElement_HH__
#define MHO_ExtensibleElement_HH__

/*
*File: MHO_ExtensibleElement.hh
*Class: MHO_ExtensibleElement
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Implements an interface by which a class that inherits from MHO_ExtensibleElement
* can have arbitrarily extended functionality added to it without modifying the original class
* Pointers to the extensions are stored internally, and access proceeds through the visitor pattern.
* This interface should be used VERY sparingly, as dynamic_casts are expensive and the number of dynamic_casts needed for a
* single call to the 'Accept' method scales like N*M (where N is the number of visitors, and M the number of extensions).
*/

#include <vector>

namespace hops
{

//forward declare these types
class MHO_Visitor;
class MHO_Element;
class MHO_ExtensibleElement;
template<class XExtensionType> class MHO_ExtendedElement;

//visitor interface
class MHO_Visitor
{
    public:
        MHO_Visitor(){};
        virtual ~MHO_Visitor(){};
};

class MHO_Element
{
    public:

        MHO_Element(){};
        virtual ~MHO_Element(){};

        virtual void Accept(MHO_Visitor* aVisitor) = 0;
};

class MHO_ExtensibleElement: public MHO_Element
{
    public:

        MHO_ExtensibleElement(){};
        virtual ~MHO_ExtensibleElement()
        {
            for(auto it = fExtensions.begin(); it != fExtensions.end(); it++)
            {
                delete (*it);
                (*it) = nullptr;
            }
            fExtensions.clear();
        }

        void Accept(MHO_Visitor* aVisitor) override
        {
            //the MHO_ExtensibleElement class by-itself is just a container,
            //no need to visit it by itself, just visit all the extentions
            for(auto it = fExtensions.begin(); it != fExtensions.end(); it++)
            {
                (*it)->Accept(aVisitor);
            }
        }

    public:

        template<class XExtensionType > bool HasExtension() const;
        template<class XExtensionType > MHO_ExtendedElement< XExtensionType >* MakeExtension();
        template<class XExtensionType > MHO_ExtendedElement< XExtensionType >* AsExtension();

    protected:

        std::vector< MHO_Element* > fExtensions;

};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


template<class XExtensionType>
inline bool
MHO_ExtensibleElement::HasExtension() const
{
    MHO_ExtendedElement<XExtensionType>* extention;
    for(auto it = fExtensions.begin(); it != fExtensions.end(); it++)
    {
        extention = dynamic_cast< MHO_ExtendedElement<XExtensionType>* >( *it );
        if(extention != nullptr){return true;}
    }
    return false;
}

template<class XExtensionType>
inline MHO_ExtendedElement<XExtensionType>*
MHO_ExtensibleElement::MakeExtension()
{
    MHO_ExtendedElement<XExtensionType>* extention;
    for(auto it = fExtensions.begin(); it != fExtensions.end(); it++)
    {
        extention = dynamic_cast<MHO_ExtendedElement<XExtensionType>*>( *it );
        if(extention != nullptr){delete extention; fExtensions.erase(it); break; }
    }
    extention = new MHO_ExtendedElement<XExtensionType>(this);
    fExtensions.push_back(extention);
    return extention;
}

template<class XExtensionType>
inline MHO_ExtendedElement<XExtensionType>*
MHO_ExtensibleElement::AsExtension()
{
    MHO_ExtendedElement<XExtensionType>* extention;
    for(auto it = fExtensions.begin(); it != fExtensions.end(); it++)
    {
        extention = dynamic_cast<MHO_ExtendedElement<XExtensionType>*>( *it );
        if (extention != nullptr){return extention;};
    }
    return nullptr;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<class XExtensionType >
class MHO_ExtendedElement: public MHO_Element, public XExtensionType
{
    public:

        MHO_ExtendedElement(MHO_ExtensibleElement* parent):
            XExtensionType(parent),
            fParent(parent)
        {};

        virtual ~MHO_ExtendedElement(){};

    public:

        class ExtendedVisitor: public MHO_Visitor
        {
            public:
                ExtendedVisitor(){};
                virtual ~ExtendedVisitor(){};
                virtual void VisitExtendedElement( MHO_ExtendedElement< XExtensionType >* anElement) = 0;
        };

    public:

        virtual void Accept(MHO_Visitor* aVisitor) override
        {
            //visit this extension
            auto* extVisitor = dynamic_cast<typename MHO_ExtendedElement< XExtensionType >::ExtendedVisitor* >(aVisitor);
            if( extVisitor != nullptr)
            {
                extVisitor->VisitExtendedElement(this);
            };
        }

    protected:

        MHO_ExtensibleElement* fParent;
};


}

#endif /* end of include guard: MHO_ExtensibleElement */
