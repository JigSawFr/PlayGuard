/* libFuzzer target: the request and report files of source/core/rescue.c,
 * which anyone with the SD card can write. Beyond the sanitizers: a report
 * that parses formats back to text that parses to the same report, and a
 * refused one leaves *out untouched. */
#include <string.h>

#include "fuzz.h"
#include "rescue.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    const char *text = (const char *)data;

    const RescueMode mode = rescue_request_mode(text, size);
    FUZZ_ASSERT(mode == RescueMode_Unlock || mode == RescueMode_Delete);

    RescueReport r;
    memset(&r, 0xA5, sizeof r);
    RescueReport untouched;
    memcpy(&untouched, &r, sizeof r);
    if (!rescue_report_parse(text, size, &r)) {
        FUZZ_ASSERT(memcmp(&r, &untouched, sizeof r) == 0);
        return 0;
    }
    char buf[256];
    const size_t n = rescue_report_format(&r, buf, sizeof buf);
    FUZZ_ASSERT(n > 0 && n < sizeof buf);
    RescueReport back;
    FUZZ_ASSERT(rescue_report_parse(buf, n, &back));
    FUZZ_ASSERT(back.mode == r.mode && back.result == r.result && back.rc == r.rc && back.unlocks == r.unlocks &&
                back.request == r.request);
    rescue_report_confirmed(&r, 0, false);
    rescue_report_confirmed(&r, 4, true);
    FUZZ_ASSERT(rescue_mode_name(r.mode) != NULL);
    return 0;
}
