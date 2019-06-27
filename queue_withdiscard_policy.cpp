/*
    Producer/Consumer queue, with discard policy
*/

#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <deque>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace policy 
{
    template <class C>
    struct discard_none {
        bool operator()(C &container) const { 
            return false;
        }
    };

    template <class C>
    struct discard_all {
        bool operator()(C &container) const { 
            container.clear();
            return true;
        }
    };

    template <class C>
    struct discard_last {
        bool operator()(C &container) const { 
            container.pop_back();
            return true;
        }
    };

    template <class C>
    struct discard_first {
        bool operator()(C &container) const { 
           container.pop_front();
           return true;
        }
    };
}

struct TimeOut{};
template<typename T, typename Container=std::deque<T>, typename DiscardPolicy= policy::discard_none<Container>>
class Queue
{
public:
    ~Queue() {std::cout<<"queue::dtro"<<std::endl;}
    Queue(int64_t size)
    : m_size(size)
    { }

    template<typename ...Args>
    bool enqueue(Args&& ...args)
    {
        {
            std::lock_guard<std::mutex> lck{m_mutex};
            if(int64_t(m_queue.size())==m_size) {
                if(!DiscardPolicy()(m_queue)) {
                    return false;
                }
            }
            m_queue.emplace_back(std::forward<Args>(args)...);
        }
        m_cv.notify_all();
        return true;
    }

    int64_t size()
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        return m_queue.size();
        
    }

    bool dequeue(T& data, int64_t timeo=-1)
    {
        auto t0 = std::chrono::system_clock::now();
        while(1) {
            std::unique_lock<std::mutex> lck{m_mutex};
            if(m_queue.size()) {
               data = move(m_queue.front());
               m_queue.pop_front();
               return true;
            }
            if(timeo == 0) return false;
            process_waiting(t0,lck,timeo);
        }
    }

    void process_waiting(std::chrono::time_point<std::chrono::system_clock>& t0,
                         std::unique_lock<std::mutex>& lck,
                         int64_t timeo)
    {
        m_nr_waiting++;
        if(timeo > 0) {
            auto t1 = std::chrono::system_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count();
            if(diff > timeo || 
               !m_cv.wait_for(lck,
                              std::chrono::milliseconds(diff - timeo),
                              [this]{return m_queue.size();}))
            {
                 m_nr_waiting--;
                 throw TimeOut();
            }
        }else {
            m_cv.wait(lck,[this]{return m_queue.size();});
        }
        m_nr_waiting--;
    }
private:
    int64_t m_size;
    Container m_queue;
    std::mutex    m_mutex;
    std::condition_variable m_cv;
    int m_nr_waiting {0} ;
};

class Caller
{
public:
    template<typename ...Args>
    static void Call(Args&&...args)
    {
        queue().enqueue(std::forward<Args>(args)...);
    }
private:
    using que_type = Queue<std::string>;
    ~Caller()
    {
        std::cout<<"dtor....."<<std::endl;
        m_started = false;
        if(m_thread.joinable())
            m_thread.join();
    }
    Caller()
        : m_que_size{1024}, 
          m_nr_ques{1},
          m_thread([this]{worker();}),
          m_started{true}
    {
    }
    static que_type& queue()
    {
        static thread_local std::shared_ptr<que_type> q;
        if(q == nullptr) {
            q = std::make_shared<que_type>(instance().m_que_size);
            std::lock_guard<std::mutex> lck{instance().m_mutex};
            instance().m_ques.emplace_back(q);
        }
        return *q;
    }
    static Caller& instance()
    {
        static Caller instance;
        return instance;
    }
    void worker()
    {
        std::cout<<"starting...."<<std::endl;
        while(m_started || m_nr_ques) 
        {
          {
            std::lock_guard<std::mutex> lck{m_mutex};
            for(auto it = m_ques.begin();it != m_ques.end();) {
               auto &que = *it;
               std::string msg;
               if(que->dequeue(msg,0)) {
                   std::cout<<msg<<std::endl;
               }
               if(que.unique() && !que->size()) 
                   it = m_ques.erase(it);
               else ++it;
            }
          }
          m_nr_ques = m_ques.size();
        }
    }
    int64_t m_que_size;
    size_t m_nr_ques;
    std::vector<std::shared_ptr<que_type>> m_ques;
    std::mutex  m_mutex;
    std::thread m_thread;
    bool   m_started;
};

struct worker
{
    worker() = default;
    ~worker() {m_thread.join();}
    void start()
    {
         m_thread= std::thread([this]{work();});
    }
    void work() {
      using namespace std::chrono_literals;
      for (int i = 0; i < 5; ++i) {
        Caller::Call("test"+std::to_string(i));
        std::this_thread::sleep_for(100ms);
      }
    }
    std::thread m_thread;
};
int main()

{
    {
    int nthreads = 4;
    std::vector<worker> ths(nthreads);
    for (auto &t : ths) {
       t.start();
    }
    }
    getchar();
}

