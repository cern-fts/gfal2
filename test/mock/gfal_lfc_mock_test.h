#pragma once
/*

*	auto-generated header file for file test/mock/gfal_lfc_mock_test.c 
 
*/

#include <lfc/lfc_api.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

void test_mock_lfc(gfal_handle handle, GError** err);

void add_mock_error_lfc_guid_resolution(const char * lfn, int error);
void add_mock_valid_lfc_guid_resolution(const char * lfn, const char* guid);

void setup_mock_lfc();


extern int lfc_last_err;
extern struct lfc_filestatg* defined_filestatg;
extern struct lfc_filestat* defined_filestat;
extern struct lfc_filereplica* define_lastfilereplica;
extern struct lfc_linkinfo *define_linkinfos;
extern int define_numberlinkinfos;
extern int define_numberreplica;

// convenience functions
void define_mock_linkinfos(int number, char** resu);

void define_mock_filestatg(mode_t mode, int gid, int uid);

void define_mock_filelstat(mode_t mode, int gid, int uid);

void define_mock_filereplica(int n, char** rep_turls);

void define_lfc_comment(char* comment);


// mock

int* lfc_mock_C__serrno();

int	lfc_mock_endsess();

int lfc_mock_rmdir(const char* path);

int lfc_mock_startsession(char* server, char* comment);

int	lfc_mock_statg(const char * lfn, const char * guid, struct lfc_filestatg * f);

int	lfc_mock_lstatg(const char * lfn, struct lfc_filestat * f);

int lfc_mock_rename(const char * oldpath, const char* newpath);

int lfc_mock_access(const char* path, int mode);

int lfc_mock_chmod(const char* path, mode_t mode);

lfc_DIR* lfc_mock_opendir(const char* path, const char* guid);

int lfc_mock_closedir(lfc_DIR* dir);

struct dirent* lfc_mock_readdir(lfc_DIR* d);

struct Cns_direnstat* lfc_mock_readdirx(lfc_DIR* d);

int lfc_mock_mkdir(const char* path, const char* guid,  mode_t mode);

int	lfc_mock_starttrans(const char *server, const char *comment);

int	lfc_mock_endtrans();

int lfc_mock_aborttrans();

int lfc_mock_getcomment (const char * path, char * comment);

int lfc_mock_setcomment (const char * path, char * comment);

int lfc_mock_getreplica(const char *path, const char *guid, const char *se, int *nbentries, struct lfc_filereplica **rep_entries);

int lfc_mock_getlinks(const char *path, const char *guid, int *nbentries, struct Cns_linkinfo **linkinfos);

void lfc_next_valid_statg_values(const char* lfn, const char* guid, struct lfc_filestatg * f);
void lfc_statg_all_invalid();



