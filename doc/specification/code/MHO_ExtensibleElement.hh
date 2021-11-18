#include <vector>

//forward declare these types
class MHO_Element;
class MHO_ExtensibleElement;
template<class XExtensionType> class MHO_ExtendedElement;

class MHO_Element{
    public:
        MHO_Element();
        virtual ~MHO_Element();
};

class MHO_ExtensibleElement{
    public:
        MHO_ExtensibleElement();
        virtual ~MHO_ExtensibleElement();
        template<class XExtensionType > MHO_ExtendedElement< XExtensionType >* MakeExtension();
        template<class XExtensionType > MHO_ExtendedElement< XExtensionType >* AsExtension();

    protected:
        std::vector< MHO_Element* > fExtensions;
};

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

    protected:

        MHO_ExtensibleElement* fParent;
};
