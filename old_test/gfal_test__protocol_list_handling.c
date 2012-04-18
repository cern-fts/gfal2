/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Test if the logic recognizes unknown protocols of wrongly formatted
 * protocol lists in the transfetParameters property of PrepareToGet.
 */
#include "srm_dependencies.h"
#include "srm2_2_ifce.h"
#include "gfal_api.h"
#include <assert.h>

#include "gfal_testsuite.h"

/* WARNING: the test functions may have memory leaks. Due to the nature of the
 * test application, they are not handled elaborately, do not worry if you find
 * some... They do not affect the test execution and production.*/

#define __CALL_TEST(test) \
    printf("\nTest group: %s\n", #test); \
    res = (test); \
    if (!res) \
        return res;

/* The actual scenario to te be tested */
typedef enum _protocol_list_scenario_type {
    E_UNSUPPORTED_PROTOCOL,
    E_EMPTY_LIST,
    E_UNUSED
} _protocol_list_scenario_type_t;

/* The variable controls the actual scenario. The test function sets it,
 * then calls the function to be tested. Deep inside, the function will call
 * the (replaced) SOAP caller. The mock SOAP caller prepares different SRM
 * responses and return it, without the physical gSOAP/network operation.
 * The response content is controlled by this variable (the scenario). */
static _protocol_list_scenario_type_t _protocol_list_scenario = E_UNUSED;

/* A fixture with a protocol list containing an unsupported protocol name */
static char* _unsupported_protocol[] = {"unsupported_protocol", NULL};

/* Signature for srmv2_turlsfromsurls_get, srmv2_gete, srmv2_prestagee,
 * srmv2_bringonline. As the signatures are common, we can reuse the same
 * test function code. */
typedef int (*srmv2_fv_type)(int, const char **, const char *, int, const char *,
    char **, char **, struct srmv2_pinfilestatus **, char *, int, int);

/* There is only 1 file in the requests, and only 1 corresponding file status in
 * the responses. */
#define NUMBER_OF_FILES 1

/* Set up the common part of the SRM responses. The common part: all the tested
 * SRM responses have the same fields, and their values are the same in case of
 * all tests/scenarios. */
static void _prepare_response_common(
    struct soap* soap,
    struct srm2__TReturnStatus ** status,
    int *remainingTotalRequestTime,
    int* __sizestatusArray)
{
    assert(status);
    assert(__sizestatusArray);
    *status = soap_malloc(soap, sizeof(struct srm2__TReturnStatus));
    (*status)->explanation = 0;
    remainingTotalRequestTime = 0;
    *(__sizestatusArray) = NUMBER_OF_FILES;
}

/* Create the SRM response for the srmPrepareToGet call, according to the
 * actual test scenario. */
static void _prepare_response_prepare_to_get(
    struct soap* soap,
    struct srm2__srmPrepareToGetResponse_ * rep)
{
    static struct srm2__TGetRequestFileStatus * mock_statuses[NUMBER_OF_FILES];
    assert(soap);
    assert(rep);

    rep->srmPrepareToGetResponse = soap_malloc(
        soap, sizeof(struct srm2__srmPrepareToGetResponse));

    rep->srmPrepareToGetResponse->arrayOfFileStatuses = soap_malloc(
        soap, sizeof(struct srm2__ArrayOfTGetRequestFileStatus));

    rep->srmPrepareToGetResponse->arrayOfFileStatuses->statusArray = mock_statuses;

    _prepare_response_common(
        soap,
        &(rep->srmPrepareToGetResponse->returnStatus),
        rep->srmPrepareToGetResponse->remainingTotalRequestTime,
        &(rep->srmPrepareToGetResponse->arrayOfFileStatuses->__sizestatusArray)
    );
}

/* Create the SRM response for the srmBringOnline call, according to the
 * actual test scenario. */
static void _prepare_response_bring_online(
    struct soap* soap, struct srm2__srmBringOnlineResponse_ * rep)
{
    static struct srm2__TBringOnlineRequestFileStatus * mock_statuses[NUMBER_OF_FILES];
    assert(soap);
    assert(rep);

    rep->srmBringOnlineResponse = soap_malloc(
        soap, sizeof(struct srm2__srmBringOnlineResponse));

    rep->srmBringOnlineResponse->arrayOfFileStatuses = soap_malloc(
        soap, sizeof(struct srm2__ArrayOfTBringOnlineRequestFileStatus));

    rep->srmBringOnlineResponse->arrayOfFileStatuses->statusArray = mock_statuses;

    _prepare_response_common(
        soap,
        &(rep->srmBringOnlineResponse->returnStatus),
        rep->srmBringOnlineResponse->remainingTotalRequestTime,
        &(rep->srmBringOnlineResponse->arrayOfFileStatuses->__sizestatusArray)
    );
}

/* Create the SRM response for the srmPrepareToPut call, according to the
 * actual test scenario. */
static void _prepare_response_prepare_to_put(
    struct soap* soap, struct srm2__srmPrepareToPutResponse_ * rep)
{
    static struct srm2__TPutRequestFileStatus * mock_statuses[NUMBER_OF_FILES];
    assert(soap);
    assert(rep);

    rep->srmPrepareToPutResponse = soap_malloc(
        soap, sizeof(struct srm2__srmPrepareToPutResponse));

    rep->srmPrepareToPutResponse->arrayOfFileStatuses = soap_malloc(
        soap, sizeof(struct srm2__ArrayOfTPutRequestFileStatus));

    rep->srmPrepareToPutResponse->arrayOfFileStatuses->statusArray = mock_statuses;

    _prepare_response_common(
        soap,
        &(rep->srmPrepareToPutResponse->returnStatus),
        rep->srmPrepareToPutResponse->remainingTotalRequestTime,
        &(rep->srmPrepareToPutResponse->arrayOfFileStatuses->__sizestatusArray)
    );
}

/* The dependency injection comes here. This function replaces the production
 * version SOAP calls so that no network operation, server, etc be required.
 * The function directly produces and returns the SRM response objects.
 *
 * This is the "SOAP call" for the srmPrepareToGet operation. */
static int _caller_play_scenarios_prepare_to_get(
    struct soap * soap, const char * srm_endpoint, const char * srmfunc,
    struct srm2__srmPrepareToGetRequest * req,
    struct srm2__srmPrepareToGetResponse_ * rep)
{
    _prepare_response_prepare_to_get(soap, rep);

    switch(_protocol_list_scenario) {
    case E_EMPTY_LIST:
        GFAL_TEST_EQUAL(0, req->transferParameters->arrayOfTransferProtocols);
        /* Pretend that the call itself is successful, and do nothing */
         rep->srmPrepareToGetResponse->returnStatus->statusCode = SRM_USCORESUCCESS;
        return SOAP_OK;

    case E_UNSUPPORTED_PROTOCOL:
        /* Check if the function filled in the appropriate fields of the request */
        GFAL_TEST_EQUAL(1,
            req->transferParameters->arrayOfTransferProtocols->__sizestringArray);

        GFAL_TEST_ASSERT(
            strcmp(req->transferParameters->arrayOfTransferProtocols->stringArray[0],
                   _unsupported_protocol[0]) == 0
        );
        /* In case of unsupported protocol, the return status is SRM_NOT_SUPPORTED */
        rep->srmPrepareToGetResponse->returnStatus->statusCode = SRM_USCORENOT_USCORESUPPORTED;
        rep->srmPrepareToGetResponse->arrayOfFileStatuses->__sizestatusArray = 0;
        return SOAP_OK;
    default:
        assert(0);
    }

    return 1;
}

/* The dependency injection comes here. This function replaces the production
 * version SOAP calls so that no network operation, server, etc be required.
 * The function directly produces and returns the SRM response objects.
 *
 * This is the "SOAP call" for the srmBringOnline operation. */
static int _caller_play_scenarios_bring_online(
    struct soap * soap, const char * srm_endpoint, const char * srmfunc,
    struct srm2__srmBringOnlineRequest * req,
    struct srm2__srmBringOnlineResponse_ * rep)
{
    _prepare_response_bring_online(soap, rep);

    switch(_protocol_list_scenario) {
    case E_EMPTY_LIST:
        GFAL_TEST_EQUAL(0, req->transferParameters->arrayOfTransferProtocols);
        /* Pretend that the call itself is successful, and do nothing */
        rep->srmBringOnlineResponse->returnStatus->statusCode = SRM_USCORESUCCESS;
        return SOAP_OK;

    case E_UNSUPPORTED_PROTOCOL:
        /* Check if the function filled in the appropriate fields of the request */
        GFAL_TEST_EQUAL(1,
            req->transferParameters->arrayOfTransferProtocols->__sizestringArray);

        GFAL_TEST_ASSERT(
            strcmp(req->transferParameters->arrayOfTransferProtocols->stringArray[0],
                   _unsupported_protocol[0]) == 0
        );
        /* In case of unsupported protocol, the return status is SRM_NOT_SUPPORTED */
        rep->srmBringOnlineResponse->returnStatus->statusCode = SRM_USCORENOT_USCORESUPPORTED;
        rep->srmBringOnlineResponse->arrayOfFileStatuses->__sizestatusArray = 0;
        return SOAP_OK;
    default:
        assert(0);
    }

    return 1;
}

/* The dependency injection comes here. This function replaces the production
 * version SOAP calls so that no network operation, server, etc be required.
 * The function directly produces and returns the SRM response objects.
 *
 * This is the "SOAP call" for the srmPrepareToPut operation. */
static int _caller_play_scenarios_prepare_to_put(
    struct soap * soap, const char * srm_endpoint, const char * srmfunc,
    struct srm2__srmPrepareToPutRequest * req,
    struct srm2__srmPrepareToPutResponse_ * rep)
{
    _prepare_response_prepare_to_put(soap, rep);

    switch(_protocol_list_scenario) {
    case E_EMPTY_LIST:
        GFAL_TEST_EQUAL(0, req->transferParameters->arrayOfTransferProtocols);
        /* Pretend that the call itself is successful, and do nothing */
        rep->srmPrepareToPutResponse->returnStatus->statusCode = SRM_USCORESUCCESS;
        return SOAP_OK;

    case E_UNSUPPORTED_PROTOCOL:
        /* Check if the function filled in the appropriate fields of the request */
        GFAL_TEST_EQUAL(1,
            req->transferParameters->arrayOfTransferProtocols->__sizestringArray);

        GFAL_TEST_ASSERT(
            strcmp(req->transferParameters->arrayOfTransferProtocols->stringArray[0],
                   _unsupported_protocol[0]) == 0
        );
        /* In case of unsupported protocol, the return status is SRM_NOT_SUPPORTED */
        rep->srmPrepareToPutResponse->returnStatus->statusCode = SRM_USCORENOT_USCORESUPPORTED;
        rep->srmPrepareToPutResponse->arrayOfFileStatuses->__sizestatusArray = 0;
        return SOAP_OK;
    default:
        assert(0);
    }

    return 1;
}

/* Inject the dependencies. Replace the SOAP callers to the test ones. */
static void _setup_srm_callers()
{
    gfal_srm_callers_v2.call_prepare_to_get = _caller_play_scenarios_prepare_to_get;
    gfal_srm_callers_v2.call_bring_online = _caller_play_scenarios_bring_online;
    gfal_srm_callers_v2.call_prepare_to_put = _caller_play_scenarios_prepare_to_put;
}

/* General fixtures for the SRM requests*/
static char *mock_surls[] = {"srm://fake_surl/"};
static int mock_nbfiles = 1;
static char *mock_srm_endpoint = "httpg://fakeendpoint.cern.ch:8443/srm/managerv2";
static int mock_desiredpintime = 10;
static const char *mock_spacetokendesc = NULL;
static char ** mock_reqtoken = NULL;
static struct srmv2_pinfilestatus * mock_pinfilestatus[3];
static int errbufsz = 1024;
static char errbuf[1024];
static int mock_timeout = 100;

/* Test the following functions (they have same signatures, so they can be
 * tested in a generic way):
 *
 * srmv2_turlsfromsurls_get, srmv2_gete, srmv2_prestagee, srmv2_bringonline */
static char* _test_protocol_list(srmv2_fv_type fv)
{
    int res;
    errno = 0;
   /* SCENARIO 1: empty protocol list. Call must be OK. */
    _protocol_list_scenario = E_EMPTY_LIST;

    res = fv(mock_nbfiles, (const char**)mock_surls, (const char*)mock_srm_endpoint,
        mock_desiredpintime, mock_spacetokendesc,
        NULL, /* The empty protocol list */
        mock_reqtoken, mock_pinfilestatus, errbuf, errbufsz, mock_timeout);

    GFAL_TEST_EQUAL(NUMBER_OF_FILES, res);
    GFAL_TEST_EQUAL(0, errno);
    errno = 0;
    errbuf[0] = 0;

    /* SCENARIO 2: Wrong protocol list. Call must return error. */
    _protocol_list_scenario = E_UNSUPPORTED_PROTOCOL;

    res = fv(mock_nbfiles, (const char**)mock_surls, (const char*)mock_srm_endpoint,
        mock_desiredpintime, mock_spacetokendesc,
        _unsupported_protocol,
        mock_reqtoken, mock_pinfilestatus, errbuf, errbufsz, mock_timeout);

    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(EOPNOTSUPP, errno);
}

/* Test the srmv2_turlsfromsurls_put function. */
static char* _test_srmv2_turlsfromsurls_put()
{
    GFAL_LONG64 mock_filesizes[NUMBER_OF_FILES] = {10};
    int res;
    errno = 0;

   /* SCENARIO 1: empty protocol list. Call must be OK. */
    _protocol_list_scenario = E_EMPTY_LIST;

    res = srmv2_turlsfromsurls_put(mock_nbfiles, (const char**)mock_surls,
        (const char*)mock_srm_endpoint, mock_filesizes, mock_desiredpintime,
        mock_spacetokendesc,
        NULL, /* The empty protocol list */
        mock_reqtoken, mock_pinfilestatus, errbuf, errbufsz, mock_timeout);

    GFAL_TEST_EQUAL(NUMBER_OF_FILES, res);
    GFAL_TEST_EQUAL(0, errno);
    errno = 0;
    errbuf[0] = 0;

    /* SCENARIO 2: Wrong protocol list. Call must return error. */
    _protocol_list_scenario = E_UNSUPPORTED_PROTOCOL;

    res = srmv2_turlsfromsurls_put(mock_nbfiles, (const char**)mock_surls,
        (const char*)mock_srm_endpoint, mock_filesizes, mock_desiredpintime,
        mock_spacetokendesc,
        _unsupported_protocol,
        mock_reqtoken, mock_pinfilestatus, errbuf, errbufsz, mock_timeout);

    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(EOPNOTSUPP, errno);
}

static char* _test_gfal_turlsfromsurls()
{
    gfal_internal int_req = NULL;
    gfal_request req = NULL;
    const errbufsz = 1024;
    char errbuf[errbufsz];
    int res;
    GFAL_LONG64 mock_filesizes[NUMBER_OF_FILES] = {10};

    req = gfal_request_new();
    req->nbfiles = mock_nbfiles;
    req->defaultsetype = TYPE_SRMv2;
    req->setype = TYPE_SRMv2;
    req->no_bdii_check = 1;
    req->srmv2_spacetokendesc = (char*) mock_spacetokendesc;
    req->surls = mock_surls;
    req->endpoint = mock_srm_endpoint;

    /* SCENARIO 1: empty protocol list. Call must be OK. */
    /* This will utilize srmv2_turlsfromsurls_get */
    req->protocols = NULL;
    gfal_init (req, &int_req, errbuf, errbufsz);
    errno = 0;
    _protocol_list_scenario = E_EMPTY_LIST;
    res = gfal_turlsfromsurls (int_req, errbuf, errbufsz);
    GFAL_TEST_EQUAL(0, res);
    GFAL_TEST_EQUAL(0, errno);

    /* This will utilize srmv2_turlsfromsurls_put */
    req->oflag |= O_ACCMODE;
    gfal_init (req, &int_req, errbuf, errbufsz);
    errno = 0;
    _protocol_list_scenario = E_EMPTY_LIST;
    res = gfal_turlsfromsurls (int_req, errbuf, errbufsz);
    GFAL_TEST_EQUAL(0, res);
    GFAL_TEST_EQUAL(0, errno);

    /* SCENARIO 1: empty protocol list. Call must be OK. */
    /* This will utilize srmv2_turlsfromsurls_get */
    req->oflag = 0;
    req->protocols = _unsupported_protocol;
    gfal_init (req, &int_req, errbuf, errbufsz);
    errno = 0;
     _protocol_list_scenario = E_UNSUPPORTED_PROTOCOL;
    res = gfal_turlsfromsurls (int_req, errbuf, errbufsz);
    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(EOPNOTSUPP, errno);

    /* This will utilize srmv2_turlsfromsurls_put */
    req->oflag |= O_ACCMODE;
    gfal_init (req, &int_req, errbuf, errbufsz);
    errno = 0;
     _protocol_list_scenario = E_UNSUPPORTED_PROTOCOL;
    res = gfal_turlsfromsurls (int_req, errbuf, errbufsz);
    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(EOPNOTSUPP, errno);

    return NULL;
}

char * gfal_test__protocol_list_handling()
{
    char* res = NULL;
    _setup_srm_callers();

    __CALL_TEST(_test_protocol_list(srmv2_turlsfromsurls_get));
    __CALL_TEST(_test_protocol_list(srmv2_gete));
    __CALL_TEST(_test_protocol_list(srmv2_prestagee));
    __CALL_TEST(_test_protocol_list(srmv2_bringonline));
    __CALL_TEST(_test_srmv2_turlsfromsurls_put());
    __CALL_TEST(_test_gfal_turlsfromsurls());

    return NULL;
}

#undef __CALL_TEST
