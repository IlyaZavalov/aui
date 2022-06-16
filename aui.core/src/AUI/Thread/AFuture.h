/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <atomic>
#include <functional>
#include <optional>
#include "AConditionVariable.h"
#include "AMutex.h"
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AException.h>
#include <AUI/Reflect/AReflect.h>

class AThreadPool;


class AInvocationTargetException: public AException {
private:
    AString mExceptionType;
    AString mMessage;

public:
    AInvocationTargetException(AString message, AString exceptionType):
        mMessage(std::move(message)),
        mExceptionType(std::move(exceptionType)) {}

    AString getMessage() const noexcept override {
        return "Exception of type " + mExceptionType + " has caught by AFuture: " + mMessage;
    }

    ~AInvocationTargetException() noexcept override = default;
};


/**
 * Controls <code>AFuture::wait</code> behaviour.
 * @see AFuture::wait
 */
AUI_ENUM_FLAG(AFutureWait) {
    DEFAULT = 0b01,
    ASYNC_ONLY = 0b00
};


namespace aui::impl::future {
    /**
     * This class calls cancel() and wait() methods of AFuture::Inner BEFORE AFuture::Inner destruction in order to keep
     * alive the weak reference created in AThreadPool::operator<<.
     */
    template<typename Inner>
    struct CancellationWrapper {
        _<Inner> wrapped;

        explicit CancellationWrapper(_<Inner> wrapped) : wrapped(std::move(wrapped)) {}

        ~CancellationWrapper() {
            wrapped->cancel();
            wrapped->waitForTask();
        }

        [[nodiscard]]
        const _<Inner>& sharedPtr() noexcept {
            return wrapped;
        }

        Inner* operator->() const noexcept {
            return wrapped.get();
        }
    };

    template<typename T>
    struct OnSuccessCallback {
        using type = std::function<void(const T& value)>;
    };

    template<>
    struct OnSuccessCallback<void> {
        using type = std::function<void()>;
    };
    template<typename Value = void>
    class Future
    {
    public:
        static constexpr bool isVoid = std::is_same_v<void, Value>;
        using TaskCallback = std::function<Value()>;

        using OnSuccessCallback = typename OnSuccessCallback<Value>::type;

        struct Inner {
            bool interrupted = false;
            /**
             * When isVoid = true, bool is a flag whether "void value" supplied or not
             */
            std::conditional_t<isVoid, bool, std::optional<Value>> value;
            std::optional<AInvocationTargetException> exception;
            AMutex mutex;
            AConditionVariable cv;
            AAbstractThread* worker = nullptr;
            TaskCallback task;
            OnSuccessCallback onSuccess;
            std::function<void(const AException& exception)> onError;
            _<AAbstractThread> thread;
            bool cancelled = false;

            explicit Inner(std::function<Value()> task) noexcept: task(std::move(task)) {
                if constexpr(isVoid) {
                    value = false;
                }
            }

            void waitForTask() noexcept {
                std::unique_lock lock(mutex);
                bool rethrowInterrupted = false;
                while ((thread) && !hasResult()) {
                    try {
                        cv.wait(lock);
                    } catch (const AThread::Interrupted& i) {
                        rethrowInterrupted = true;
                    }
                }
                if (rethrowInterrupted) {
                    AThread::current()->interrupt();
                }
            }

            [[nodiscard]]
            bool isWaitNeeded() noexcept {
                return (thread || !cancelled) && !hasResult();
            }

            [[nodiscard]]
            bool hasResult() const noexcept {
                return value || exception || interrupted;
            }

            [[nodiscard]]
            bool hasValue() const noexcept {
                return bool(value);
            }

            bool setThread(_<AAbstractThread> thr) noexcept {
                std::unique_lock lock(mutex);
                if (cancelled) return true;
                if (thread) return true;
                thread = std::move(thr);
                return false;
            }

            void wait(const _weak<Inner>& innerWeak, AFutureWait flags = AFutureWait::DEFAULT) noexcept {
                std::unique_lock lock(mutex);
                try {
                    if ((thread || !cancelled) && !hasResult() && int(flags) & 0b1 && task) {
                        // task is not have picked up by the threadpool; execute it here
                        lock.unlock();
                        if (tryExecute(innerWeak)) {
                            return;
                        }
                        lock.lock();
                    }
                    while ((thread || !cancelled) && !hasResult()) {
                        cv.wait(lock);
                    }
                } catch (const AThread::Interrupted& e) {
                    e.needRethrow();
                }
            }

