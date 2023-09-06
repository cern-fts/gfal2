/*
 * Copyright (c) CERN 2023
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#ifndef _GFAL_HTTP_PLUGIN_COPY_H
#define _GFAL_HTTP_PLUGIN_COPY_H

/** SciTag maximum legal value */
#define GFAL_SCITAG_MAX_VALUE 1<<16

/**
 * Gfal2 HTTP Plugin abstraction for the copy mode,
 * which can be one of "pull", "push" and "stream".
 *
 * There is a fallback mechanism from pull --> push --> stream.
 *
 * A factory method is provided to construct the HttpCopyMode object
 * based on the Gfal2 configuration and the source and destination endpoints.
 */

class HttpCopyMode {
public:
    enum class CopyMode {
        PULL,
        PUSH,
        STREAM,
        NONE
    };

    void next();
    bool end() const;
    const char* str() const;

    inline CopyMode value() const {
        return copyMode;
    };

    inline bool isStreamingOnly() const {
        return streamingOnly;
    };

    inline bool isStreamingEnabled() const {
        return streamingEnabled;
    };

    static HttpCopyMode ConstructCopyMode(gfal2_context_t context, const char* src, const char* dst);

    static CopyMode CopyModeFromQueryArguments(const char* surl);
    static CopyMode CopyModeFromStr(const char* copyModeStr);

    static const char* CopyModeToStr(CopyMode copyMode);

private:
    HttpCopyMode(CopyMode copyMode, bool streamingOnly, bool streamingEnabled) :
            copyMode(copyMode), streamingOnly(streamingOnly), streamingEnabled(streamingEnabled) {}

    CopyMode copyMode;
    bool streamingOnly;
    bool streamingEnabled;
};

#endif //_GFAL_HTTP_PLUGIN_COPY_H
