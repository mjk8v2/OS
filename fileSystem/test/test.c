#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../src/FonefiveFS.c"

#define DESCRIPTOR_TESTS 0
// set to 1 to enable descriptor tests

#define assert(e) ((e) ? (true) : \
                   (fprintf(stderr,"%s,%d: assertion '%s' failed\n",__FILE__, __LINE__, #e), \
                    fflush(stderr), fflush(stdout), abort()))

/*

    int fs_format(const char *const fname);
    1   Normal
    2   NULL
    3   Empty string

    F15FS_t *fs_mount(const char *const fname);
    1   Normal
    2   NULL
    3   Empty string

    int fs_unmount(F15FS_t *fs);
    1   Normal
    2   NULL

    int fs_create_file(F15FS_t *const fs, const char *const fname, const ftype_t ftype);
    1. Normal, file, in root
    2. Normal, directory, in root
    3. Normal, file, not in root
    4. Normal, directory, not in root
    5. Error, NULL fs
    6. Error, NULL fname
    7. Error, empty fname
    8. Error, bad type
    9. Error, path does not exist
    10. Error, Root clobber
    11. Error, directory exists
    12. Error, file exists
    13. Error, part of path not directory
    14. Error, path terminal not directory
    15. Error, path string has no leading slash
    16. Error, path has trailing slash (no name for desired file)
    17. Error, bad path, path part too long (will fail right now?)
    18. Error, bad path, desired filename too long
    19. Error, directory full.
    20. Error, out of inodes.
    21. Error, out of data blocks & file is directory (requires functional write)

    int fs_get_dir(const F15FS_t *const fs, const char *const fname, dir_rec_t *const records)
    1. Normal, root I guess?
    2. Normal, subdir somewhere
    3. Normal, empty dir
    4. Error, empty fname
    5. Error, NULL fname
    6. Error, NULL fs
    7. Error, NULL records
    8. Error, not a directory

    int fs_open_file(F15FS_t *const fs, const char * const fname)
    1. Normal, file at root
    2. Normal, file in subdir
    3. Normal, multiple fd to the same file
    4. Error, NULL fs
    5. Error, NULL fname
    6. Error, empty fname
    7. Error, not a regular file
    8. Error, file does not exist
    9. Error, out of descriptors

    ssize_t fs_write_file(F15FS_t *const fs, const char *const fname, const void *data, size_t nbyte, size_t offset)
    1. Normal, in a subdir, 0 size to < 1 block
    2. Normal, in a subdir, < 1 block to next (also, test offset below size of file)
    3. Normal, in a subdir, 0 size to 1 block
    4. Normal, in a subdir, 1 block to next
    5. Normal, in a subdir, 1 block to partial
    6. Normal, in a subdir, direct -> indirect
    7. Normal, in a subdir, indirect -> dbl_indirect
    8. Normal, in a subdir, full file (run out of blocks before max file size :/ )
    9. Error, file full/blocks full (also test fs_create_file 13)
    10. Error, file does not exist
    11. Error, file is a directory
    12. Error, nbyte + offset rollover
    13. Error, fs NULL
    14. Error, fname NULL
    15. Error, fname empty
    16. Error, data NULL
    17. Error, nbyte 0 (not an error...? Bad parameters? Hmm.)
    18. Error, offset past end of file

    ssize_t fs_write_file(F15FS_t *const fs, const int fd, const void *data, size_t nbyte);
    1. Normal, in a subdir, 0 size to < 1 block
    2. Normal, in a subdir, < 1 block to next
    3. Normal, in a subdir, 0 size to 1 block
    4. Normal, in a subdir, 1 block to next
    5. Normal, in a subdir, 1 block to partial
    6. Normal, in a subdir, direct -> indirect
    7. Normal, in a subdir, indirect -> dbl_indirect
    8. Normal, in a subdir, full file (run out of blocks before max file size :/ )
    9. Error, file full/blocks full (also test fs_create_file 13)
    10. Error, nbyte + position rollover
    11. Error, fs NULL
    12. Error, data NULL
    13. Error, nbyte 0 (not an error...? Bad parameters? Hmm.)
    14. Error, bad fd

    ssize_t fs_read_file(F15FS_t *const fs, const char *const fname, void *data, size_t nbyte, size_t offset);
    1. Normal, subdir, begin to < 1 block
    2. Normal, subdir, < 1 block to part of next
    3. Normal, subdir, whole block
    4. Normal, subdir, multiple blocks
    5. Normal, subdir, direct->indirect transition
    6. Normal, subdir, indirect->dbl_indirect transition
    7. Error, file does not exist
    8. Error, file not a regular file
    9. Error, NULL fs
    10. Error, NULL fname
    11. Error, empty fname
    12. Error, NULL data
    13. Error, nbyte 0 (not an error?)
    14. Error, offset well past EOF
    15. Error, offset AT EOF

    ssize_t fs_read_file(F15FS_t *const fs, const int fd, void *data, size_t nbyte);
    1. Normal, subdir, begin to < 1 block
    2. Normal, subdir, < 1 block to part of next
    3. Normal, subdir, whole block
    4. Normal, subdir, multiple blocks
    5. Normal, subdir, direct->indirect transition
    6. Normal, subdir, indirect->dbl_indirect transition
    7. Error, NULL fs
    8. Error, NULL data
    9. Error, nbyte 0 (not an error?)
    10. Error, at EOF (not an error?)

    int fs_remove_file(F15FS_t *const fs, const char *const fname);
    1. Normal, file at root
    2. Normal, file in subdirectory (attept to write to already opened descriptor to this afterwards - DESCRIPTOR ONLY)
    3. Normal, in subdir, empty directory
    4. Normal, file in double indirects somewhere (use full fs file from write_file?)
    5. Error, directory with contents
    6. Error, file does not exist
    7. Error, Root (also empty)
    8. Error, NULL fs
    9. Error, NULL fname
    10. Error, Empty fname

    int fs_move_file(F15FS_t *const fs, const char *const fname_src, const char *const fname_dst);
    1. Normal, file, root to another place in root
    2. Normal, file, one dir to another (attempt to use already opened descriptor after move - DESCRIPTOR ONLY)
    3. Normal, directory
    4. Normal, Rename of file where the directory is full
    5. Error, dst exists
    6. Error, dst parent does not exist
    7. Error, dst parent full
    8. Error, src does not exist
    9. ?????, src = dst
    10. Error, FS null
    11. Error, src null
    12. Error, src empty
    13. Error, src is root
    14. Error, dst NULL
    15. Error, dst empty
    16. Error, dst root?
    Evil one I won't test: Move a directory into itself

    int fs_seek_file(F15FS_t *const fs, const int fd, const off_t offset, const seek_t whence)
    1. Normal, wherever, really - make sure it doesn't change a second fd to the file
    2. ?????, seek past beginning - resulting location unspecified by our api, can't really test?
    3. ?????, seek past end - resulting location unspecified by our api, can't really test?
    4. Error, FS null
    5. Error, fd invalid
    6. Error, whence not a valid value

    int fs_close_file(F15FS_t *const fs, const int fd)
    1. Normal, whatever, attempt to use after closing, assert failure
    2. Normal, multiple opens, close does not affect the others
    3. Error, FS null
    4. Error, invalid fd

*/

