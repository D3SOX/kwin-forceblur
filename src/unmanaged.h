/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2006 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWIN_UNMANAGED_H
#define KWIN_UNMANAGED_H

#include <netwm.h>

#include "abstract_client.h"

namespace KWin
{

class KWIN_EXPORT Unmanaged : public AbstractClient
{
    Q_OBJECT
public:
    explicit Unmanaged();
    bool windowEvent(xcb_generic_event_t *e);
    bool track(xcb_window_t w);
    bool hasScheduledRelease() const;
    static void deleteUnmanaged(Unmanaged *c);
    int desktop() const override;
    QStringList activities() const override;
    QVector<VirtualDesktop *> desktops() const override;
    QPoint clientPos() const override;
    Layer layer() const override
    {
        return UnmanagedLayer;
    }
    NET::WindowType windowType(bool direct = false, int supported_types = 0) const override;
    bool isOutline() const override;

    QString captionNormal() const override { return {}; }
    QString captionSuffix() const override { return {}; }
    bool isCloseable() const override { return false; }
    bool isShown() const override { return false; }
    bool isHiddenInternal() const override { return false; }
    void hideClient() override { /* nothing to do */ }
    void showClient() override { /* nothing to do */ }
    AbstractClient *findModal(bool /*allow_itself*/) override { return nullptr; }
    bool isResizable() const override { return false; }
    bool isMovable() const override { return false; }
    bool isMovableAcrossScreens() const override { return false; }
    bool takeFocus() override { return false; }
    bool wantsInput() const override { return false; }
    void killWindow() override { /* nothing to do */ }
    void destroyClient() override { /* nothing to do */ }
    void closeWindow() override { /* nothing to do */ }
    bool acceptsFocus() const override { return false; }
    bool belongsToSameApplication(const AbstractClient *other, SameApplicationChecks /*checks*/) const override { return other == this; }
    void moveResizeInternal(const QRect & /*rect*/, KWin::AbstractClient::MoveResizeMode /*mode*/) override { /* nothing to do */ }
    void updateCaption() override { /* nothing to do */ }
    void resizeWithChecks(const QSize&) override { /* nothing to do */ }

public Q_SLOTS:
    void release(ReleaseReason releaseReason = ReleaseReason::Release);

private:
    ~Unmanaged() override; // use release()
    // handlers for X11 events
    void configureNotifyEvent(xcb_configure_notify_event_t *e);
    void damageNotifyEvent();
    QWindow *findInternalWindow() const;
    void checkOutput();
    void associate();
    void initialize();
    bool m_outline = false;
    bool m_scheduledRelease = false;
};

} // namespace

#endif
