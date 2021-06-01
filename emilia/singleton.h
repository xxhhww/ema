#ifndef _EMILIA_SINGLETON_H_
#define _EMILIA_SINGLETON_H_

//返回一个T的实例指针
namespace emilia
{
    template <class T, class x = void, int N = 0>
    class singleton
    {
    public:
        static T* GetInstance(){
            static T v;
            return &v;
        }
    };
}

#endif