// SIZES AND CREATION/MOUNT/UNMOUNT
void tests_a();

// CREATE FILE
void tests_b();

// FS_GET_DIR
void tests_c();

#if DESCRIPTOR_TESTS

    // FS_OPEN_FILE
    void tests_d();

#endif

// FS_WRITE_FILE
void tests_e();

// FS_READ_FILE
void tests_f();

// FS_REMOVE_FILE
void tests_g();

// FS_MOVE_FILE
void tests_h();

#if DESCRIPTOR_TESTS

    // FS_SEEK_FILE
    void tests_i();

    // FS_CLOSE_FILE
    void tests_j();
#endif


int main() {

    puts("Running autotests, sit back and relax, it'll be awhile...\n");

    tests_a();

    puts("\n\nA tests passed...\n");

    tests_b();

    puts("\n\nB tests passed...\n");

    tests_c();

    puts("\n\nC tests passed...\n");

    #if DESCRIPTOR_TESTS
    tests_d();

    puts("\n\nD tests passed...\n");
    #endif

    tests_e();

    puts("\n\nE tests passed...\n");

    tests_f();

    puts("\n\nF tests passed...\n");

    /* This is where the bug, in this test */ 
	tests_g();

    puts("\n\nG tests passed...\n");

    /* Bug still in fs_move_file(...) */
    //tests_h();

    //puts("\n\nH tests passed...\n");

    #if DESCRIPTOR_TESTS
    tests_i();

    puts("\n\nI tests passed...\n");

    tests_j();

    puts("\n\nJ tests passed...\n");
    #endif

    puts("TESTS COMPLETE");

}

void tests_a() {

    char *test_fname = "a_tests.f15fs";

    /*
        assert(sizeof(mdata_t) == 48);
        assert(sizeof(inode_t) == 128);
        assert(sizeof(data_block_t) == 1024);
        assert(sizeof(dir_ent_t) == 49);
        assert(sizeof(dir_mdata_t) == 44);
        assert(sizeof(dir_block_t) == 1024);
    */

    // FORMAT 1
    assert(fs_format(test_fname) == 0);
    // FORMAT 2
    assert(fs_format(NULL) < 0);
    // FORMAT 3
    assert(fs_format("") < 0);

    // MOUNT 1
    F15FS_t *fs = fs_mount(test_fname);
    assert(fs);
    // MOUNT 2
    assert(fs_mount(NULL) == NULL);
    // MOUNT 3
    assert(fs_mount("") == NULL);

    // UNMOUNT 1
    assert(fs_unmount(fs) == 0);
    // UNMOUNT 2
    assert(fs_unmount(NULL) < 0);

}

