/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2021 Micron Technology, Inc.  All rights reserved.
 */


#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include <cjson/cJSON.h>
#include <bsd/string.h>

#include <hse_ikvdb/hse_gparams.h>
#include <hse_ikvdb/runtime_home.h>
#include <hse_ikvdb/param.h>
#include <hse_ikvdb/limits.h>
#include <hse_util/logging.h>
#include <hse_util/compiler.h>

struct hse_gparams hse_gparams;

bool
logging_destination_converter(const struct param_spec *ps, const cJSON *node, void *value)
{
    assert(ps);
    assert(node);
    assert(value);

    if (!cJSON_IsString(node))
        return false;

    const char *setting = cJSON_GetStringValue(node);

    if (strcmp(setting, "stdout") == 0) {
        *(enum log_destination *)value = LD_STDOUT;
    } else if (strcmp(setting, "stderr") == 0) {
        *(enum log_destination *)value = LD_STDERR;
    } else if (strcmp(setting, "file") == 0) {
        *(enum log_destination *)value = LD_FILE;
    } else if (strcmp(setting, "syslog") == 0) {
        *(enum log_destination *)value = LD_SYSLOG;
    } else {
        return false;
    }

    return true;
}

bool
logging_destination_validator(const struct param_spec *ps, const void *value)
{
    assert(ps);
    assert(value);

    const enum log_destination dest = *(enum log_destination *)value;

    return dest == LD_STDOUT || dest == LD_STDERR || dest == LD_FILE || dest == LD_SYSLOG;
}

void
logging_destination_default(const struct param_spec *ps, void *value)
{
    assert(ps);
    assert(value);

    enum log_destination *dest = (enum log_destination *)value;

    *dest = LD_SYSLOG;
}

void
socket_path_default(const struct param_spec *ps, void *value)
{
    assert(ps);
    assert(value);

    HSE_MAYBE_UNUSED int n;

    n = snprintf(value, sizeof(hse_gparams.gp_socket.path), "/tmp/hse-%d.sock", getpid());
    assert(n < sizeof(hse_gparams.gp_socket.path) && n > 0);
}

