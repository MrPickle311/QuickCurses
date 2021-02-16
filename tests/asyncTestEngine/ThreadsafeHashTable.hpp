#include <functional>
#include <list>
#include <shared_mutex>
#include <mutex>
#include <algorithm>
#include <vector>

template<typename Key,typename Value,typename Hash = std::hash<Key>>
class ThreadsafeHashTable
{
private:
    class BucketType
    {
    private:
        using BucketValue = std::pair<Key, Value>;
        using BucketData = std::list<BucketValue>;
        using BucketIterator = typename BucketData::iterator;
        using BucketConstIterator = typename BucketData::const_iterator;
        
        BucketData data_;

        mutable std::shared_mutex mutex_;

        BucketIterator findEntryFor(Key const& key) //possible data-race
        {
            return  std::find_if(data_.begin(), data_.end(),[&](BucketValue const& item)
                                                            {return item.first == key;}); 
        }

        BucketConstIterator findConstEntryFor(Key const& key) const //for non-modyfing operations
        {
            return std::find_if(data_.begin(), data_.end(),[&](BucketValue const& item)
                                                            {return item.first == key;});  
        }

    public:
        Value valueFor(Key const& key,Value const& default_value) const//no data-race -> const at the end
        {
            std::shared_lock<std::shared_mutex> lock{mutex_};//shared-lock -> safe shared access
            BucketConstIterator found_entry {findConstEntryFor(key)};
            return (found_entry == data_.cend() ? default_value : found_entry->second);
        }

        //push or update value in single bucket
        void addOrUpdateMapping(Key const& key,Value const& value) //not const, possible data-race
        {
            std::unique_lock<std::shared_mutex> lock{mutex_}; //but not this time
            BucketIterator found_entry {findEntryFor(key)};
            if(found_entry == data_.end())
                data_.push_back(BucketValue{key,value});
            else found_entry->second = value;
        }

        void removeMapping(Key const& key)
        {
            std::unique_lock<std::shared_mutex> lock{mutex_};
            BucketIterator found_entry {findEntryFor(key)};
            if(found_entry != data_.end())
                data_.erase(found_entry);
        }

        friend bool operator==(BucketType const& left, BucketType const& right)
        {
            std::scoped_lock lock{left.mutex_, right.mutex_};
            return left.data_ == right.data_;
        }

        friend bool operator!=(BucketType const& left, BucketType const& right)
        {
            return !operator==(left,right);
        }
    };
private:
    //i do not trust it
    std::vector<std::unique_ptr<BucketType>> buckets_;
    Hash hasher_;
    mutable std::shared_mutex sensitive_operation_mutex_;// equality operator etc..

    BucketType& getBucket(Key const& key) const //possible data-race
    {
        std::shared_lock<std::shared_mutex> multithreaded_lock{sensitive_operation_mutex_};
        std::size_t const bucket_index {hasher_(key) % buckets_.size()}; //map hasher_(key) to size
        return *buckets_[bucket_index]; //unique_ptr dereference
    }
    friend bool compareTables(ThreadsafeHashTable const& left, 
                       ThreadsafeHashTable const& right)
    {
        if(left.size() != right.size())
            return false;
        for(size_t i = 0; i < left.size(); ++i)
        {
            if(*(left.buckets_[i]) != *(right.buckets_[i]))
                return false;
        }
        return true;
    }
public:
    using KeyType = Key;
    using MappedType = Value;
    using HashType = Hash;

    ThreadsafeHashTable(
            size_t buckets_number = 30,
            Hash const& hasher = Hash{}):
        buckets_{buckets_number}, 
        hasher_{hasher}
    {
        for (size_t i = 0; i < buckets_number; ++i)//change to range loop
            buckets_[i].reset(new BucketType{});
    }

    ThreadsafeHashTable(ThreadsafeHashTable const& other)=delete;
    ThreadsafeHashTable& operator=(ThreadsafeHashTable const& other)=delete;

    Value valueFor(Key const& key,Value const& default_value = Value{}) const //threadsafe
    {
        return getBucket(key).valueFor(key,default_value);
    }

    void addOrUpdateMapping(Key const& key,Value const& value)
    {
        getBucket(key).addOrUpdateMapping(key,value);
    }

    void removeMapping(Key const& key)
    {
        getBucket(key).removeMapping(key);
    }

    size_t size() const
    {
        return buckets_.size();
    }

    friend bool operator==(ThreadsafeHashTable const& left, 
                           ThreadsafeHashTable const& right)
    {
        std::scoped_lock<std::shared_mutex,std::shared_mutex> 
                                            singlethreaded_lock{
                                             left.sensitive_operation_mutex_,
                                             right.sensitive_operation_mutex_};
        return compareTables(left,right);
    }

    friend bool operator!=(ThreadsafeHashTable const& left, 
                           ThreadsafeHashTable const& right)
    {
        return !operator==(left,right);
    }
};
//Such a buckets structure is because it need to store every key.
//Hashing and the modulo operation may blur the keys which have similar hash values