// CREATE_FILE
void tests_b() {
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };

    const char *(test_fname[2]) = {"b_tests_normal.f15fs", "b_tests_full_table.f15fs"};

    assert(fs_format(test_fname[0]) == 0);

    F15FS_t *fs = fs_mount(test_fname[0]);

    assert(fs);

    // CREATE_FILE 1
    assert(fs_create_file(fs, filenames[0], REGULAR) == 0);

    // CREATE_FILE 2
    assert(fs_create_file(fs, filenames[1], DIRECTORY) == 0);

    // CREATE_FILE 3
    assert(fs_create_file(fs, filenames[2], REGULAR) == 0);

    // CREATE_FILE 4
    assert(fs_create_file(fs, filenames[3], DIRECTORY) == 0);

    // CREATE_FILE 5
    assert(fs_create_file(NULL, filenames[4], REGULAR) < 0);

    // CREATE_FILE 6
    assert(fs_create_file(fs, NULL, REGULAR) < 0);

    // CREATE_FILE 7
    assert(fs_create_file(fs, "", REGULAR) < 0);

    // CREATE_FILE 8
    assert(fs_create_file(fs, "", 44) < 0);

    // CREATE_FILE 9
    assert(fs_create_file(fs, filenames[6], REGULAR) < 0);

    // CREATE_FILE 10
    assert(fs_create_file(fs, filenames[12], DIRECTORY) < 0);

    // CREATE_FILE 11
    assert(fs_create_file(fs, filenames[1], DIRECTORY) < 0);
    assert(fs_create_file(fs, filenames[1], REGULAR) < 0);

    // CREATE_FILE 12
    assert(fs_create_file(fs, filenames[0], REGULAR) < 0);
    assert(fs_create_file(fs, filenames[0], DIRECTORY) < 0);

    // CREATE_FILE 13
    assert(fs_create_file(fs, filenames[5], REGULAR) < 0);

    // CREATE_FILE 14
    assert(fs_create_file(fs, filenames[7], REGULAR) < 0);

    // CREATE_FILE 15
    //assert(fs_create_file(fs, filenames[8], REGULAR) < 0);
    // Hmm, because of the way strtok works, this fails.
    // But if we don't support relative paths, is there a reason to force abolute notation?
    // It's really a semi-arbitrary restriction
    // I suppose relative paths are up to the implementation, since . and .. are just special folder entires
    // but that would mess with the directory content total, BUT extra parsing can work around that.
    // Hmmmm.

    // CREATE_FILE 16
    assert(fs_create_file(fs, filenames[9], DIRECTORY) < 0);

    // CREATE_FILE 17
    assert(fs_create_file(fs, filenames[10], REGULAR) < 0);

    // CREATE_FILE 18
    assert(fs_create_file(fs, filenames[11], REGULAR) < 0);

    // CREATE_FILE 19 - OUT OF INODES (and test 18 along the way)
    // Gotta make... Uhh... A bunch of files. (255, but we'll need directories to hold them as well)
    // Closing this file now for inspection to make sure these tests didn't mess it up
    // There should be... 3 allocated data blocks and 5 inodes in use

    fs_unmount(fs);

    assert(fs_format(test_fname[1]) == 0);

    fs = fs_mount(test_fname[1]);

    assert(fs);

    {
        //puts("Attempting to fill inode table...");

        // Dummy string to loop with
        char fname[] = "/a/a";
        // If we do basic a-z, with a-z contained in each, that's... 26*20 which is ~2x as much as we need
        // Gotta do the math on when this should fail to set the right bounds

        // 12 dirs of 20, remainder is... 3 inodes (dir and 2 files)
        for (char dir = 'a'; dir < 'm'; fname[1] = ++dir) {
            fname[2] = '\0';
            assert(fs_create_file(fs, fname, DIRECTORY) == 0);
            //printf("File: %s\n",fname);
            fname[2] = '/';
            for (char file = 'a'; file < 'u'; fname[3] = ++file) {
                //printf("File: %s\n",fname);
                assert(fs_create_file(fs, fname, REGULAR) == 0);
            }
        }

        // CREATE_FILE 19
        assert(fs_create_file(fs, "/a/z", REGULAR) < 0);

        // Catch up to finish creation
        fname[1] = 'm';
        fname[2] = '\0';
        //printf("File: %s\n",fname);
        assert(fs_create_file(fs, fname, DIRECTORY) == 0);

        fname[2] = '/';
        fname[3] = 'a';
        //printf("File: %s\n",fname);
        assert(fs_create_file(fs, fname, REGULAR) == 0);

        fname[3] = 'b';
        //printf("File: %s\n",fname);
        assert(fs_create_file(fs, fname, REGULAR) == 0);
        //puts("Inode table full?");

        // This should fail, no more inodes.
        // CREATE_FILE 20
        fname[3] = 'c';
        assert(fs_create_file(fs, fname, REGULAR) < 0);

    }
    // save file for inspection
    fs_unmount(fs);

    // ... Can't really test 20 yet.
}

void print_dir_records(const char *const fname, const dir_rec_t *const records) {
    if (fname && records) {
        printf("\n*** DIRECTORY RECORD ***\n");
        printf("* DIRECTORY: %s\n", fname);
        printf("* ENTRIES: %d\n", records->total);
        for (unsigned i = 0; i < records->total; ++i) {
            printf("* %d:\t%s - %s\n", i, records->contents[i].fname,
                   ((records->contents[i].ftype == REGULAR) ? "REGULAR" :
                    ((records->contents[i].ftype == DIRECTORY) ? "DIRECTORY" :
                     "UNKNOWN")));
        }
        printf("****** RECORD END ******\n\n");
        return;
    }
    assert(false /* tester broken? */);
}

bool check_record_for_file(const char *const fname, const ftype_t ftype, const dir_rec_t *const records) {
    if (fname && records) {
        for (unsigned i = 0; i < records->total; ++i) {
            // Technically this may be risky since strncmp
            // Doesn't say when it will stop
            // and fname probably won't be FNAME_MAX in length
            // But at least my fname will be properly terminated
            // So it should cause strncmp to stop first
            if (strncmp(records->contents[i].fname, fname, FNAME_MAX) == 0) {
                return (records->contents[i].ftype == ftype);
            }
        }
        return false;
    } else {
        assert(false /* tester broken? */);
    }
}

// FS_GET_DIR
void tests_c() {

    // We'll ref the files from the b tests, but copy them over first
    // Just in case it changes anything
    assert(system("cp b_tests_normal.f15fs c_tests_normal.f15fs") == 0);
    const char *test_fname = "c_tests_normal.f15fs";
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };

    F15FS_t *fs = fs_mount(test_fname);
    // This should have root, folder, file, folder/with_file, folder/with_folder

    dir_rec_t record_struct;
    // FS_GET_DIR 1
    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(record_struct.total == 2);
    assert(check_record_for_file(strrchr(filenames[0], '/') + 1, REGULAR, &record_struct));
    assert(check_record_for_file(strrchr(filenames[1], '/') + 1, DIRECTORY, &record_struct));

    // FS_GET_DIR 2
    assert(fs_get_dir(fs, filenames[1], &record_struct) == 0);
    print_dir_records(filenames[1], &record_struct);
    assert(record_struct.total == 2);
    assert(check_record_for_file(strrchr(filenames[2], '/') + 1, REGULAR, &record_struct));
    assert(check_record_for_file(strrchr(filenames[3], '/') + 1, DIRECTORY, &record_struct));

    // FS_GET_DIR 3
    assert(fs_get_dir(fs, filenames[3], &record_struct) == 0);
    print_dir_records(filenames[3], &record_struct);
    assert(record_struct.total == 0);

    // FS_GET_DIR 4
    assert(fs_get_dir(fs, "", &record_struct) < 0);

    // FS_GET_DIR 5
    assert(fs_get_dir(fs, NULL, &record_struct) < 0);

    // FS_GET_DIR 6
    assert(fs_get_dir(NULL, filenames[12], &record_struct) < 0);

    // FS_GET_DIR 7
    assert(fs_get_dir(fs, filenames[12], NULL) < 0);

    // FS_GET_DIR 8
    assert(fs_get_dir(fs, filenames[0], &record_struct) < 0);

    assert(fs_unmount(fs) == 0);
}

