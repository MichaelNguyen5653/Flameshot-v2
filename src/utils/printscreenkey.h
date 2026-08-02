// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if defined(Q_OS_WIN) || defined(_WIN32)

/**
 * @brief Windows' own claim on the Print Screen key.
 *
 * Windows opens its built-in snipping tool on Print Screen unless the user
 * turns that off, which stops Flameshot from ever seeing the key. Both the
 * first-run welcome prompt and the shortcuts settings page offer to turn it
 * off, so the registry access lives here rather than in either of them.
 */
namespace PrintScreenKey {

// True when Windows is *not* claiming the key, i.e. Flameshot can use it
bool isSnippingDisabled();

// Ask Windows to stop claiming the key. Returns false if the registry
// could not be written. Takes effect after a restart.
bool disableSnipping();

} // namespace PrintScreenKey

#endif
