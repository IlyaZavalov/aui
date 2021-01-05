#pragma once
#include "AUI/Common/SharedPtr.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/AException.h"

/**
 * \brief Minimal Dependency Injection implementation
 */
namespace Autumn
{
	namespace detail
	{
		template <typename T>
		_<T>& storage()
		{
			static _<T> t = nullptr;
			return t;
		}
		template <typename T>
		AMap<AString, _<T>>& storageMap()
		{
			static AMap<AString, _<T>> t;
			return t;
		}
	}
	
	template <typename T>
	_<T> get()
	{
		return detail::storage<T>();
	}
	template <typename T>
	void put(_<T> obj)
	{
		detail::storage<T>() = obj;
	}


	template <typename T>
	_<T> get(const AString& name)
	{
		if (auto c = detail::storageMap<T>().contains(name))
		{
			return c->second;
		}
		throw AException("No such object in Autumn: " + name);
	}
	template <typename T>
	void put(const AString& name, _<T> obj)
	{
		detail::storageMap<T>()[name] = obj;
	}



    /**
     * \brief Temporary replace object in Autumn.
     * \tparam object type.
     */
    template <typename T>
    class Temporary {
    private:
        _<T> mPrevValue;
        std::function<void()> mOnDeleteCallback;

    public:
        Temporary(const _<T>& newValue) {
            mPrevValue = Autumn::get<T>();
            Autumn::put(newValue);
        }

        ~Temporary() {
            Autumn::put(mPrevValue);
            if (mOnDeleteCallback)
                mOnDeleteCallback();
        }

        void onDelete(const std::function<void()>& callback) {
            mOnDeleteCallback = callback;
        }
    };

    template <class T>
    struct construct {
        template<typename... Args>
	    static _<T> with() {
            return _new<T>(Autumn::get<Args>()...);
        }
    };
}