#if DESCRIPTOR_TESTS

// FS_OPEN_FILE
void tests_d() {
    // Once again, just borrow B's file.
    assert(system("cp b_tests_normal.f15fs d_tests.f15fs") == 0);

    const char *test_fname = "d_tests.f15fs";
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };

    F15FS_t *fs = fs_mount(test_fname);
    assert(fs);

    // FS_OPEN_FILE 1
    assert(fs_open_file(fs, filenames[0]) >= 0);

    // FS_OPEN_FILE 2
    assert(fs_open_file(fs, filenames[2]) >= 0);

    // FS_OPEN_FILE 3
    assert(fs_open_file(fs, filenames[2]) >= 0);

    // FS_OPEN_FILE 4
    assert(fs_open_file(NULL, filenames[2]) < 0);

    // FS_OPEN_FILE 5
    assert(fs_open_file(fs, NULL) < 0);

    // FS_OPEN_FILE 6
    assert(fs_open_file(fs, "") < 0);

    // FS_OPEN_FILE 7
    assert(fs_open_file(fs, filenames[1]) < 0);

    // FS_OPEN_FILE 8
    assert(fs_open_file(fs, filenames[4]) < 0);

    // FS_OPEN_FILE 9
    // we have... 3 open so far
    for (unsigned i = 3; i < 256; ++i) {
        assert(fs_open_file(fs, filenames[2]) >= 0);
    }
    // So this should fail
    assert(fs_open_file(fs, filenames[2]) < 0);

    assert(fs_unmount(fs) == 0);

}

// FS_WRITE_FILE - DESCRIPTOR
void tests_e() {
    // Terrible tests for a terrible function

    // Once again, just borrow B's file.
    assert(system("cp b_tests_normal.f15fs e_tests_normal.f15fs") == 0);

    const char *(test_fname[2]) = {"e_tests_normal.f15fs", "e_tests_full.f15fs"};
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };

    F15FS_t *fs = fs_mount(test_fname[0]);
    assert(fs);

    uint8_t file_data[4096];
    memset(file_data, 1, 4096);


    int fd;
    fd = fs_open_file(fs, filenames[2]);
    assert(fd >= 0);

    // FS_WRITE_FILE 1
    assert(fs_write_file(fs, fd, file_data, 300) == 300);

    // FS_WRITE_FILE 2
    assert(fs_write_file(fs, fd, file_data, 824) == 824); // file[2] goes to 1024


    fd = fs_open_file(fs, filenames[0]);
    assert(fd >= 0);

    // FS_WRITE_FILE 3
    assert(fs_write_file(fs, fd, file_data, 1024) == 1024); // file[0] goes to 1024

    // FS_WRITE_FILE 4
    assert(fs_write_file(fs, fd, file_data, 1024) == 1024); // file[0] goes to 2048 now

    // FS_WRITE_FILE 5
    assert(fs_write_file(fs, fd, file_data, 300) == 300); // file[2] goes to 2348 now

    // FS_WRITE_FILE 6
    assert(fs_write_file(fs, fd, file_data, 4095) == 4095); // file[0] goes to 6143 now, 1 before end of directs
    assert(fs_write_file(fs, fd, file_data, 1025) == 1025); // file[0] goes to 7168, 1 block into indirect

    // FS_WRITE_FILE 7
    // Need to fill out... 256 more blocks to put us 1 into double indirect
    for (int i = 0; i < 256; ++i) {
        assert(fs_write_file(fs, fd, file_data, 1024) == 1024);
    }
    // File's into double indirect

    fs_unmount(fs);

    assert(fs_format(test_fname[1]) == 0);
    fs = fs_mount(test_fname[1]);
    assert(fs);

    assert(fs_create_file(fs, filenames[0], REGULAR) == 0);

    fd = fs_open_file(fs, filenames[0]);
    assert(fd >= 0);

    // Going to fill the fs with a single file
    for (unsigned i = 0; i < 65239; ++i) {
        assert(fs_write_file(fs, fd, file_data, 1024) == 1024);
    }
    // File system is now FULL.
    // Throw in that test for FS_CREATE_FILE

    // FS_CREATE_FILE 13
    assert(fs_create_file(fs, filenames[1], DIRECTORY) < 0);

    // FS_WRITE_FILE 9
    assert(fs_write_file(fs, fd, file_data, 1024) < 0); // out of room should be an error, not a 0 written

    assert(fs_unmount(fs) == 0);

    fs = fs_mount(test_fname[0]);
    assert(fs);

    fd = fs_open_file(fs, filenames[2]);
    assert(fd >= 0);

    // FS_WRITE_FILE 10
    assert(fs_write_file(fs, fd, file_data, 1024) == 1024);
    assert(fs_write_file(fs, fd, file_data, SIZE_MAX - 512) < 0);

    // FS_WRITE_FILE 11
    fd = fs_open_file(fs, filenames[2]);
    assert(fd >= 0);
    assert(fs_write_file(NULL, fd, file_data, 300) < 0);

    // FS_WRITE_FILE 12
    assert(fs_write_file(fs, fd, NULL, 300) < 0);

    // FS_WRITE_FILE 13
    assert(fs_write_file(fs, fd, file_data, 0) <= 0); // Allow an error or 0 written

    // FS_WRITE_FILE 14
    assert(fs_write_file(fs, 9999, file_data, 0) < 0);

    assert(fs_unmount(fs) == 0);

}

