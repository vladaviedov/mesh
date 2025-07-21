/**
 * @file ext/nrlcustom.h
 * @author Vladyslav Aviedov <vladaviedov at protonmail dot com>
 * @version ?
 * @date 2025
 * @license GPLv3.0
 * @brief Custom handlers for nanorl.
 */
#pragma once

#include <nanorl/escape.h>
#include <nanorl/nanorl.h>

void nrlcustom_register(nrl_config *config);

void nrlcustom_reset(void);
