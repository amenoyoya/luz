#pragma once

#include "../core.hpp"

extern "C" {
    /// zip archiver structure
    typedef struct {
        unsigned long  handler;
        unsigned short level; // compression level
    } zip_archiver_t;

    /// zip extractor structure
    typedef struct {
        unsigned long handler;
        size_t size; // total zip file size
    } unz_archiver_t;

    /// date time structure
    typedef struct {
        unsigned long sec, min, hour, day, month, year;
    } datetime_t;

    /// file information in zip structure
    typedef struct {
        unsigned long   version, // created version
                        needed_version, // needed version for uncompression
                        flag, // bit flag
                        compression_method, // compression method
                        dos_date, // modified time (DOS)
                        crc32, // crc32
                        compressed_size, // compressed size
                        uncompressed_size, // uncompressed size
                        filename_size, // file name length
                        extra_size, // extra fields size
                        comment_size, // comment length
                        disknum_start, // start position of disk num
                        internal_attr, // internal file attribution
                        external_attr; // external file attribution
        datetime_t      created_at; // created date time
    } unz_file_info_t;

    /// local file position in the zip file
    typedef struct {
        unsigned long   pos_in_zip,  // offset in zip file directory
                        num_of_file; // of file
    } unz_file_pos_t;

    /// open zip file (archiver)
    // @param mode:
    //         "w": create new file.
    //         "w+": embed zip data to the file
    //         "a": append data to the zip file
    //        * parent directories will be created automatically
    // @param compresslevel: 0 - 9
    __export zip_archiver_t *zip_open(const char *filename, const char *mode, unsigned short compresslevel);

    /// close zip archiver
    // @param comment: zip global comment can be appended if you want
    __export void zip_close(zip_archiver_t *self, const char *comment);

    /// append data as a file into zip file
    __export bool zip_append(zip_archiver_t *self, const char *data, size_t datasize, const char *dest_filename, const char *password, const char *comment);
    
    /// append file into zip file
    __export bool zip_append_file(zip_archiver_t *self, const char *src_filename, const char *dest_filename, const char *password, const char *comment);


    /// open zip file (extractor)
    __export unz_archiver_t *unz_open(const char *filename);

    /// close zip extractor
    __export void unz_close(unz_archiver_t *self);

    /// get global comment of the zip file
    __export const char *unz_comment(unz_archiver_t *self);
    
    /// locate first entry file in the zip file
    __export bool unz_locate_first(unz_archiver_t *self);

    /// locate next entry file in the zip file
    __export bool unz_locate_next(unz_archiver_t *self);

    /// locate specified name of file in the zip file
    __export bool unz_locate_name(unz_archiver_t *self, const char *name);

    /// get current file information in the zip data
    __export bool unz_info(unz_archiver_t *self, unz_file_info_t *dest, char *filename, size_t filename_size, char *comment, size_t comment_size);

    /// get current file content (uncompressed) in the zip file
    __export bool unz_content(unz_archiver_t *self, char *dest, size_t datasize, const char *password);

    /// get current file position in the zip data
    __export bool unz_pos(unz_archiver_t *self, unz_file_pos_t *dest);

    /// locate to the designated position
    __export bool unz_locate(unz_archiver_t *self, unz_file_pos_t *pos);

    /// get the zip file offset (if the data is embedded zip data, return larger than 0)
    __export size_t unz_offset(unz_archiver_t *self);


    /*** utility functions ***/

    /// remove embedded zip data from the file
    __export bool unz_rmdata(const char *filename);

    /// Compress the directory to zip
    __export bool zip_compress(const char *dir, const char *output, unsigned short level, const char *password, const char *mode, const char *root);

    /// uncompress the zip into directory
    __export bool unz_uncompress(const char *zip, const char *dir, const char *password);
}