// FS_READ_FILE - DESCRIPTOR
void tests_f() {

    assert(system("cp e_tests_normal.f15fs f_tests_normal.f15fs") == 0);

    const char *test_fname = "f_tests_normal.f15fs";
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };

    // file 0 should go into double_indirect, file 2 goes to 1024

    F15FS_t *fs = fs_mount(test_fname);
    assert(fs);

    uint8_t read_space[4096];
    uint8_t check_val[4096];
    memset(check_val, 1, 4096); // Didn't actually write any other values


    int fd[2];
    fd[0] = fs_open_file(fs, filenames[0]);
    fd[1] = fs_open_file(fs, filenames[2]);
    assert(fd[0] >= 0);
    assert(fd[1] >= 0);

    // FS_READ_FILE 1
    assert(fs_read_file(fs, fd[1], read_space, 300) == 300);
    assert(memcmp(read_space, check_val, 300) == 0);

    // FS_READ_FILE 2
    assert(fs_read_file(fs, fd[0], read_space, 512) == 512);
    assert(memcmp(read_space, check_val, 512) == 0);
    assert(fs_read_file(fs, fd[0], read_space, 1024) == 1024);
    assert(memcmp(read_space, check_val, 1024) == 0);

    assert(fs_read_file(fs, fd[0], read_space, 512) == 512); // jump to end of block for other tests

    // FS_READ_FILE 3
    assert(fs_read_file(fs, fd[0], read_space, 1024) == 1024);
    assert(memcmp(read_space, check_val, 1024) == 0);

    // FS_READ_FILE 4
    assert(fs_read_file(fs, fd[0], read_space, 2048) == 2048);
    assert(memcmp(read_space, check_val, 2048) == 0);

    // FS_READ_FILE 5
    assert(fs_read_file(fs, fd[0], read_space, 2048) == 2048);
    assert(memcmp(read_space, check_val, 2048) == 0);

    // we have read 7 blocks. we have to jump ahead... 254 and then, eh, 1.5
    for (unsigned i = 0; i < 254; ++i) {
        assert(fs_read_file(fs, fd[0], read_space, 1024) == 1024);
        assert(memcmp(read_space, check_val, 1024) == 0);
    }

    // FS_READ_FILE 6
    assert(fs_read_file(fs, fd[0], read_space, 1536) == 1536);
    assert(memcmp(read_space, check_val, 1536) == 0);

    // FS_READ_FILE 7
    assert(fs_read_file(NULL, fd[0], read_space, 10) < 0);

    // FS_READ_FILE 8
    assert(fs_read_file(fs, fd[0], NULL, 10) < 0);

    // FS_READ_FILE 9
    assert(fs_read_file(fs, fd[0], read_space, 0) <= 0);

    // FS_READ_FILE 10
    // I've actually sortof lost track, but we should be within 4 blocks of then end (should be 512B left?)
    // Request 4 blocks, that should get us to EOF, then request more.
    assert(fs_read_file(fs, fd[0], read_space, 4096) > 0);
    // This should fail, or return 0
    assert(fs_read_file(fs, fd[0], read_space, 4096) <= 0);

    assert(fs_unmount(fs) == 0);

}

#else


// FS_WRITE_FILE
void tests_e() {
    // Terrible tests for a terrible function


    // Once again, just borrow B's file.
    assert(system("cp b_tests_normal.f15fs e_tests_normal.f15fs") == 0);

    const char *(test_fname[2]) = {"e_tests_normal.f15fs", "e_tests_full.f15fs"};
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };

    F15FS_t *fs = fs_mount(test_fname[0]);
    assert(fs);

    uint8_t file_data[4096];
    memset(file_data, 1, 4096);


    // FS_WRITE_FILE 1
    assert(fs_write_file(fs, filenames[2], file_data, 300, 0) == 300);

    // FS_WRITE_FILE 2
    assert(fs_write_file(fs, filenames[2], file_data, 824, 200) == 824); // file[2] goes to 1024

    // FS_WRITE_FILE 3
    assert(fs_write_file(fs, filenames[0], file_data, 1024, 0) == 1024); // file[0] goes to 1024

    // FS_WRITE_FILE 4
    assert(fs_write_file(fs, filenames[0], file_data, 1024, 1024) == 1024); // file[0] goes to 2048 now

    // FS_WRITE_FILE 5
    assert(fs_write_file(fs, filenames[0], file_data, 300, 2048) == 300); // file[0] goes to 2348 now

    // FS_WRITE_FILE 6
    assert(fs_write_file(fs, filenames[0], file_data, 3795, 2348) == 3795); // file[0] goes to 6143 now, 1 before end of directs
    assert(fs_write_file(fs, filenames[0], file_data, 1025, 6143) == 1025); // file[0] goes to 7168, 1 block into indirect

    // FS_WRITE_FILE 7
    for (int i = 0; i < 256; ++i) {
        assert(fs_write_file(fs, filenames[0], file_data, 1024, 7168 + i * 1024) == 1024);
    }
    // File's one block into double indirect

    fs_unmount(fs);

    assert(fs_format(test_fname[1]) == 0);
    fs = fs_mount(test_fname[1]);
    assert(fs);

    assert(fs_create_file(fs, filenames[0], REGULAR) == 0);

    // Going to fill the fs with a single file
    // That's 65239 blocks worth of file data
    // A file can hold 65798 blocks, but we have less in the block store
    // AND indirects and double indirects take up space
    // so it comes up to fewer blocks than you'd expect. A fresh F15FS has 65536 - 41 free (65495)
    // We get 6 directs through our inode, so 6 file blocks, 65489 free
    // now we have to start allocating indirects. We get one indirect in out inode, and it gives us 256 blocks
    // 262 file blocks, 1 overhead, 65232 free
    // Now we make our double indirect (and others, but taking this in steps), it is an overhead of 1 for 256 indirects
    // 262 file, 2 overhead, 65231 free
    // From here we have room for 256 indirects, each indirect taking 256 file blocks, and 1 overhead
    // 256 * 257 is 65792, though, so we don't max out the double indirect
    // 65231/257 = 253.81, so we get 254 indirects, but one isn't completely full
    // Not counting the contents of the partially filled indirect (but counting the indirect itself), we have
    // 65030 file blocks, 256 overhead, and 209 free, which goes in that last indirect, so in total, one file can take up
    // 65239 file blocks, with 256 overhead, and 0 free blocks remaining

    for (unsigned i = 0; i < 65239; ++i) {
        assert(fs_write_file(fs, filenames[0], file_data, 1024, i * 1024) == 1024);
    }
    // File system is now FULL.
    // Throw in that test for FS_CREATE_FILE

    // FS_CREATE_FILE 13
    assert(fs_create_file(fs, filenames[1], DIRECTORY) < 0);

    // FS_WRITE_FILE 9
    assert(fs_write_file(fs, filenames[0], file_data, 1024, 65495 * 1024) < 0); // out of room should be an error, not a 0 written

    assert(fs_unmount(fs) == 0);

    fs = fs_mount(test_fname[0]);
    assert(fs);

    // FS_WRITE_FILE 10
    assert(fs_write_file(fs, filenames[4], file_data, 300, 0) < 0);

    // FS_WRITE_FILE 11
    assert(fs_write_file(fs, filenames[1], file_data, 300, 0) < 0);

    // FS_WRITE_FILE 12
    assert(fs_write_file(fs, filenames[2], file_data, SIZE_MAX - 3, 10) < 0);

    // FS_WRITE_FILE 13
    assert(fs_write_file(NULL, filenames[2], file_data, 300, 0) < 0);

    // FS_WRITE_FILE 14
    assert(fs_write_file(fs, NULL, file_data, 300, 0) < 0);

    // FS_WRITE_FILE 15
    assert(fs_write_file(fs, "", file_data, 300, 0) < 0);

    // FS_WRITE_FILE 16
    assert(fs_write_file(fs, filenames[2], NULL, 300, 0) < 0);

    // FS_WRITE_FILE 17
    assert(fs_write_file(fs, filenames[2], file_data, 0, 0) <= 0); // Allow an error or 0 written

    // FS_WRITE_FILE 18
    assert(fs_write_file(fs, filenames[2], file_data, 300, 4096) < 0);

    assert(fs_unmount(fs) == 0);

}

