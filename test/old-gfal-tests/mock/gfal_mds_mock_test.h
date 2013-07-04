#pragma once
/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */


void setup_mock_bdii();

void mock_endpoint_answer(const char * endpoint, const char * basename);


extern char** define_se_endpoints;
extern char** define_se_types;
extern char* define_lfc_endpoint;

// convenience functions

void define_mock_endpoints(char* endpoint);


// mocks