            void cancel() noexcept {
                std::unique_lock lock(mutex);
                if (!cancelled) {
                    cancelled = true;
                    if (thread && !hasResult()) {
                        thread->interrupt();
                    }
                }
            }

            /**
             * @brief Executes the task stored in the future.
             * Using weak_ptr to internal object in order to make possible Future cancellation by it's destruction.
             * @param innerWeak self weak_ptr
             * @return true, then task is successfully executed and supplied result.
             */
            bool tryExecute(const _weak<Inner>& innerWeak) {
                /*
                 * We should assume that this == nullptr or invalid.
                 * We can assert that this pointer is safe only if we hold at least one shared_ptr.
                 * It allows callers to pass a weak_ptr in any state.
                 */
                if (auto inner = innerWeak.lock()) {
                    if (value) return false;
                    if (inner->setThread(AThread::current())) return false;
                    try {
                        if (task == nullptr) { // task is executed in wait() function
                            return false;
                        }
                        std::unique_lock lock(mutex);
                        auto func = std::move(task);
                        lock.unlock();
                        inner = nullptr;
                        if constexpr(isVoid) {
                            func();
                            if (auto sharedPtrLock = innerWeak.lock()) {
                                std::unique_lock lock(mutex);
                                value = true;
                                cv.notify_all();
                                nullsafe(onSuccess)();
                            }
                        } else {
                            auto result = func();
                            if (auto sharedPtrLock = innerWeak.lock()) {
                                std::unique_lock lock(mutex);
                                value = std::move(result);
                                cv.notify_all();
                                nullsafe(onSuccess)(*value);
                            }
                        }
                    } catch (const AException& e) {
                        nullsafe(innerWeak.lock())->reportException(e);
                        return false;
                    } catch (...) {
                        nullsafe(innerWeak.lock())->reportInterrupted();
                        throw;
                    }
                }
                return true;
            }

            void reportInterrupted() noexcept {
                std::unique_lock lock(mutex);
                interrupted = true;
                cv.notify_all();
            }

            void reportException(const AException& e) noexcept {
                std::unique_lock lock(mutex);
                exception.emplace(e.getMessage(), AReflect::name(&e)); // NOLINT(bugprone-throw-keyword-missing)
                cv.notify_all();
                nullsafe(onError)(*exception);
            }

        };

    protected:
        _<CancellationWrapper<Inner>> mInner;


    public:
        /**
         * @param task a callback which will be executed by Future::Inner::tryExecute. Can be null. If null, the result
         *        should be provided by AFuture::supplyResult function.
         */
        Future(TaskCallback task = nullptr) noexcept: mInner(_new<CancellationWrapper<Inner>>(aui::ptr::manage(new Inner(std::move(task))))) {}

        [[nodiscard]]
        const _<CancellationWrapper<Inner>>& inner() {
            return mInner;
        }

        /**
         * @return true if call to wait() function would cause thread block.
         */
        [[nodiscard]]
        bool isWaitNeeded() const noexcept {
            return (*mInner)->isWaitNeeded();
        }

        /**
         * @return true if the value or exception or interruption received.
         */
        [[nodiscard]]
        bool hasResult() const noexcept {
            return (*mInner)->hasResult();
        }

        /**
         * @return true if the value can be obtained without waiting.
         */
        [[nodiscard]]
        bool hasValue() const noexcept {
            return (*mInner)->hasValue();
        }

        void reportException(const AException& e) noexcept {
            (*mInner)->reportException(e);
        }

        template<typename Callback>
        Future& onSuccess(Callback&& callback) noexcept {
            std::unique_lock lock((*mInner)->mutex);
            if constexpr(isVoid) {
                if ((*mInner)->onSuccess) {
                    (*mInner)->onSuccess = [prev = std::move((*mInner)->onSuccess),
                                            innerStorage = mInner,
                                            callback = std::forward<Callback>(callback)]() {
                        prev();
                        callback();
                    };
                } else {
                    (*mInner)->onSuccess = [innerStorage = mInner, callback = std::forward<Callback>(callback)]() {
                        callback();
                    };
                }
            } else {
                if ((*mInner)->onSuccess) {
                    (*mInner)->onSuccess = [prev = std::move((*mInner)->onSuccess),
                            innerStorage = mInner,
                            callback = std::forward<Callback>(callback)](const Value& v) {
                        prev(v);
                        callback(v);
                    };
                } else {
                    (*mInner)->onSuccess = [innerStorage = mInner, callback = std::forward<Callback>(callback)](
                            const Value& v) {
                        callback(v);
                    };
                }
            }
            return *this;
        }