// FS_READ_FILE
void tests_f() {

    assert(system("cp e_tests_normal.f15fs f_tests_normal.f15fs") == 0);

    const char *test_fname = "f_tests_normal.f15fs";
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };

    // file 0 should go into double_indirect, file 2 goes to 1024

    F15FS_t *fs = fs_mount(test_fname);
    assert(fs);

    uint8_t read_space[4096];
    uint8_t check_val[4096];
    memset(check_val, 1, 4096); // Didn't actually write any other values

    // FS_READ_FILE 1
    assert(fs_read_file(fs, filenames[2], read_space, 300, 0) == 300);
    assert(memcmp(read_space, check_val, 300) == 0);

    // FS_READ_FILE 2
    assert(fs_read_file(fs, filenames[0], read_space, 1024, 512) == 1024);
    assert(memcmp(read_space, check_val, 1024) == 0);

    // FS_READ_FILE 3
    assert(fs_read_file(fs, filenames[0], read_space, 1024, 1024) == 1024);
    assert(memcmp(read_space, check_val, 1024) == 0);

    // FS_READ_FILE 4
    assert(fs_read_file(fs, filenames[0], read_space, 4096, 1024) == 4096);
    assert(memcmp(read_space, check_val, 4096) == 0);

    // FS_READ_FILE 5
    assert(fs_read_file(fs, filenames[0], read_space, 4096, 4096) == 4096);
    assert(memcmp(read_space, check_val, 4096) == 0);

    // FS_READ_FILE 6
    assert(fs_read_file(fs, filenames[0], read_space, 2048, 267264) == 2048);
    assert(memcmp(read_space, check_val, 1024) == 0);

    // FS_READ_FILE 7
    assert(fs_read_file(fs, filenames[5], read_space, 300, 0) < 0);

    // FS_READ_FILE 8
    assert(fs_read_file(fs, filenames[1], read_space, 300, 0) < 0);

    // FS_READ_FILE 9
    assert(fs_read_file(NULL, filenames[0], read_space, 300, 0) < 0);

    // FS_READ_FILE 10
    assert(fs_read_file(fs, NULL, read_space, 300, 0) < 0);

    // FS_READ_FILE 11
    assert(fs_read_file(fs, "", read_space, 300, 0) < 0);

    // FS_READ_FILE 12
    assert(fs_read_file(fs, filenames[0], NULL, 300, 0) < 0);

    // FS_READ_FILE 13
    assert(fs_read_file(fs, filenames[0], read_space, 0, 0) <= 0);

    // FS_READ_FILE 14
    assert(fs_read_file(fs, filenames[2], NULL, 300, 4173) < 0);

    // FS_READ_FILE 15
    assert(fs_read_file(fs, filenames[0], NULL, 300, 1024) < 0);

    assert(fs_unmount(fs) == 0);

}
#endif


