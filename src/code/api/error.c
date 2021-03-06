#include "api.h"

int api_errno(int res){
    switch(res){
        case RES_SUCCESS:{
            return 0;
        }
        case RES_CLOSE:{
            return 0;
        }
        case RES_DATA:{
            return 0;
        }
        case RES_ERROR:{
            log_error("[API_ERROR] An error as occurred. Try again.");
            return EAGAIN;
        }
        case RES_ERROR_DATA:{
            log_error("[API_ERROR] An error as occurred. Try again.");
            return EAGAIN;
        }
        case RES_EXIST:{
            log_error("[API_ERROR] File already exists");
            return EEXIST;
        }
        case RES_NOT_EXIST:{
            log_error("[API_ERROR] File not exists");
            return EFAULT;
        }
        case RES_NOT_OPEN:{
            log_error("[API_ERROR] File isn't open. Open first.");
            return EFAULT;
        }
        case RES_IS_LOCKED:{
            log_error("[API_ERROR] File is already locked");
            return EBUSY;
        }
        case RES_NOT_LOCKED:{
            log_error("[API_ERROR] File is not locked");
            return EPERM;
        }
        case RES_TOO_BIG:{
            log_error("[API_ERROR] File is too big for upload");
            return EFBIG;
        }
        case RES_DELETE:{
            log_error("[API_ERROR] Someone deleted the file while you were in the queue.");
            return EFAULT;
        }
        case RES_NULL:{
            log_error("[API_ERROR] An error as occurred to communicate. Try again.");
            return ECOMM;
        }
        case RES_NOT_EMPTY:{
            log_error("[API_ERROR] The file has already been written");
            return EAGAIN;
        }
        case RES_EMPTY:{
            log_error("[API_ERROR] Canno't append file with zero space.");
            return EAGAIN;
        }
        case RES_SERVER_EMPTY:{
            log_error("[API_ERROR] The server is empty");
            return RES_SERVER_EMPTY;
        }
        case RES_NOT_YOU_LOCKED:{
            log_error("[API_ERROR] File is locked by another client.");
            return EAGAIN;
        }
        case RES_YOU_LOCKED:{
            log_error("[API_ERROR] File is already locked by you.");
            return EAGAIN;
        }
        case RES_ALREADY_OPEN:{
            log_error("[API_ERROR] File is already open by you.");
            return EAGAIN;
        }
        default:{
            return ENOTRECOVERABLE;
        }
            
    }
}