        template<typename Callback>
        Future& onError(Callback&& callback) noexcept {
            std::unique_lock lock((*mInner)->mutex);

            if ((*mInner)->onError) {
                (*mInner)->onError = [prev = std::move((*mInner)->onError),
                        innerStorage = mInner,
                        callback = std::forward<Callback>(callback)](const AException& v) {
                    prev(v);
                    callback(v);
                };
            } else {
                (*mInner)->onError = [innerStorage = mInner, callback = std::forward<Callback>(callback)](
                        const AException& v) {
                    callback(v);
                };
            }
            return *this;
        }


        void cancel() noexcept {
            (*mInner)->cancel();
        }

        void reportInterrupted() {
            (*mInner)->reportInterrupted();
        }

        /**
         * Sleeps if the supplyResult is not currently available.
         * @note The task will be executed inside wait() function if the threadpool have not taken the task to execute
         *       yet. This behaviour can be disabled by <code>AFutureWait::ASYNC_ONLY</code> flag.
         */
        void wait(AFutureWait flags = AFutureWait::DEFAULT) noexcept {
            (*mInner)->wait(mInner->sharedPtr(), flags);
        }

        /**
         * Returns the supplyResult from the another thread. Sleeps if the supplyResult is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        decltype(auto) get(AFutureWait flags = AFutureWait::DEFAULT) {
            AThread::interruptionPoint();
            (*mInner)->wait(mInner->sharedPtr(), flags);
            AThread::interruptionPoint();
            if ((*mInner)->exception) {
                throw *(*mInner)->exception;
            }
            if ((*mInner)->interrupted) {
                throw AInvocationTargetException("Future execution interrupted", "AThread::Interrupted");
            }
            if constexpr(!isVoid) {
                return *(*mInner)->value;
            }
        }

        /**
         * Returns the supplyResult from the another thread. Sleeps if the supplyResult is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        decltype(auto) operator*() {
            return get();
        }

        /**
         * Returns the supplyResult from the another thread. Sleeps if the supplyResult is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        Value* operator->() {
            return &operator*();
        }

        /**
         * Returns the supplyResult from the another thread. Sleeps if the supplyResult is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        Value const * operator->() const {
            return &operator*();
        }

        /**
         * Returns the task result from the another thread. Sleeps if the task result is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        decltype(auto) operator*() const {
            return **const_cast<Future*>(this);
        }

    };

}


template<typename T = void>
class AFuture final: public aui::impl::future::Future<T> {
private:
    using super = typename aui::impl::future::Future<T>;

public:
    using Task = typename super::TaskCallback;

    AFuture(Task task = nullptr) noexcept: super(std::move(task)) {}

    void supplyResult(T v) noexcept {
        auto& inner = (*super::mInner);
        assert(("task is already provided", inner->task == nullptr));

        std::unique_lock lock(inner->mutex);
        inner->value = std::move(v);
        inner->cv.notify_all();
        nullsafe(inner->onSuccess)(*inner->value);
    }

    AFuture& operator=(std::nullptr_t) noexcept {
        super::mInner = nullptr;
        return *this;
    }

    [[nodiscard]]
    bool operator==(const AFuture& r) const noexcept {
        return super::mInner == r.mInner;
    }
};

template<>
class AFuture<void> final: public aui::impl::future::Future<void> {
private:
    using T = void;
    using super = typename aui::impl::future::Future<T>;

public:
    using Task = typename super::TaskCallback;

    AFuture(Task task = nullptr) noexcept: super(std::move(task)) {}

    void supplyResult() noexcept {
        auto& inner = (*super::mInner);
        assert(("task is already provided", inner->task == nullptr));

        std::unique_lock lock(inner->mutex);
        inner->value = true;
        inner->cv.notify_all();
        nullsafe(inner->onSuccess)();
    }

    AFuture& operator=(std::nullptr_t) noexcept {
        super::mInner = nullptr;
        return *this;
    }

    [[nodiscard]]
    bool operator==(const AFuture& r) const noexcept {
        return super::mInner == r.mInner;
    }
};


#include <AUI/Thread/AThreadPool.h>
#include <AUI/Common/AException.h>
