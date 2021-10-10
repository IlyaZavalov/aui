//
// Created by alex2 on 6/9/2021.
//

#pragma once

#include <AUI/View/AViewContainer.h>
#include "AWindowManager.h"

class API_AUI_VIEWS ABaseWindow: public AViewContainer {

private:
    _weak<AView> mFocusedView;
    glm::ivec2 mMousePos;

protected:
    float mDpiRatio = 1.f;
    bool mIsFocused = true;

    static ABaseWindow*& currentWindowStorage();

    static void checkForStencilBits();

public:
    ABaseWindow();

    virtual ~ABaseWindow() = default;

    AWindowManager& getWindowManager() const;

    float getDpiRatio()
    {
        return mDpiRatio;
    }

    _<AView> getFocusedView() const
    {
        return mFocusedView.lock();
    }

    void setFocusedView(const _<AView>& view);
    void focusNextView();

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseMove(glm::ivec2 pos) override;

    bool isFocused() const {
        return mIsFocused;
    }

    [[nodiscard]]
    const glm::ivec2& getMousePos() const {
        return mMousePos;
    }


    void onKeyDown(AInput::Key key) override;

    virtual void flagRedraw();
    virtual void flagUpdateLayout();

    void onKeyUp(AInput::Key key) override;

    void onCharEntered(wchar_t c) override;

    void makeCurrent() {
        currentWindowStorage() = this;
    }

    /**
     * Create a surface used for context menus and combo boxes. Closure of the surface is managed by window but can be
     * closed manually by calling closeOverlappingSurface.
     * @param position position where does the surface should be created. It's not exact the top left corner of the
     *        surface but a hint (i.e. if the surface does not fit)
     * @param size size
     * @return a new surface.
     */
    virtual _<AViewContainer> createOverlappingSurface(const glm::ivec2& position, const glm::ivec2& size) = 0;
    virtual void closeOverlappingSurface(AViewContainer* surface) = 0;

signals:
    emits<>            dpiChanged;
    emits<glm::ivec2>  mouseMove;
    emits<AInput::Key> keyDown;
};


