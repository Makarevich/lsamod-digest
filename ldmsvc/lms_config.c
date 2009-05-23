

#include "../shared/shared.h"
#include "lms_config.h"

static int next_arg(int *argc, const char ***argv){
    (++(*argv));
    return ((--(*argc)) > 0);
}

static int parse_int(int *pnum, const char *str){
    int value = 0;
    int i;

    for(i = 0; i < 6; i++){
        char c = str[i];

        if((c >= '0') && (c <= '9')){
            value = value * 10 + c - '0';
        }else if(c == 0){
            *pnum = value;
            return 1;
        }else{
            return 0;
        }
    }

    return 0;
}

static void err_expected(char* expectee){
    dout(va("Invalid arguments: %s expected", expectee));
}

int config_parse_args(config_t* conf, int argc, const char **argv){
    conf->port = CONF_DEFAULT_PORT;
    conf->maxconn = CONF_DEFAULT_MAXCONN;
    strncpy(conf->pipe, CONF_DEFAULT_PIPE, sizeof(conf->pipe));

    dout(va("Param cnt: %u\n", argc));

    while(next_arg(&argc, &argv)){
        if(!strcmp("-p", *argv)){
            if(!next_arg(&argc, &argv)){
                err_expected("port value");
                return 0;
            }

            if(!parse_int(&conf->port, *argv)){
                dout("Invalid port value");
                return 0;
            }
        }else if(!strcmp("-mc", *argv)){
            if(!next_arg(&argc, &argv)){
                err_expected("max connections");
                return 0;
            }

            if(!parse_int(&conf->maxconn, *argv)){
                dout("Invalid max connections value");
                return 0;
            }
        }else if(!strcmp("-pipe", *argv)){
            if(!next_arg(&argc, &argv)){
                err_expected("pipe name");
                return 0;
            }

            strncpy(conf->pipe, *argv, sizeof(conf->pipe));
        }else{
            dout(va("Invalid argument: %s", *argv));
        }
    }

    return 1;
}

