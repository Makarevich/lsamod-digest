
#ifndef __LMS_CONFIG_H__
#define __LMS_CONFIG_H__

#include "../shared/shpipe.h"

#define CONF_DEFAULT_PORT       3111
#define CONF_DEFAULT_PIPE       SHARED_PIPE_NAME
#define CONF_DEFAULT_MAXCONN    8

typedef struct {
    int     port;
    char    pipe[64];
    int     maxconn;
} config_t;


int config_parse_args(config_t* conf, int argc, const char **argv);

#endif // __LMS_CONFIG_H__

