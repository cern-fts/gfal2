/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */



#include "gfal_srm_mock_test.h"
#include <string.h>
#include <cgreen/cgreen.h>
#include <errno.h>
#include <stdlib.h>

#include "unit_test_util.h"

void setup_mock_srm(){
	if(gfal2_tests_is_mock() ){
		setup_mock_bdii();
		gfal_srm_external_call.srm_prepare_to_get = &srm_mock_srm_prepare_to_get;
		gfal_srm_external_call.srm_prepare_to_put = &srm_mock_srm_prepare_to_put;
		gfal_srm_external_call.srm_context_init = &srm_mock_srm_context_init;
		gfal_srm_external_call.srm_check_permission= &srm_mock_srm_check_permission;
		gfal_srm_external_call.srm_ls = &srm_mock_srm_ls;
		gfal_srm_external_call.srm_mkdir = &srm_mock_srm_mkdir;
		gfal_srm_external_call.srm_rmdir = &srm_mock_srm_rmdir;
		gfal_srm_external_call.srm_put_done = &srm_mock_srm_put_done;
		gfal_srm_external_call.srm_setpermission= & srm_mock_srm_setpermission;
		gfal_srm_external_call.srm_srmv2_pinfilestatus_delete = &srm_mock_srm_srmv2_pinfilestatus_delete;
		gfal_srm_external_call.srm_srm2__TReturnStatus_delete = &srm_mock_srm_srm2__TReturnStatus_delete;
	}
}


void add_mock_srm_ls_locality_valid(const char* file, const char* endpoint, TFileLocality tl){
	if(gfal2_tests_is_mock() ){
		define_mock_locality_file_valid(file, tl);
		define_mock_endpoints(endpoint);

		will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, endpoint));
		will_respond(srm_mock_srm_ls, 0, want_non_null(context), want_non_null(inut), want_non_null(output));	
	}	
}


void add_mock_srm_ls_error(const char* file, const char* endpoint, int status, const char* err){
	if(gfal2_tests_is_mock()){
		define_mock_stat_file_error(file, status, err);
		define_mock_endpoints(endpoint);
		will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, endpoint));
		will_respond(srm_mock_srm_ls, 0, want_non_null(context), want_non_null(inut), want_non_null(output));			
	}	
}


struct srm_ls_output defined_srm_ls_output;
struct srm_rmdir_output defined_srm_rmdir_output;
struct srm_getpermission_output defined_srm_getpermission_output;
struct srmv2_filestatus* defined_srmv2_filestatus=NULL;
struct srmv2_pinfilestatus * defined_get_output=NULL;
struct srmv2_filestatus* defined_put_done=NULL;
struct srmv2_pinfilestatus * defined_put_output=NULL;

void define_mock_stat_file_valid(char* surl, mode_t mode, uid_t uid, gid_t gid){
	defined_srm_ls_output.statuses= g_new0(struct srmv2_mdfilestatus,1);
	defined_srm_ls_output.statuses->surl = strdup(surl);
	memset(&defined_srm_ls_output.statuses->stat, 0, sizeof(struct stat));
	defined_srm_ls_output.statuses->stat.st_mode = mode;
	defined_srm_ls_output.statuses->stat.st_uid = uid;
	defined_srm_ls_output.statuses->stat.st_gid= gid;
	
}

void define_mock_locality_file_valid(char* surl, TFileLocality tl){
	defined_srm_ls_output.statuses= g_new0(struct srmv2_mdfilestatus,1);
	defined_srm_ls_output.statuses->surl = strdup(surl);
	memset(&defined_srm_ls_output.statuses->stat, 0, sizeof(struct stat));
	defined_srm_ls_output.statuses->locality = tl;
	
}

void define_mock_readdir_file_valid(char** surls, mode_t* mode, uid_t* uid, gid_t* gid, int n){
	defined_srm_ls_output.statuses->subpaths = g_new0(struct srmv2_mdfilestatus,n);
	defined_srm_ls_output.statuses->nbsubpaths= n;	
	int i;
	for(i=0; i< n; ++i){
		defined_srm_ls_output.statuses->subpaths[i].surl = strdup(surls[i]);
		memset(&defined_srm_ls_output.statuses->subpaths[i].stat, 0, sizeof(struct stat));
		defined_srm_ls_output.statuses->subpaths[i].stat.st_mode = mode[i];
		defined_srm_ls_output.statuses->subpaths[i].stat.st_uid = uid[i];
		defined_srm_ls_output.statuses->subpaths[i].stat.st_gid= gid[i];
	}
}

void define_mock_stat_file_error(char* surl, int status, char* err){
	defined_srm_ls_output.statuses= g_new0(struct srmv2_mdfilestatus,1);
	defined_srm_ls_output.statuses->surl = strdup(surl);
	memset(&defined_srm_ls_output.statuses->stat, 0, sizeof(struct stat));
	defined_srm_ls_output.statuses->status = status;
	defined_srm_ls_output.statuses->explanation = strdup(err);
}

void define_put_done(int number, char** surl, char** explanation, char** turl, int* status){
	int i;
	defined_put_done= calloc(sizeof(struct srmv2_filestatus), number);
	for(i=0; i < number; ++i){
		if(surl)
			defined_put_done[i].surl = strdup(surl[i]);
		if(explanation)
			defined_put_done[i].explanation = strdup(explanation[i]);
		if(turl)
			defined_put_done[i].turl = strdup(turl[i]);
		if(status)
			defined_put_done[i].status = status[i];
	}
}

