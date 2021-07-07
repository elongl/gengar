struct upload_file_cmd
{
    unsigned int local_path_len;
    char *local_path;
    unsigned long long file_size;
    char file_chunk[8192];
};