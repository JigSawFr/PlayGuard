// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/error_hint_logic.hpp"

#include "core/nx_types.h"

namespace error_hint_logic
{

const char* hint_key(uint32_t rc)
{
    if (NXM_IS_APP_RESULT(rc)) {
        // By description: NXM_RC_READ_ONLY is 1.
        static const char* const OWN[] = {
            "",                       // 0: not used
            "read_only",              // NXM_RC_READ_ONLY
            "write_gated",            // NXM_RC_WRITE_GATED
            "fw_unsupported",         // NXM_RC_FW_UNSUPPORTED
            "unlock_not_effective",   // NXM_RC_UNLOCK_NOT_EFFECTIVE
            "not_custom",             // NXM_RC_NOT_CUSTOM
            "invalid_argument",       // NXM_RC_INVALID_ARGUMENT
            "autocorrect_off",        // NXM_RC_AUTOCORRECT_OFF
            "state_unknown",          // NXM_RC_STATE_UNKNOWN
            "not_confirmed",          // NXM_RC_NOT_CONFIRMED
            "no_pin",                 // NXM_RC_NO_PIN
            "relock_failed",          // NXM_RC_RELOCK_FAILED
            "not_applied",            // NXM_RC_NOT_APPLIED
            "not_saved",              // NXM_RC_NOT_SAVED
            "pt_not_understood",      // NXM_RC_PT_NOT_UNDERSTOOD
        };
        const uint32_t desc = NXM_RESULT_DESC(rc);
        return desc < sizeof(OWN) / sizeof(OWN[0]) ? OWN[desc] : "";
    }
    switch (rc) {
        case 0xF601: return "session_closed";   // 2001-0123 SessionClosed
        case 0xF80E: return "bad_pin_format";
        // 2142-0132: reported writing the play timer on 22.1 (NX-Pctl-Manager #5); meaning unknown.
        case 0x1088E: return "pctl_0132";
        // 2142-0223 (switchbrew): IsAnyInternetRequestAccepted returned false, no connection.
        case 0x1BE8E: return "no_internet";
        // 2116-0102 (switchbrew: OffsetInvalid): switch-time saw it reading a never-set network clock.
        case 0xCC74: return "clock_not_set";
        // 2116-0103 (switchbrew: UninitializedClock).
        case 0xCE74: return "clock_not_set";
        // 2021-0008 (switchbrew: sm NotAllowed): no access to the service asked for.
        case 0x1015: return "service_denied";
        // 2178-0150 (switchbrew: pdm "Service unavailable").
        case 0x12CB2: return "pdm_unavailable";
        default: break;
    }
    if ((rc & 0x1FF) == 142) return "pctl_refused";   // pctl module
    return "";
}

}   // namespace error_hint_logic
