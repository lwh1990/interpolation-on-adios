adios_groupsize = 4 \
                + 4 \
                + 4 \
                + 4 \
                + 4 \
                + 4 \
                + 4 * (nx_local) * ( ny_local);
adios_group_size (adios_handle, adios_groupsize, &adios_totalsize);
adios_write (adios_handle, "nx_global", &nx_global);
adios_write (adios_handle, "ny_global", &ny_global);
adios_write (adios_handle, "offs_x", &offs_x);
adios_write (adios_handle, "offs_y", &offs_y);
adios_write (adios_handle, "nx_local", &nx_local);
adios_write (adios_handle, "ny_local", &ny_local);
adios_write (adios_handle, "data", data);