void define_mock_srmv2_filestatus(int number, char** surl, char** explanation, char** turl, int* status){
	int i;
	defined_srmv2_filestatus= calloc(sizeof(struct srmv2_filestatus), number);
	for(i=0; i < number; ++i){
		if(surl)
			defined_srmv2_filestatus[i].surl = strdup(surl[i]);
		if(explanation)
			defined_srmv2_filestatus[i].explanation = strdup(explanation[i]);
		if(turl)
			defined_srmv2_filestatus[i].turl = strdup(turl[i]);
		if(status)
			defined_srmv2_filestatus[i].status = status[i];
	}
}


void define_mock_srmv2_pinfilestatus(int number, char** surl, char** explanation, char** turl, int* status){
	int i;
	defined_get_output= calloc(sizeof(struct srmv2_pinfilestatus), number);
	for(i=0; i < number; ++i){
		if(surl)
			defined_get_output[i].surl = strdup(surl[i]);
		if(explanation)
			defined_get_output[i].explanation = strdup(explanation[i]);
		if(turl)
			defined_get_output[i].turl = strdup(turl[i]);
		if(status)
			defined_get_output[i].status = status[i];
	}
}

void define_mock_srmv2_putoutput(int number, char** surl, char** explanation, char** turl, int* status){
	int i;
	defined_put_output= calloc(sizeof(struct srmv2_pinfilestatus), number);
	for(i=0; i < number; ++i){
		if(surl)
			defined_put_output[i].surl = strdup(surl[i]);
		if(explanation)
			defined_put_output[i].explanation = strdup(explanation[i]);
		if(turl)
			defined_put_output[i].turl = strdup(turl[i]);
		if(status)
			defined_put_output[i].status = status[i];
	}
}


void define_mock_defined_srm_rmdir_output(char* surl, int status){
	defined_srm_rmdir_output.statuses= calloc(sizeof(struct srmv2_filestatus), 1);
	defined_srm_rmdir_output.statuses->surl = strdup(surl);
	defined_srm_rmdir_output.statuses->status = status;	
}

void srm_mock_srm_context_init(struct srm_context *context,char *srm_endpoint,char *errbuf,int errbufsz,int verbose){
	mock(context, srm_endpoint, errbuf, errbufsz, verbose);
}

int srm_mock_srm_ls(struct srm_context *context,
	struct srm_ls_input *input,struct srm_ls_output *output){
	int a = mock(context, input, output);
	if(a){
		errno = a;
		return -1;
	}
	memcpy(output, &defined_srm_ls_output, sizeof(struct srm_ls_output));
	return 0;
}
	
int srm_mock_srm_rmdir(struct srm_context *context,
	struct srm_rmdir_input *input,struct srm_rmdir_output *output){
	int a = mock(context, input, output);
	if(a){
		errno = a;
		return -1;
	}
	memcpy(output, &defined_srm_rmdir_output, sizeof(struct srm_rmdir_output));
	return 0;		
}
	
int srm_mock_srm_mkdir(struct srm_context *context,
	struct srm_mkdir_input *input){
	int a = mock(context, input);
	if(a<0){
		errno = -a;
		return -1;
	}
	return a;		
}
	
int srm_mock_srm_getpermission (struct srm_context *context,
	struct srm_getpermission_input *input,struct srm_getpermission_output *output){
	int a = mock(context, input, output);
	if(a<0){
		errno = a;
		return -1;
	}
	memcpy(output, &defined_srm_getpermission_output, sizeof(struct srm_getpermission_output));
	return a;			
}

int srm_mock_srm_check_permission(struct srm_context *context,
	struct srm_checkpermission_input *input,struct srmv2_filestatus **statuses){
	int a = mock(context, input, statuses);
	if(a <0){
		errno = -a;
		return -1;
	}
	*statuses = defined_srmv2_filestatus; 
	return a;				
}

int srm_mock_srm_prepare_to_get(struct srm_context *context,
	struct srm_preparetoget_input *input,struct srm_preparetoget_output *output){
	int a = mock(context, input, output);
	if(a <0 ){
		errno = -a;
		return -1;
	}
	output->filestatuses = defined_get_output;
	return a;				
			
}

int srm_mock_srm_put_done(struct srm_context *context,
		struct srm_putdone_input *input, struct srmv2_filestatus **statuses){
		int a = mock(context, input, statuses);
		if(a < 0){
			errno =a;
			return -1;
		}
	*statuses = defined_put_done;
	return a;		
}

int srm_mock_srm_prepare_to_put(struct srm_context *context,
		struct srm_preparetoput_input *input,struct srm_preparetoput_output *output){
	int a = mock(context, input, output);
	if(a <0 ){
		errno = -a;
		return -1;
	}
	output->filestatuses = defined_put_output;
	return a;
}

int srm_mock_srm_setpermission (struct srm_context *context,
		struct srm_setpermission_input *input){
	int a = mock(context, input);
	if(a!=0){
		errno = a;
		return -1;
	}
	return 0;
}
	
void srm_mock_srm_srmv2_pinfilestatus_delete(struct srmv2_pinfilestatus*  srmv2_pinstatuses, int n){
	mock(srmv2_pinstatuses, n);	
}

void srm_mock_srm_srmv2_mdfilestatus_delete(struct srmv2_mdfilestatus* mdfilestatus, int n){
	mock(mdfilestatus, n);
}

void srm_mock_srm_srmv2_filestatus_delete(struct srmv2_filestatus*  srmv2_statuses, int n){
	mock(srmv2_statuses, n);
}

void srm_mock_srm_srm2__TReturnStatus_delete(struct srm2__TReturnStatus* status){
	mock(status);
}


// convenience functions
