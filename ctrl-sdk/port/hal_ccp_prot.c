#include "hal_ccp.h"
#include "hal_ccp_recv.h"

hal_ccp_status hal_ccp_prot_init(void)
{
    return HAL_CCP_TRUE;
}

hal_ccp_status hal_ccp_prot_send(uint8_t *data, size_t len)
{
    return HAL_CCP_TRUE;
}

void hal_ccp_recv_prot_init(hal_ccp_recv_handle_t *recv)
{
    switch(recv->id)
    {
        case 1:
        {

            break;
        }
    }
}

_Bool hal_ccp_recv_prot_check_errors(hal_ccp_recv_handle_t *recv)
{
    switch(recv->id)
    {
        case 1:
        {
            break;
        }
    }
    return 0;
}

inline void hal_ccp_recv_prot_lock(void)
{

}

inline void hal_ccp_recv_prot_unlock(void)
{

}
