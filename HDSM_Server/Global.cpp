#include "Global.h"

string Global::CONFIGURE_FILE_PATH = ConfigureMgr::get_local_configure_file();
HUINT32 Global::MAX_KEY_LENGTH = ConfigureMgr::get_max_key_length() + 9;
HUINT32 Global::MAX_VALUE_LENGTH = ConfigureMgr::get_max_value_length();
HUINT32 Global::MAX_SHARD_COUNT = ConfigureMgr::get_shard_count();
HUINT32 Global::MAX_INVALID_KEYS = ConfigureMgr::get_max_invalid_keys();
HUINT32 Global::MAX_BUFFER_ROOMS_COUNT = 128;
HUINT32 Global::MAX_BUFFER_ROOMS_SIZE = 128 + MAX_VALUE_LENGTH + MAX_KEY_LENGTH;
HUINT32 Global::SECONDS_IN_ONE_HOURS = 3600;
HUINT32 Global::SECONDS_IN_ONE_MINUTE = 60;
HUINT32 Global::SECONDS_IN_ONE_DAY = 24 * 3600;
HUINT32 Global::SERVER_MODE = ConfigureMgr::get_server_mode();
HUINT32 Global::VERSION_CODE = 1;