// FS_REMOVE_FILE
void tests_g() {

    assert(system("cp e_tests_normal.f15fs g_tests_normal.f15fs") == 0);
    assert(system("cp e_tests_full.f15fs g_tests_full.f15fs") == 0);

    const char *(test_fname[2]) = {"g_tests_normal.f15fs", "g_tests_full.f15fs"};
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };


    dir_rec_t record_struct;

    F15FS_t *fs = fs_mount(test_fname[0]);


    // FS_REMOVE_FILE 1
    assert(fs_remove_file(fs, filenames[0]) == 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(filenames[0], '/') + 1, REGULAR, &record_struct) == false);

    // FS_REMOVE_FILE 5
    assert(fs_remove_file(fs, filenames[1]) < 0);

    // FS_REMOVE_FILE 2

    #if DESCRIPTOR_TESTS
    int fd = fs_open_file(fs, filenames[2]);
    assert(fd >= 0);
    #endif

    assert(fs_remove_file(fs, filenames[2]) == 0);
    printf("Made it this far 2\n");
    assert(fs_get_dir(fs, filenames[1], &record_struct) == 0);
    print_dir_records(filenames[1], &record_struct);
    assert(check_record_for_file(strrchr(filenames[2], '/') + 1, REGULAR, &record_struct) == false);

    #if DESCRIPTOR_TESTS
    uint8_t scratch_space[1024];
    assert(fs_read_file(fs, fd, scratch_space, 300) < 0);
    #endif

    // FS_REMOVE_FILE 3
    assert(fs_remove_file(fs, filenames[3]) == 0);

    assert(fs_get_dir(fs, filenames[1], &record_struct) == 0);
    print_dir_records(filenames[1], &record_struct);
    assert(check_record_for_file(strrchr(filenames[3], '/') + 1, DIRECTORY, &record_struct) == false);


    // FS_REMOVE_FILE 6
    assert(fs_remove_file(fs, filenames[4]) < 0);

    // FS_REMOVE_FILE 8
    assert(fs_remove_file(NULL, filenames[1]) < 0);

    // FS_REMOVE_FILE 9
    assert(fs_remove_file(fs, NULL) < 0);

    // FS_REMOVE_FILE 10
    assert(fs_remove_file(fs, "") < 0);

    // FS_REMOVE_FILE 7
    assert(fs_remove_file(fs, filenames[12]) < 0);

    assert(fs_unmount(fs) == 0);

    fs = fs_mount(test_fname[1]);
    assert(fs);

    // FS_REMOVE_FILE 4
    assert(fs_remove_file(fs, filenames[0]) == 0);

    assert(fs_unmount(fs) == 0);

}


// FS_MOVE_FILE
void tests_h() {

    assert(system("cp e_tests_normal.f15fs h_tests_normal_a.f15fs") == 0);
    assert(system("cp e_tests_normal.f15fs h_tests_normal_b.f15fs") == 0);
    assert(system("cp b_tests_full_table.f15fs h_tests_full_table.f15fs") == 0);

    const char *(test_fname[3]) = {"h_tests_normal_a.f15fs", "h_tests_normal_b.f15fs", "h_tests_full_table.f15fs"};
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };

    const char *(new_filenames[4]) = {
        "/new_file", "/new_folder", "/new_with_folder", "/new_with_file"
    };


    dir_rec_t record_struct;

    F15FS_t *fs = fs_mount(test_fname[0]);

    // FS_MOVE_FILE 1
    assert(fs_move_file(fs, filenames[0], new_filenames[0]) == 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(filenames[0], '/') + 1, REGULAR, &record_struct) == false);
    assert(check_record_for_file(strrchr(new_filenames[0], '/') + 1, REGULAR, &record_struct));

    // FS_MOVE_FILE 2
    #if DESCRIPTOR_TESTS
    int fd = fs_open_file(fs, filenames[2]);
    assert(fd >= 0);
    #endif

    assert(fs_move_file(fs, filenames[2], new_filenames[3]) == 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(new_filenames[3], '/') + 1, REGULAR, &record_struct));
    print_dir_records(filenames[1], &record_struct);
    assert(check_record_for_file(strrchr(filenames[2], '/') + 1, REGULAR, &record_struct) == false);

    #if DESCRIPTOR_TESTS
    uint8_t scratch_space[300];
    assert(fs_read_file(fs, fd, scratch_space, 300) == 300);
    #endif

    // FS_MOVE_FILE 3
    assert(fs_move_file(fs, filenames[1], new_filenames[1]) == 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(new_filenames[1], '/') + 1, DIRECTORY, &record_struct));
    assert(check_record_for_file(strrchr(filenames[1], '/') + 1, DIRECTORY, &record_struct) == false);

    // Cycle files because it's easier to know where all these files are
    assert(fs_unmount(fs) == 0);
    fs = fs_mount(test_fname[1]);
    assert(fs);

    // FS_MOVE_FILE 5
    assert(fs_move_file(fs, filenames[0], filenames[2]) < 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(filenames[0], '/') + 1, REGULAR, &record_struct));
    print_dir_records(filenames[1], &record_struct);
    assert(check_record_for_file(strrchr(filenames[2], '/') + 1, REGULAR, &record_struct) == false);

    // FS_MOVE_FILE 6

    assert(fs_move_file(fs, filenames[0], filenames[6]) < 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(filenames[0], '/') + 1, REGULAR, &record_struct));

    // FS_MOVE_FILE 8
    assert(fs_move_file(fs, filenames[4], new_filenames[1]) < 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(new_filenames[1], '/') + 1, REGULAR, &record_struct) == false);
    assert(check_record_for_file(strrchr(new_filenames[1], '/') + 1, DIRECTORY, &record_struct) == false);


    // FS_MOVE_FILE 9
    assert(fs_move_file(fs, filenames[0], filenames[0]) <= 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(filenames[0], '/') + 1, REGULAR, &record_struct));

    // FS_MOVE_FILE 10
    assert(fs_move_file(NULL, filenames[0], new_filenames[1]) < 0);

    // FS_MOVE_FILE 11
    assert(fs_move_file(fs, NULL, new_filenames[1]) < 0);

    // FS_MOVE_FILE 12
    assert(fs_move_file(fs, "", new_filenames[1]) < 0);

    // FS_MOVE_FILE 13
    assert(fs_move_file(fs, filenames[12], new_filenames[1]) < 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(new_filenames[1], '/') + 1, DIRECTORY, &record_struct) == false);

    // FS_MOVE_FILE 14
    assert(fs_move_file(fs, filenames[0], NULL) < 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(filenames[0], '/') + 1, REGULAR, &record_struct));

    // FS_MOVE_FILE 15
    assert(fs_move_file(fs, filenames[0], "") < 0);

    assert(fs_get_dir(fs, filenames[12], &record_struct) == 0);
    print_dir_records(filenames[12], &record_struct);
    assert(check_record_for_file(strrchr(filenames[0], '/') + 1, REGULAR, &record_struct));

    // FS_MOVE_FILE 16
    assert(fs_move_file(fs, filenames[1], filenames[12]) < 0);



    assert(fs_unmount(fs) == 0);
    fs = fs_mount(test_fname[2]);
    assert(fs);

    // FS_MOVE_FILE 7
    assert(fs_move_file(fs, "/a/a", "/b/z") < 0);

    assert(fs_get_dir(fs, "/a", &record_struct) == 0);
    print_dir_records("/a", &record_struct);
    assert(check_record_for_file("a", REGULAR, &record_struct));

    assert(fs_get_dir(fs, "/b", &record_struct) == 0);
    print_dir_records("/b", &record_struct);
    assert(check_record_for_file("z", REGULAR, &record_struct) == false);

    // FS_MOVE_FILE 4
    assert(fs_move_file(fs, "/a/a", "/a/z") == 0);

    assert(fs_get_dir(fs, "/a", &record_struct) == 0);
    print_dir_records("/a", &record_struct);
    assert(check_record_for_file("a", REGULAR, &record_struct) == false);
    assert(check_record_for_file("z", REGULAR, &record_struct));

    assert(fs_unmount(fs) == 0);

}

