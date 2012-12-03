#pragma once
/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */
 
#include "gfal_mds_mock_test.h" 
#include <gfal_srm_ifce.h>
#include <gfal_srm_ifce_types.h>
#include "srm/gfal_common_srm.h"
#include "srm/gfal_common_srm_internal_layer.h"


void setup_mock_srm();

void add_mock_srm_ls_locality_valid(const char* file, const char* endpoint, TFileLocality tl);

void add_mock_srm_ls_error(const char* file, const char* endpoint, int status, const char* err);


extern struct srm_ls_output defined_srm_ls_output;
extern struct srm_rmdir_output defined_srm_rmdir_output;
extern struct srm_getpermission_output defined_srm_getpermission_output;
extern struct srmv2_filestatus* defined_srmv2_filestatus;
extern struct srmv2_pinfilestatus * defined_get_output;


// convenience functions
void define_mock_stat_file_valid(char* surl, mode_t mode, uid_t uid, gid_t gid);
void define_mock_readdir_file_valid(char** surls, mode_t* mode, uid_t* uid, gid_t* gid, int n);
void define_mock_srmv2_filestatus(int number, char** surl, char** explanation, char** turl, int* status);
void define_mock_stat_file(mode_t mode, int uid, int gid);
void define_mock_stat_file_error(char* surl, int status, char* err);
void define_mock_srmv2_pinfilestatus(int number, char** surl, char** explanation, char** turl, int* status);
void define_put_done(int number, char** surl, char** explanation, char** turl, int* status);
void define_mock_defined_srm_rmdir_output(char* surl, int status);
void define_mock_srmv2_putoutput(int number, char** surl, char** explanation, char** turl, int* status);

void define_mock_locality_file_valid(char* surl, TFileLocality tl);
// mock functions


void srm_mock_srm_context_init(struct srm_context *context,char *srm_endpoint,char *errbuf,int errbufsz,int verbose);

int srm_mock_srm_ls(struct srm_context *context,
	struct srm_ls_input *input,struct srm_ls_output *output);
	
int srm_mock_srm_rmdir(struct srm_context *context,
	struct srm_rmdir_input *input,struct srm_rmdir_output *output);
	
int srm_mock_srm_mkdir(struct srm_context *context,
	struct srm_mkdir_input *input);
	
int srm_mock_srm_getpermission (struct srm_context *context,
	struct srm_getpermission_input *input,struct srm_getpermission_output *output);

int srm_mock_srm_check_permission(struct srm_context *context,
	struct srm_checkpermission_input *input,struct srmv2_filestatus **statuses);	
	
int srm_mock_srm_prepare_to_get(struct srm_context *context,
		struct srm_preparetoget_input *input,struct srm_preparetoget_output *output);
		
int srm_mock_srm_prepare_to_put(struct srm_context *context,
		struct srm_preparetoput_input *input,struct srm_preparetoput_output *output);
		
int srm_mock_srm_put_done(struct srm_context *context,
		struct srm_putdone_input *input, struct srmv2_filestatus **statuses);
		

int srm_mock_srm_setpermission (struct srm_context *context,
		struct srm_setpermission_input *input);
	
void srm_mock_srm_srmv2_pinfilestatus_delete(struct srmv2_pinfilestatus*  srmv2_pinstatuses, int n);

void srm_mock_srm_srmv2_mdfilestatus_delete(struct srmv2_mdfilestatus* mdfilestatus, int n);

void srm_mock_srm_srmv2_filestatus_delete(struct srmv2_filestatus*  srmv2_statuses, int n);

void srm_mock_srm_srm2__TReturnStatus_delete(struct srm2__TReturnStatus* status);


