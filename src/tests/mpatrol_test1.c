

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "mpatrol.h"
#include "mpatrol_suite.h"


START_TEST(mpatrol_test_test1)
{
    void *a[256];
    size_t i, n;

    for (i = 0; i < 256; i++)
    {
        n = (size_t) ((rand() * 256.0) / (RAND_MAX + 1.0)) + 1;
        a[i] = malloc(n);
    }
    for (i = 256; i > 0; i--)
    {
        n = (size_t) ((rand() * 256.0) / (RAND_MAX + 1.0)) + 1;
        a[i - 1] = realloc(a[i - 1], n);
    }
    for (i = 0; i < 256; i += 2)
        free(a[i]);
    for (i = 256; i > 0; i -= 2)
        free(a[i - 1]);
}
END_TEST

void
mpatrol_test1(TCase *tc)
{
   tcase_add_test(tc, mpatrol_test_test1);
}
