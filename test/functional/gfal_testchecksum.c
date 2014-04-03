/**
 * Compile command : gcc gfal_testchecksum.c `pkg-config --libs --cflags gfal2`
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <string.h>
#include <common/gfal_lib_test.h>

static int get_precomputed(const char* algorithm, const char** content, const char** checksum)
{
    static const char *message = "THIS IS A STATIC MESSAGE USED FOR CHECKSUMING";
    static struct {
        const char* algorithm;
        const char* hash;
    } precomputed[] = {
        {"MD5", "d0fb2562f39431dbe78c140581ca06b8"},
        {"SHA1", "87c813e21babb87879588dc6009770f3c2b5d2f6"},
        {"ADLER32", "111a0c0c"},
        {"CRC32", "ecb904fa"},
        {NULL, NULL}
    };

    *content = message;
    size_t i;
    for (i = 0; precomputed[i].algorithm != NULL; ++i) {
        if (strcmp(algorithm, precomputed[i].algorithm) == 0) {
            *checksum = precomputed[i].hash;
            return 0;
        }
    }

    return 1;
}

static int fill_file(gfal2_context_t context,
        const char* surl, const char* content)
{
    GError* error = NULL;

    int fd = gfal2_open(context, surl, O_CREAT | O_TRUNC | O_WRONLY, &error);
    if (fd < 0) {
        printf("Failed to open: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    size_t len = strlen(content);
    if (gfal2_write(context, fd, content, len, &error) < 0) {
        printf("Failed to write: %s\n", error->message);
        g_error_free(error);
        gfal2_close(context, fd, &error); // Ignore errors
        return 1;
    }

    if (gfal2_close(context, fd, &error) != 0) {
        printf("Failed to close: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    return 0;
}

static int checksum_match(const char* expected, const char* actual)
{
    if (strcasecmp(expected, actual) == 0)
        return 1;
    // Second try using integer value (the base may change!)
    unsigned long anum = strtol(expected, NULL, 16);
    unsigned long bnum = strtol(actual, NULL, 10);
    return anum == bnum;
}

static int test_checksum(gfal2_context_t context,
        const char* url, const char* algorithm, const char* expected)
{
    GError *error = NULL;
    char buffer[512];

    if (gfal2_checksum(context, url, algorithm, 0, 0, buffer, sizeof(buffer), &error) < 0) {
        printf("Failed to get the checksum: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    if (!checksum_match(expected, buffer)) {
        printf("The checksums do not match! %s != %s\n", expected, buffer);
        return 1;
    }

    printf("The checksums match!\n");

    return 0;
}

static char* to_upper(char* s)
{
    size_t i;
    for (i = 0; s[i] != '\0'; ++i)
        s[i] = toupper(s[i]);
    return s;
}

int main(int argc, char **argv)
{
    GError *error = NULL;
    int ret = 0;

    if (argc < 3) {
        printf("usage: %s root_dir [MD5|ADLER32|SHA1|CRC32]\n", argv[0]);
        return 1;
    }
    const char* root_dir = argv[1];
    const char* algorithm = to_upper(argv[2]);

    const char* precomputed_content;
    const char* precomputed_hash;

    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_DEBUG | GFAL_VERBOSE_VERBOSE);

    gfal2_context_t context = gfal2_context_new(&error);
    g_assert(context != NULL);
    g_assert(error == NULL);

    if (get_precomputed(algorithm, &precomputed_content, &precomputed_hash) != 0) {
        printf("Could not find precomputed content with %s\n", algorithm);
        return 1;
    }

    char filename[2048];
    generate_random_uri(root_dir, "test_checksum", filename, 2048);
    if ((ret = fill_file(context, filename, precomputed_content)) != 0)
        goto done;

    ret = test_checksum(context, filename, algorithm, precomputed_hash);

done:
    clean_file(filename);
    return ret;
}
