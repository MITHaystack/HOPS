#ifndef MHO_ExtensibleElement_HH__
#define MHO_ExtensibleElement_HH__

#include <vector>

namespace hops
{

/*!
 *@file MHO_ExtensibleElement.hh
 *@class MHO_ExtensibleElement
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Sep 15 13:18:04 2021 -0400
 *@brief Implements an interface by which a class that inherits from MHO_ExtensibleElement
 * can have arbitrarily extended functionality added to it without modifying the original class
 * Pointers to the extensions are stored internally, and access proceeds through the visitor pattern.
 * This interface should be used VERY sparingly, as dynamic_casts are expensive and the number of dynamic_casts needed for a
 * single call to the 'Accept' method scales like N*M (where N is the number of visitors, and M the number of extensions).
 */

//forward declare these types
class MHO_Visitor;
class MHO_Element;
class MHO_ExtensibleElement;
template< class XExtensionType > class MHO_ExtendedElement;

//visitor interface
/**
 * @brief Class MHO_Visitor
 */
class MHO_Visitor
{
    public:
        MHO_Visitor(){};
        virtual ~MHO_Visitor(){};
};

/**
 * @brief Class MHO_Element
 */
class MHO_Element
{
    public:
        MHO_Element(){};
        virtual ~MHO_Element(){};

        /**
         * @brief Visits all extensions of this extensible element using the given visitor.
         *
         * @param aVisitor MHO_Visitor used to visit each extension.
         * @note This is a virtual function.
         */
        virtual void Accept(MHO_Visitor* aVisitor) = 0;
};

/**
 * @brief Class MHO_ExtensibleElement
 */
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

        /**
         * @brief Visits all extensions of MHO_ExtensibleElement using given visitor.
         *
         * @param aVisitor MHO_Visitor to traverse and operate on extensions
         */
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
        /**
         * @brief Checks if an extensible element has a specific extension type.
         *
         * @return True if the element has the specified extension type, false otherwise.
         */
        template< class XExtensionType > bool HasExtension() const;
        /**
         * @brief Creates and adds a new extension of type XExtensionType to the list of extensions.
         *
         * @tparam XExtensionType Template parameter XExtensionType
         * @return Pointer to the newly created MHO_ExtendedElement<XExtensionType
         */
        template< class XExtensionType > MHO_ExtendedElement< XExtensionType >* MakeExtension();
        /**
         * @brief Returns an extension of type XExtensionType if found in the list of extensions.
         *
         * @tparam XExtensionType Template parameter XExtensionType
         * @return Pointer to MHO_ExtendedElement<XExtensionType or nullptr if not found
         */
        template< class XExtensionType > MHO_ExtendedElement< XExtensionType >* AsExtension();

    protected:
        std::vector< MHO_Element* > fExtensions;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Checks if an extensible element has a specific extension type.
 *
 * @return True if extension exists, false otherwise.
 */
template< class XExtensionType > inline bool MHO_ExtensibleElement::HasExtension() const
{
    MHO_ExtendedElement< XExtensionType >* extention;
    for(auto it = fExtensions.begin(); it != fExtensions.end(); it++)
    {
        extention = dynamic_cast< MHO_ExtendedElement< XExtensionType >* >(*it);
        if(extention != nullptr)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Creates and adds a new extension of type XExtensionType to the extensible element.
 *
 * @return Pointer to the newly created MHO_ExtendedElement<XExtensionType
 */
template< class XExtensionType > inline MHO_ExtendedElement< XExtensionType >* MHO_ExtensibleElement::MakeExtension()
{
    MHO_ExtendedElement< XExtensionType >* extention;
    for(auto it = fExtensions.begin(); it != fExtensions.end(); it++)
    {
        extention = dynamic_cast< MHO_ExtendedElement< XExtensionType >* >(*it);
        if(extention != nullptr)
        {
            delete extention;
            fExtensions.erase(it);
            break;
        }
    }
    extention = new MHO_ExtendedElement< XExtensionType >(this);
    fExtensions.push_back(extention);
    return extention;
}

/**
 * @brief Returns a dynamically casted extension of type XExtensionType if found among stored extensions.
 *
 * @return Pointer to MHO_ExtendedElement<XExtensionType or nullptr if not found
 */
template< class XExtensionType > inline MHO_ExtendedElement< XExtensionType >* MHO_ExtensibleElement::AsExtension()
{
    MHO_ExtendedElement< XExtensionType >* extention;
    for(auto it = fExtensions.begin(); it != fExtensions.end(); it++)
    {
        extention = dynamic_cast< MHO_ExtendedElement< XExtensionType >* >(*it);
        if(extention != nullptr)
        {
            return extention;
        };
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Class MHO_ExtendedElement
 */
template< class XExtensionType > class MHO_ExtendedElement: public MHO_Element, public XExtensionType
{
    public:
        MHO_ExtendedElement(MHO_ExtensibleElement* parent): XExtensionType(parent), fParent(parent){};

        virtual ~MHO_ExtendedElement(){};

    public:
        /**
         * @brief Class ExtendedVisitor
         */
        class ExtendedVisitor: public MHO_Visitor
        {
            public:
                ExtendedVisitor(){};
                virtual ~ExtendedVisitor(){};

                /**
                 * @brief Function VisitExtendedElement
                 *
                 * @param anElement (MHO_ExtendedElement< XExtensionType >*)
                 * @note This is a virtual function.
                 */
                virtual void VisitExtendedElement(MHO_ExtendedElement< XExtensionType >* anElement) = 0;
        };

    public:
        /**
         * @brief Visits all extensions of this extensible element using the given visitor.
         *
         * @param aVisitor MHO_Visitor used to visit each extension.
         * @note This is a virtual function.
         */
        virtual void Accept(MHO_Visitor* aVisitor) override
        {
            //visit this extension
            auto* extVisitor = dynamic_cast< typename MHO_ExtendedElement< XExtensionType >::ExtendedVisitor* >(aVisitor);
            if(extVisitor != nullptr)
            {
                extVisitor->VisitExtendedElement(this);
            };
        }

    protected:
        MHO_ExtensibleElement* fParent;
};

} // namespace hops

#endif /*! end of include guard: MHO_ExtensibleElement */
