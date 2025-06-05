#pragma once
#include <Arduino.h>

void cmd_ls(const String& args);
void cmd_ls_detailed(const String& args);
void cmd_cat(const String& args);
void cmd_rm(const String& args);
void cmd_edit(const String& args);
void cmd_touch(const String& args);
void cmd_mv(const String& args);
void cmd_cp(const String& args);
void cmd_echo(const String& args);
void cmd_clear(const String& args);
void cmd_help(const String& args);
void cmd_config(const String& args);
void cmd_reload(const String& args);
void cmd_tokens(const String& args);
void cmd_top(const String& args);
void cmd_exit(const String& args);
void cmd_reboot(const String& args);
void cmd_ping(const String& args);
void cmd_version(const String& args);
void cmd_whoami(const String& args);