/*
    int fs_seek_file(F15FS_t *const fs, const int fd, const off_t offset, const seek_t whence)
    1. Normal, wherever, really
    2. Error, seek past beginning - resulting location unspecified by our api, can't really test?
    3. Error, seek past end - resulting location unspecified by our api, can't really test?
    4. Error, FS null
    5. Error, fd invalid
    6. Error, whence not a valid value

    int fs_close_file(F15FS_t *const fs, const int fd)
    1. Normal, whatever, attempt to use after closing, assert failure
    2. Normal, multiple opens, close does not affect the others
    3. Error, FS null
    4. Error, invalid fd
*/

#if DESCRIPTOR_TESTS

// FS_SEEK_FILE
void tests_i() {

    assert(system("cp e_tests_normal.f15fs i_tests_normal.f15fs") == 0);

    const char *(test_fname[1]) = {"i_tests_normal.f15fs"};
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };

    F15FS_t *fs = fs_mount(test_fname[0]);

    // Since we can't ask our position, it's a bit harder to test seek
    // Also, since I don't have an FD version, it's a little hard to verify that these tests are correct

    const char *comp_str = "TESTSTRINGWOOOOOOO!";
    char rw_str[20];

    // FS_SEEK_FILE 1

    int fd[2];
    fd[0] = fs_open_file(fs, filenames[0]);
    assert(fd >= 0);

    assert(fs_write_file(fs, fd[0], comp_str, 20) == 20);

    fd[1] = fs_open_file(fs, filenames[0]);
    assert(fd[1] >= 0);


    // fd 0 is at the end of test string
    // fd 1 is at the beginning

    fs_seek_file(fs, fd[0], -10, FS_SEEK_CUR);
    // should be back 10 positions, so... 'W'
    assert(fs_read_file(fs, fd[0], rw_str, 3) == 3);
    assert(rw_str[0] == 'W' && rw_str[1] == 'O' && rw_str[2] == 'O');
    // check it didn't mess with fd[1]

    assert(fs_read_file(fs, fd[1], rw_str, 3) == 3);
    assert(rw_str[0] == 'T' && rw_str[1] == 'E' && rw_str[2] == 'S');

    // Seek to begin, read everything
    fs_seek_file(fs, fd[0], 0, FS_SEEK_BEGIN);
    assert(fs_read_file(fs, fd[0], rw_str, 20) == 20);
    assert(strncmp(rw_str, comp_str, 20) == 0);

    // Close enough.

    // FS_SEEK_FILE 2
    assert(fs_seek_file(fs, fd[0], -20, FS_SEEK_BEGIN) <= 0);

    // FS_SEEK_FILE 3
    assert(fs_seek_file(fs, fd[0], 20, FS_SEEK_END) <= 0);

    // FS_SEEK_FILE 4
    assert(fs_seek_file(NULL, fd[0], -20, FS_SEEK_END) <= 0);

    // FS_SEEK_FILE 5
    assert(fs_seek_file(fs, 99999, 0, FS_SEEK_BEGIN) <= 0);

    // FS_SEEK_FILE 6
    assert(fs_seek_file(fs, fd[1], 0, 9999) <= 0);

    assert(fs_unmount(fs) == 0);
}

// FS_CLOSE_FILE
void tests_j() {

    /*
        int fs_close_file(F15FS_t *const fs, const int fd)
        1. Normal, whatever, attempt to use after closing, assert failure
        2. Normal, multiple opens, close does not affect the others
        3. Error, FS null
        4. Error, invalid fd
    */

    assert(system("cp e_tests_normal.f15fs j_tests_normal.f15fs") == 0);

    const char *(test_fname[1]) = {"j_tests_normal.f15fs"};
    const char *(filenames[13]) = {
        "/file", "/folder", "/folder/with_file", "/folder/with_folder",
        "/DOESNOTEXIST", "/file/BAD_REQUEST", "/DOESNOTEXIST/with_file", "/folder/with_file/bad_req",
        "folder/missing_slash", "/folder/new_folder/", "/folder/withwaytoolongfilenamethattakesupmorespacethanitshould/bad_req",
        "/folder/withfilethatiswayyyyytoolongwhydoyoumakefilesthataretoobig", "/"
    };

    F15FS_t *fs = fs_mount(test_fname[0]);

    int fd[2];

    // FS_CLOSE_FILE 2
    fd[0] = fs_open_file(fs, filenames[0]);
    assert(fd[0] >= 0);

    fd[1] = fs_open_file(fs, filenames[0]);
    assert(fd[1] >= 0);

    assert(fs_close_file(fs, fd[0]) == 0);

    assert(fs_seek_file(fs, fd[1], 0, FS_SEEK_BEGIN) == 0);

    // FS_CLOSE_FILE 1
    assert(fs_seek_file(fs, fd[0], 0, FS_SEEK_BEGIN) < 0);




    // FS_CLOSE_FILE 3
    assert(fs_close_file(NULL, fd[1]) < 0);



    assert(fs_close_file(fs, fd[1]) == 0);


    assert(fs_close_file(fs, 99999) < 0);

    assert(fs_unmount(fs) == 0);

}
#endif
