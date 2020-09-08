#pragma once

#include <type_traits>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Common/ASignal.h>


template <typename Model>
class ADataBinding;

template<typename Model, typename Klass, typename Data>
class ADataBindingLinker {
private:
    ADataBinding<Model>* mBinder;
    void(Klass::*mSetterFunc)(Data);
    std::decay_t<Data>(Model::*mField);
    ASignal<std::decay_t<Data>>(Klass::*mGetter);

public:
    ADataBindingLinker(ADataBinding<Model>* binder, ASignal<std::decay_t<Data>>(Klass::*getter), void (Klass::*setterFunc)(Data),
                       std::decay_t<Data>(Model::*field)):
        mBinder(binder), mGetter(getter), mSetterFunc(setterFunc), mField(field) {}

    ADataBinding<Model>* getBinder() const {
        return mBinder;
    }

    auto getSetterFunc() const {
        return mSetterFunc;
    }

    auto getField() const {
        return mField;
    }

    auto getGetter() const {
        return mGetter;
    }
};

template <typename Model>
class ADataBinding: public AObject {
private:
    using Applier = std::function<void(const Model& model)>;
    ADeque<Applier> mLinkAppliers;

    Model mModel;

public:

    virtual ~ADataBinding() = default;

    template<typename Klass, typename Data>
    ADataBindingLinker<Model, Klass, Data> link(std::decay_t<Data>(Model::*field), void(Klass::*setterFunc)(Data)) {
        return ADataBindingLinker<Model, Klass, Data>(this, nullptr, setterFunc, field);
    }
    template<typename Klass, typename Data>
    ADataBindingLinker<Model, Klass, Data> link(std::decay_t<Data>(Model::*field), ASignal<std::decay_t<Data>>(Klass::*getter), void(Klass::*setterFunc)(Data)) {
        return ADataBindingLinker<Model, Klass, Data>(this, getter, setterFunc, field);
    }

    const Model& getModel() const {
        return mModel;
    }
    Model& getEditableModel() {
        return mModel;
    }
    void setModel(const Model& model) {
        mModel = model;
        update();
    }
    void update() {
        for (auto& applier : mLinkAppliers) {
            applier(mModel);
        }
    }

    void addApplier(const Applier& applier) {
        mLinkAppliers << applier;
    }
};

template<typename Klass1, typename Klass2, typename Model, typename Data>
_<Klass1> operator&&(const _<Klass1>& object, const ADataBindingLinker<Model, Klass2, Data>& linker) {
    if (linker.getGetter()) {
        AObject::connect(object.get()->*(linker.getGetter()), linker.getBinder(), [object, linker](const Data& data) {
            object->setSignalsEnabled(false);
            linker.getBinder()->getEditableModel().*(linker.getField()) = data;
            linker.getBinder()->update();
            object->setSignalsEnabled(true);
        });
    }

    linker.getBinder()->addApplier([object, linker](const Model& model) {
        (object.get()->*(linker.getSetterFunc()))(model.*(linker.getField()));
    });

    return object;
}
