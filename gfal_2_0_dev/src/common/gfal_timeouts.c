/*
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
 */

/*
 * @(#)$RCSfile: gfal_timeouts.c,v $ $Revision: 1.7 $ $Date: 2009/03/25 13:41:07 $ CERN Remi Mollon
 */

#if 0

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfal_internals.h"

static int gfal_timeout_connect = 60;
static int gfal_timeout_sendreceive = 0;
static int gfal_timeout_bdii = 60;
static int gfal_timeout_srm = 3600;

void
gfal_set_timeout_connect (int value) {
	char *lfc_timeout = NULL;

	if (value >= 0)
		gfal_timeout_connect = value;

	/* set 'LFC_CONNTIMEOUT' to same value */
	asprintf (&lfc_timeout, "LFC_CONNTIMEOUT=%d", value);
	if (lfc_timeout == NULL)
		return;

	putenv (lfc_timeout);
	free (lfc_timeout);
}

int gfal_get_timeout_connect () {
	return (gfal_timeout_connect);
}

void
gfal_set_timeout_sendreceive (int value) {
	if (value >= 0)
		gfal_timeout_sendreceive = value;
}

int gfal_get_timeout_sendreceive () {
	return (gfal_timeout_sendreceive);
}

void
gfal_set_timeout_bdii (int value) {
	if (value >= 0)
		gfal_timeout_bdii = value;
}

int gfal_get_timeout_bdii () {
	return (gfal_timeout_bdii);
}

void
gfal_set_timeout_srm (int value) {
	if (value >= 0)
		gfal_timeout_srm = value;
}

int gfal_get_timeout_srm () {
	return (gfal_timeout_srm);
}

#endif
