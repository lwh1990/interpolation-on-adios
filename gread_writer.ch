s = adios_selection_writeblock (rank);
adios_schedule_read (fp, s, "data", 0, 1, data);
adios_perform_reads (fp, 1);
adios_selection_delete (s);
