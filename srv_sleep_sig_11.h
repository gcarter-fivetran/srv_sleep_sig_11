//
// Created by gcarter on 12/9/23.
//

#ifndef SRV_SLEEP_SIG_11_H
#define SRV_SLEEP_SIG_11_H

#include <ctpublic.h>
#include <ospublic.h>

#define SERVER_NAME "srv_sleep_sig_11"

CS_RETCODE CS_PUBLIC
start_handler(SRV_SERVER *server);
CS_RETCODE CS_PUBLIC
connect_handler(SRV_PROC *sp);
CS_RETCODE CS_PUBLIC
lang_handler(SRV_PROC *sp);

#endif //SRV_SLEEP_SIG_11_H
