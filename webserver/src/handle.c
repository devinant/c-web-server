#include <string.h>
#include "handle.h"

void handle_init(handle_s *handle) {
    memset(handle, 0, sizeof(*handle));
}