#if defined(cl_khr_fp64)
    #  pragma OPENCL EXTENSION cl_khr_fp64: enable
    #elif defined(cl_amd_fp64)
    #  pragma OPENCL EXTENSION cl_amd_fp64: enable
    #else
    #  error double precision is not supported
    #endif
    kernel void add(
           ulong n,
           global const double *a,
           global const double *b,
           global double *c,
           global size_t *global_id
           )
    {
        size_t i = get_global_id(0);

         //get_local_size(0) * get_num_groups(0)
        //*global_id = get_global_id(1025);

        *global_id = get_local_size(0);
        // *global_id = get_num_groups(0);

        if (i < n) {
           c[i] = a[i] + b[i];
        }


    };

    kernel void readInfo(global size_t *info) {
        //*info =  get_global_id(10) ;
         *info =  124;


    }
