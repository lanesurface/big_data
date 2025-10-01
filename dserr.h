
#ifndef DS_ERRNO_T_
#define DS_ERRNO_T_

#define FOR_EACH_ERR(_do) \
  _do(DS_ERR_SUCCESS,0) \
  _do(DS_ERR_NO_MEM,1) \
  _do(DS_ERR_INV_ARG,2) \
  _do(DS_ERR_MAX,3)
#define MK_ERR_STR_ARRAY(_errno,_id,...) [_id]=#_errno,
#define DEF_ERR_T(_errno,_id,...) _errno=_id,

static const char *_ds_errno_str_array[]={
  FOR_EACH_ERR(MK_ERR_STR_ARRAY)
};

enum hm_errno_T { 
  FOR_EACH_ERR(DEF_ERR_T)
};

static inline const char *
ds_err_str(enum hm_errno_T errno)
{
  static const char *errno_inv_str="[[inv. error]]";
  if (errno<0) {
    errno*=-1;
    if (errno>=DS_ERR_MAX) {
      return errno_inv_str;
    }
  }
  return _ds_errno_str_array[errno];
}

#endif