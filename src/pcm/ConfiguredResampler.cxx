/*
 * Copyright (C) 2003-2013 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"
#include "ConfiguredResampler.hxx"
#include "FallbackResampler.hxx"
#include "ConfigGlobal.hxx"
#include "ConfigOption.hxx"
#include "ConfigError.hxx"
#include "util/Error.hxx"

#ifdef HAVE_LIBSAMPLERATE
#include "LibsamplerateResampler.hxx"
#endif

#include <string.h>

enum class SelectedResampler {
	FALLBACK,

#ifdef HAVE_LIBSAMPLERATE
	LIBSAMPLERATE,
#endif
};

static SelectedResampler selected_resampler = SelectedResampler::FALLBACK;

bool
pcm_resampler_global_init(Error &error)
{
	const char *converter =
		config_get_string(CONF_SAMPLERATE_CONVERTER, "");

	if (strcmp(converter, "internal") == 0)
		return true;

#ifdef HAVE_LIBSAMPLERATE
	selected_resampler = SelectedResampler::LIBSAMPLERATE;
	return pcm_resample_lsr_global_init(converter, error);
#endif

	if (*converter == 0)
		return true;

	error.Format(config_domain,
		     "The samplerate_converter '%s' is not available",
		     converter);
	return false;
}

PcmResampler *
pcm_resampler_create()
{
	switch (selected_resampler) {
	case SelectedResampler::FALLBACK:
		return new FallbackPcmResampler();

#ifdef HAVE_LIBSAMPLERATE
	case SelectedResampler::LIBSAMPLERATE:
		return new LibsampleratePcmResampler();
#endif
	}

	gcc_unreachable();
}