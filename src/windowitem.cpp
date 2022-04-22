/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "windowitem.h"
#include "decorationitem.h"
#include "deleted.h"
#include "internal_client.h"
#include "shadowitem.h"
#include "surfaceitem_internal.h"
#include "surfaceitem_wayland.h"
#include "surfaceitem_x11.h"
#include "window.h"

namespace KWin
{

WindowItem::WindowItem(Window *window, Item *parent)
    : Item(parent)
    , m_window(window)
{
    connect(window, &Window::decorationChanged, this, &WindowItem::updateDecorationItem);
    updateDecorationItem();

    connect(window, &Window::shadowChanged, this, &WindowItem::updateShadowItem);
    updateShadowItem();

    connect(window, &Window::windowClosed, this, &WindowItem::handleWindowClosed);
}

SurfaceItem *WindowItem::surfaceItem() const
{
    return m_surfaceItem.data();
}

DecorationItem *WindowItem::decorationItem() const
{
    return m_decorationItem.data();
}

ShadowItem *WindowItem::shadowItem() const
{
    return m_shadowItem.data();
}

Window *WindowItem::window() const
{
    return m_window;
}

void WindowItem::handleWindowClosed(Window *original, Deleted *deleted)
{
    Q_UNUSED(original)
    m_window = deleted;
}

void WindowItem::updateSurfaceItem(SurfaceItem *surfaceItem)
{
    m_surfaceItem.reset(surfaceItem);

    if (m_surfaceItem) {
        connect(m_window, &Window::shadeChanged, this, &WindowItem::updateSurfaceVisibility);
        connect(m_window, &Window::bufferGeometryChanged, this, &WindowItem::updateSurfacePosition);
        connect(m_window, &Window::frameGeometryChanged, this, &WindowItem::updateSurfacePosition);

        updateSurfacePosition();
        updateSurfaceVisibility();
    } else {
        disconnect(m_window, &Window::shadeChanged, this, &WindowItem::updateSurfaceVisibility);
        disconnect(m_window, &Window::bufferGeometryChanged, this, &WindowItem::updateSurfacePosition);
        disconnect(m_window, &Window::frameGeometryChanged, this, &WindowItem::updateSurfacePosition);
    }
}

void WindowItem::updateSurfacePosition()
{
    const QRect bufferGeometry = m_window->bufferGeometry();
    const QRect frameGeometry = m_window->frameGeometry();

    m_surfaceItem->setPosition(bufferGeometry.topLeft() - frameGeometry.topLeft());
}

void WindowItem::updateSurfaceVisibility()
{
    m_surfaceItem->setVisible(!m_window->isShade());
}

void WindowItem::updateShadowItem()
{
    Shadow *shadow = m_window->shadow();
    if (shadow) {
        if (!m_shadowItem || m_shadowItem->shadow() != shadow) {
            m_shadowItem.reset(new ShadowItem(shadow, m_window, this));
        }
        if (m_decorationItem) {
            m_shadowItem->stackBefore(m_decorationItem.data());
        } else if (m_surfaceItem) {
            m_shadowItem->stackBefore(m_decorationItem.data());
        }
    } else {
        m_shadowItem.reset();
    }
}

void WindowItem::updateDecorationItem()
{
    if (m_window->isDeleted() || m_window->isZombie()) {
        return;
    }
    if (m_window->decoration()) {
        m_decorationItem.reset(new DecorationItem(m_window->decoration(), m_window, this));
        if (m_shadowItem) {
            m_decorationItem->stackAfter(m_shadowItem.data());
        } else if (m_surfaceItem) {
            m_decorationItem->stackBefore(m_surfaceItem.data());
        }
    } else {
        m_decorationItem.reset();
    }
}

WindowItemX11::WindowItemX11(Window *window, Item *parent)
    : WindowItem(window, parent)
{
    initialize();

    // Xwayland windows and Wayland surfaces are associated asynchronously.
    connect(window, &Window::surfaceChanged, this, &WindowItemX11::initialize);
}

void WindowItemX11::initialize()
{
    switch (kwinApp()->operationMode()) {
    case Application::OperationModeX11:
        updateSurfaceItem(new SurfaceItemX11(window(), this));
        break;
    case Application::OperationModeXwayland:
        if (!window()->surface()) {
            updateSurfaceItem(nullptr);
        } else {
            updateSurfaceItem(new SurfaceItemXwayland(window(), this));
        }
        break;
    case Application::OperationModeWaylandOnly:
        Q_UNREACHABLE();
    }
}

WindowItemWayland::WindowItemWayland(Window *window, Item *parent)
    : WindowItem(window, parent)
{
    updateSurfaceItem(new SurfaceItemWayland(window->surface(), window, this));
}

WindowItemInternal::WindowItemInternal(InternalClient *window, Item *parent)
    : WindowItem(window, parent)
{
    updateSurfaceItem(new SurfaceItemInternal(window, this));
}

} // namespace KWin
