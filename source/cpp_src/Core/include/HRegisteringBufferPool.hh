#ifndef HRegisteringBufferPool_HH__
#define HRegisteringBufferPool_HH__


namespace hops
{

    /*
    *File: HMultiConsumerBufferPool.hh
    *Class: HMultiConsumerBufferPool
    *Author: J. Barrett
    *Email: barrettj@mit.edu
    *Date: Thu Oct  4 21:23:05 EDT 2018
    *Description: ring of data buffers with registered set of consumers
    */

//forward declare registering buffer pool
class HRegisteringBufferPool;

class HRegisteredConsumer
{
    public:
        HRegisteredConsumer():fID(0){};
        virtual ~HRegisteredConsumer(){};

        unsigned int GetConsumerID() const {return fID;};
        
        unsigned int GetNextConsumerID() const {return fID+1;};

    private:
        friend class HRegisteringBufferPool;

    protected:
        unsigned int fID;
};


class HRegisteringBufferPool
{
    public:
        HRegisteringBufferPool():fNRegisteredConsumers(0){};
        virtual ~HRegisteringBufferPool(){};

        virtual void Initialize(){};

        bool IsRegistered(HRegisteredConsumer* consumer) const
        {
            bool is_present = false;
            for(unsigned int i=0; i<fConsumerList.size(); i++)
            {
                if(fConsumerList[i] == consumer){is_present = true;};
            }
            return is_present;
        }

        void RegisterConsumer(HRegisteredConsumer* consumer)
        {
            bool is_present = IsRegistered(consumer);
            if(!is_present)
            {
                consumer->fID = fConsumerList.size();
                fConsumerList.push_back(consumer);
                //std::cout<<"registering consumer: "<<consumer<<" with id: "<<consumer->fID<<" for buffer pool: "<<this<<std::endl;
            }
            fNRegisteredConsumers = fConsumerList.size();
        }

    protected:

        std::vector<HRegisteredConsumer*> fConsumerList;
        unsigned int fNRegisteredConsumers;
};


}


#endif /* end of include guard: HRegisteringBufferPool_HH__ */
