#include "api.h"

int api_errno(int res){
    switch(res){
        case RES_SUCCESS:{
            return 0;
        }
        case RES_CLOSE:{
            return 0;
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
        case RES_NOT_LOCKED:{ //Maybe not use
            log_error("[API_ERROR] File is not locked");
            return EPERM;
        }
        case RES_TOO_BIG:{
            log_error("[API_ERROR] File is too big for upload");
            return EFBIG;
        }
        case RES_ERROR:{
            log_error("[API_ERROR] An error as occurred. Try again.");
            return EAGAIN;
        }
        case RES_NULL:{
            log_error("[API_ERROR] An error as occurred to communicate. Try again.");
            return ECOMM;
        }
        default:{
            return ENOTRECOVERABLE;
        }
            
    }
}