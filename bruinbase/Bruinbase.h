/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @date 3/24/2008
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 */

#ifndef BRUINBASE_H
#define BRUINBASE_H

typedef int RC;

const int RC_FILE_OPEN_FAILED    = -1001;
const int RC_FILE_CLOSE_FAILED   = -1002;
const int RC_FILE_SEEK_FAILED    = -1003;
const int RC_FILE_READ_FAILED    = -1004;
const int RC_FILE_WRITE_FAILED   = -1005;
const int RC_INVALID_FILE_MODE   = -1006;
const int RC_INVALID_PID         = -1007;
const int RC_INVALID_RID         = -1008;
const int RC_INVALID_FILE_FORMAT = -1009;
const int RC_NODE_FULL           = -1010;
const int RC_INVALID_CURSOR      = -1011;
const int RC_NO_SUCH_RECORD      = -1012;
const int RC_END_OF_TREE         = -1013;
const int RC_INVALID_ATTRIBUTE   = -1014;

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#endif // BRUINBASE_H
