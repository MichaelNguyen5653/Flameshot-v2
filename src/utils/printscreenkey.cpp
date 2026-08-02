// SPDX-License-Identifier: GPL-3.0-or-later

#include "printscreenkey.h"

#if defined(Q_OS_WIN) || defined(_WIN32)

#include <QSettings>

namespace {
constexpr auto KeyboardKey = "HKEY_CURRENT_USER\\Control Panel\\Keyboard";
constexpr auto SnippingValue = "PrintScreenKeyForSnippingEnabled";
}

namespace PrintScreenKey {

bool isSnippingDisabled()
{
    QSettings settings(KeyboardKey, QSettings::NativeFormat);
    return settings.value(SnippingValue, 1).toInt() == 0;
}

bool disableSnipping()
{
    QSettings settings(KeyboardKey, QSettings::NativeFormat);
    settings.setValue(SnippingValue, 0);
    settings.sync();
    if (QSettings::AccessError == settings.status()) {
        return false;
    }
    return isSnippingDisabled();
}

} // namespace PrintScreenKey

#endif