static const struct param_spec pspecs[] = {
	{
		.ps_name = "logging.enabled",
		.ps_description = "Whether logging is enabled",
		.ps_flags = 0,
		.ps_type = PARAM_TYPE_BOOL,
		.ps_offset = offsetof(struct hse_gparams, gp_logging.enabled),
		.ps_size = sizeof(bool),
		.ps_convert = param_default_converter,
		.ps_validate = param_default_validator,
		.ps_default_value = {
			.as_bool = true,
		},
	},
	{
		.ps_name = "logging.structured",
		.ps_description = "Whether logging is structured",
		.ps_flags = 0,
		.ps_type = PARAM_TYPE_BOOL,
		.ps_offset = offsetof(struct hse_gparams, gp_logging.structured),
		.ps_size = sizeof(bool),
		.ps_convert = param_default_converter,
		.ps_validate = param_default_validator,
		.ps_default_value = {
			.as_bool = false,
		},
	},
	{
		.ps_name = "logging.destination",
		.ps_description = "Where log messages should be written to",
		.ps_flags = PARAM_FLAG_DEFAULT_BUILDER,
		.ps_type = PARAM_TYPE_ENUM,
		.ps_offset = offsetof(struct hse_gparams, gp_logging.destination),
		.ps_size = sizeof(enum log_destination),
		.ps_convert = logging_destination_converter,
		.ps_validate = logging_destination_validator,
		.ps_default_value = {
			.as_builder = logging_destination_default,
		},
		.ps_bounds = {
			.as_enum = {
				.ps_values = {
					"stdout",
					"stderr",
					"file",
					"syslog",
				},
				.ps_num_values = LD_SYSLOG + 1,
			},
		},
	},
	{
		.ps_name = "logging.path",
		.ps_description = "Name of log file when destination == file",
		.ps_flags = PARAM_FLAG_NULLABLE,
		.ps_type = PARAM_TYPE_STRING,
		.ps_offset = offsetof(struct hse_gparams, gp_logging.path),
		.ps_convert = param_default_converter,
		.ps_validate = param_default_validator,
		.ps_default_value = {
			.as_string = "hse.log",
		},
		.ps_bounds = {
			.as_string = {
				.ps_max_len = sizeof(((struct hse_gparams *)0)->gp_logging.path),
			},
		},
	},
	{
		.ps_name = "logging.level",
		.ps_description = "Maximum log level which will be written",
		.ps_flags = 0,
		.ps_type = PARAM_TYPE_I32,
		.ps_offset = offsetof(struct hse_gparams, gp_logging.level),
		.ps_size = sizeof(log_priority_t),
		.ps_convert = param_default_converter,
		.ps_validate = param_default_validator,
		.ps_default_value = {
			.as_scalar = HSE_LOG_PRI_DEFAULT,
		},
		.ps_bounds = {
			.as_scalar = {
				.ps_min = HSE_EMERG_VAL,
				.ps_max = HSE_DEBUG_VAL,
			}
		}
	},
    {
        .ps_name = "logging.squelch_ns",
        .ps_description = "drop messages repeated within nsec window",
        .ps_flags = PARAM_FLAG_EXPERIMENTAL,
        .ps_type = PARAM_TYPE_U64,
        .ps_offset = offsetof(struct hse_gparams, gp_logging.squelch_ns),
        .ps_size = sizeof(((struct hse_gparams *) 0)->gp_logging.squelch_ns),
        .ps_convert = param_default_converter,
        .ps_validate = param_default_validator,
        .ps_default_value = {
            .as_uscalar = HSE_LOG_SQUELCH_NS_DEFAULT,
        },
        .ps_bounds = {
            .as_uscalar = {
                .ps_min = 0,
                .ps_max = UINT64_MAX,
            },
        },
    },
	    {
        .ps_name = "c0kvs_ccache_sz_max",
        .ps_description = "max size of c0kvs cheap cache (bytes)",
        .ps_flags = PARAM_FLAG_EXPERIMENTAL,
        .ps_type = PARAM_TYPE_U64,
        .ps_offset = offsetof(struct hse_gparams, gp_c0kvs_ccache_sz_max),
        .ps_size = sizeof(((struct hse_gparams *) 0)->gp_c0kvs_ccache_sz_max),
        .ps_convert = param_default_converter,
        .ps_validate = param_default_validator,
        .ps_default_value = {
            .as_uscalar = HSE_C0_CCACHE_SZ_DFLT,
        },
        .ps_bounds = {
            .as_uscalar = {
                .ps_min = 0,
                .ps_max = HSE_C0_CCACHE_SZ_MAX,
            },
        },
    },
    {
        .ps_name = "c0kvs_ccache_sz",
        .ps_description = "size of c0kvs cheap cache (bytes)",
        .ps_flags = PARAM_FLAG_EXPERIMENTAL,
        .ps_type = PARAM_TYPE_U64,
        .ps_offset = offsetof(struct hse_gparams, gp_c0kvs_ccache_sz),
        .ps_size = sizeof(((struct hse_gparams *) 0)->gp_c0kvs_ccache_sz),
        .ps_convert = param_default_converter,
        .ps_validate = param_default_validator,
        .ps_default_value = {
            .as_uscalar = HSE_C0_CCACHE_SZ_DFLT,
        },
        .ps_bounds = {
            .as_uscalar = {
                .ps_min = 0,
                .ps_max = HSE_C0_CCACHE_SZ_MAX,
            },
        },
    },
    {
        .ps_name = "c0kvs_cheap_sz",
        .ps_description = "set c0kvs cheap size (bytes)",
        .ps_flags = PARAM_FLAG_EXPERIMENTAL,
        .ps_type = PARAM_TYPE_U64,
        .ps_offset = offsetof(struct hse_gparams, gp_c0kvs_cheap_sz),
        .ps_size = sizeof(((struct hse_gparams *) 0)->gp_c0kvs_cheap_sz),
        .ps_convert = param_default_converter,
        .ps_validate = param_default_validator,
        .ps_default_value = {
            .as_uscalar = HSE_C0_CHEAP_SZ_DFLT,
        },
        .ps_bounds = {
            .as_uscalar = {
                .ps_min = HSE_C0_CHEAP_SZ_MIN,
                .ps_max = HSE_C0_CHEAP_SZ_MAX,
            },
        },
    },
    {
        .ps_name = "socket.enabled",
        .ps_description = "Enable the REST server",
        .ps_flags = 0,
        .ps_type = PARAM_TYPE_BOOL,
        .ps_offset = offsetof(struct hse_gparams, gp_socket.enabled),
        .ps_size = sizeof(((struct hse_gparams *) 0)->gp_socket.enabled),
        .ps_convert = param_default_converter,
        .ps_validate = param_default_validator,
        .ps_default_value = {
            .as_bool = true,
        }
    },
    {
        .ps_name = "socket.path",
        .ps_description = "UNIX socket path",
        .ps_flags = PARAM_FLAG_DEFAULT_BUILDER,
        .ps_type = PARAM_TYPE_STRING,
        .ps_offset = offsetof(struct hse_gparams, gp_socket.path),
        .ps_convert = param_default_converter,
        .ps_validate = param_default_validator,
        .ps_default_value = {
            .as_builder = socket_path_default,
        },
        .ps_bounds = {
            .as_string = {
                .ps_max_len = sizeof(((struct hse_gparams *)0)->gp_socket.path),
            },
        },
    },
};

const struct param_spec *
hse_gparams_pspecs_get(size_t *pspecs_sz)
{
    if (pspecs_sz)
        *pspecs_sz = NELEM(pspecs);
    return pspecs;
}

merr_t
hse_gparams_resolve(struct hse_gparams *const params, const char *const runtime_home)
{
    merr_t err;
    char   buf[PATH_MAX];

    assert(params);
    assert(runtime_home);

    err = runtime_home_socket_path_get(runtime_home, params, buf, sizeof(buf));
    if (err)
        return err;
    strlcpy(hse_gparams.gp_socket.path, buf, sizeof(hse_gparams.gp_socket.path));

    err = runtime_home_logging_path_get(runtime_home, params, buf, sizeof(buf));
    if (err)
        return err;
    strlcpy(hse_gparams.gp_logging.path, buf, sizeof(hse_gparams.gp_logging.path));

    return 0;
}

struct hse_gparams
hse_gparams_defaults()
{
    struct hse_gparams params;
    const union params p = { .as_hse_gp = &params };
    param_default_populate(pspecs, NELEM(pspecs), p);
    return params;
}
