
class helper_base
{
    public:
        helper_base(){};
        virtual ~helper_base(){};

        virtual bool init() = 0;
        virtual bool exe() = 0;

    protected:

};

template< typename XType >
class helper_specific: public helper_base
{
    public:

        helper_specific(XType* ptr):fPtr(ptr){};
        virtual ~helper_specific(){};

        virtual bool init() override {return true;};

        virtual bool exe() override
        {
            //do something with fPtr
            (void) fPtr1;
            return true;
        }

    private:

        XType* fPtr;
};


template< typename XType1, typename XType2 >
class helper_specific2: public helper_base
{
    public:

        helper_specific(XType1* ptr1, XType2* ptr2):
            fPtr1(ptr1),
            fPtr2(ptr2)
        {};
        virtual ~helper_specific(){};

        virtual init() override () {return true;}

        virtual bool exe() override
        {
            //do something with fPtr1 and fPtr2
            (void) fPtr1;
            (void) fPtr2;
            return true;
        }

    private:

        XType1* fPtr1;
        XType2* fPtr2
};

class impl_base
{
    public:
        impl_base(){}
        virtual ~impl_base(){};


        virtual void Initialize() = 0;


        //delayed dispatch to type erased helper
        virtual void Execute() = 0;



}


class impl_1: public impl_base
{
    public impl_1():fHelper(nullptr){};
    virtual ~impl_1()
    {
        delete fHelper;
    };

    template< typename XType >
    void SetInput(XType* ptr)
    {
        if(fHelper){delete fHelper; fHelper = nullptr;}
        fHelper = new helper_specific<XType>(ptr);
    }

    virtual void Initialize() override
    {
        fHelper->init();
    }


    //delayed dispatch to type erased helper
    virtual void Execute() override
    {
        fHelper->exe();
    }

    private:

        helper_base* fHelper;
}




class impl_2: public impl_base
{
    public impl_2():fHelper(nullptr){};
    virtual ~impl_2()
    {
        delete fHelper;
    };

    template< typename XType1, typename XType2 >
    void SetInput(XType1* ptr1, XType2* ptr2)
    {
        if(fHelper){delete fHelper; fHelper = nullptr;}
        fHelper = new helper_specific<XType1, XType2>(ptr1, ptr2);
    }

    virtual void Initialize() override
    {
        fHelper->init();
    }

    //delayed dispatch to type erased helper
    virtual void Execute() override
    {
        fHelper->exe();
    }

    private:

        helper_base* fHelper;

}
